#include <pid/pids.h>
#include <string.h>
#include <rmi/rmi.h>
#include <miscellaneous/hash.h>
#include <unicode.h>
#include <pid/pids.h>

#include "../fs/fsregister.h"
#include "../fs/charerr.h"
#include "../fs/charexports.h"
#include "../fs/chardev.h"
#include "../vfs/vfstypes.h"
#include "../vfs/exports.h"

#include "exports.h"
#include "fserr.h"

#define TIMEOUT	20

typedef struct {
	MOUNT mount;
	RMISERIAL charDevice;
	void* fsSpecific;
	UINT32 uID;
} MOUNT_STORE;


/* ---------------------------------------------------------------------------------------------------------------------------------------- */

CALLBACK MountFileSystem(RMISERIAL ataDriver, INIT_MOUNT *mountInfo, UINT32 msgID);
CALLBACK UnMountFileSystem(RMISERIAL ataDriver, INIT_MOUNT *mountInfo, UINT32 msgID);
CALLBACK AccessFile(RMISERIAL sender, ACCOBJ open, UINT32 msgID);
CALLBACK ReadFileAt(RMISERIAL sender, FILESTREAM file, UINT32 msgID);
CALLBACK FindNextFile(RMISERIAL sender, VFS_HANDLE handle, UINT32 msgID);
CALLBACK FileInfo(RMISERIAL sender, VFS_HANDLE handle, UINT32 msgID);

/* ---------------------------------------------------------------------------------------------------------------------------------------- */

UINT32 globalUID = 0;
PHASH mountHash;
PHASH handleHash;

void WaitForCharDevice(UINT32 number) {
	char hexNumber[19] = "CharDev:0x00000000";
	char hex[16] = "0123456789ABCDEF";

	char *hexConvert = hexNumber + 17;
	
	while(number) {
		*hexConvert = hex[number % 16];
		hexConvert--;
		number /= 16;	
	}
	
	BarrierArrive(hexNumber);
}

UINT32 GenerateUID() {
	return ++globalUID;
}


void FSInit(RMISERIAL FSserial, UINT32 maxMounts, UINT32 maxHandles) {
	RMISetup(FSserial, 8);
	RMIRegister(FS_EXPORT_MOUNT_FS, MountFileSystem, 255, FALSE, RTYPE_STRING, 2048);
	RMIRegister(FS_EXPORT_UNMOUNT_FS, UnMountFileSystem, 255, FALSE, RTYPE_STRING, 2048);
	RMIRegister(FS_EXPORT_ACCESSFILE, AccessFile, 255, FALSE, RTYPE_STRING, 2048);
	RMIRegister(FS_EXPORT_READFILEAT, ReadFileAt, 255, FALSE, RTYPE_STRING, sizeof(struct _FILESTREAM));
	RMIRegisterValue(FS_EXPORT_FINDNEXTFILE, FindNextFile, 255, FALSE);
	RMIRegisterValue(FS_EXPORT_FILEINFO, FileInfo, 255, FALSE);
	
	mountHash = (PHASH)malloc(sizeof(HASH));
	HashCreate(mountHash, maxMounts);
	HashInit(mountHash);
	
	handleHash = (PHASH)malloc(sizeof(HASH));
	HashCreate(handleHash, maxHandles);
	HashInit(handleHash);
	
}


MOUNT_STORE* AddMount(MOUNT mountInfo, RMISERIAL charDevice, UINT32 specificLength) {
	MOUNT_STORE* newMount;
	
	newMount = (MOUNT_STORE*)malloc(sizeof(MOUNT_STORE) + specificLength);
	if (!newMount) return NULL;
	newMount->fsSpecific = newMount + 1;
	newMount->mount = mountInfo;
	newMount->uID = GenerateUID();
	
	HashInsert(mountHash, newMount->uID, (UINT32)newMount);
	return newMount;
}

static BOOL RegisterDriver(UINT32 charDevice, UINT8 systemID, char *fsName) {
	FS_REGISTER *myFS;
	RMIMESSAGE result;
	UINT32 tries = TIMEOUT;
	
	myFS = (FS_REGISTER*)AllocateMessageBuffer(sizeof(FS_REGISTER), RTYPE_STRING);
	if (!myFS) {
		return FALSE;
	}
	
	myFS->systemID = systemID;
	strcpy(myFS->fileSystemName, fsName);
	
contact:
	switch (RMCallMessageValue(charDevice, CHAR_EXPORT_REGISTER_FS, (RMIMESSAGE)myFS, &result)) {
	case RMI_SUCCESS:
		switch ((UINT32)result) {
		case CHAR_ERR_SUCCESS:
			return TRUE;
			break;
		default:
			return FALSE;
		}
		break;
	default:
		if (!--tries) {
			return FALSE;
		}
		Relinquish();
		goto contact;
		break;
	}
}

static VFS_STATUS RegisterVFSNode(TUString rootPath, TUString nodeName, TUString addInfo, UINT32 mountID) {
	REGNODE reg;
	RMIMESSAGE result;
	UINT32 tries = TIMEOUT;
	UINT32 errCode;
	
	reg = (REGNODE)AllocateMessageBuffer(sizeof(struct _REGNODE) + (rootPath.length + nodeName.length + addInfo.length) * sizeof(UChar), RTYPE_STRING);
	if (!reg) {
		return VFS_ERR_MEMORY;
	}
	
	RMINewStructure(rmiStruct, reg, reg + 1);
	
	
	CreateRMIStructFromString(rmiStruct, reg, registerPath, rootPath);
	CreateRMIStructFromString(rmiStruct, reg, newNodeName, nodeName);
	CreateRMIStructFromString(rmiStruct, reg, addInfoName, addInfo);
	
	reg->mountID = mountID;
	
contact:
	switch (RMCallMessage(DRV_VFS, VFS_EXPORT_REGISTERNODE, (RMIMESSAGE)reg, &result, RTYPE_STRING, 2048)) {
	case RMI_SUCCESS:
		if (result) {
			reg = (REGNODE)result;
			errCode = reg->errCode;
			FreeMessageBuffer((RMIMESSAGE)reg);
			return errCode;
		} else {
			return VFS_ERR_GENERAL;
		}
		break;
	default:
		if (!--tries) {
			return VFS_ERR_TIMEOUT;
		}
		Relinquish();
		goto contact;
		break;
	}
}

UINT32 ReadSectors(PCHAR_DISK read, unsigned char **pointer) {
	RMIMESSAGE data;
	
	switch (RMCallMessage(DRV_ATA, CHAR_EXPORT_READSEC, (RMIMESSAGE)read, &data, RTYPE_STRING, read->count * 512)) {
	case RMI_SUCCESS:
		if (data) {
			if ((UINT32)data < 100) {
				return (UINT32)data;
			}
			*pointer = (unsigned char*)data;
			return FS_ERR_SUCCESS;
		} else {
			return FS_ERR_READING_SECTOR;
		}
		break;
	case RMI_NOT_SUPPORTED:
		return FS_ERR_COMMUNICATION;
	default:
		return FS_ERR_READ;
	}
}

