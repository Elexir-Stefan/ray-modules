#include <raykernel.h>
#include <string.h>
#include <unicode.h>
#include <rmi/rmi.h>
#include <memory/memory.h>
#include <threads/sleep.h>
#include <pid/pids.h>
#include <miscellaneous/hash.h>
#include <miscellaneous/stringhash.h>

#include "vfs.h"


#include "vfsstructure.h"
#include "vfstree.h"

#include "exports.h"

#include "../fs/exports.h"

#define MAX_HANDLES	256

extern VFSnode *root;
PHASH handleHash;
int currHandle;

static inline UINT32 GetNewHandle();

static inline UINT32 GetNewHandle() {
	//UINT32 handle;
	//__asm__ __volatile__("rdtsc":"=a"(handle)::"edx");
	return currHandle++;
}


VFSnode *CreateNode(RMISERIAL sender, PTUString registerPath, PTUString newNodeName, PTUString addInfoName, UINT32 mountID) {
	VFSnode *newNode;
	newNode = (VFSnode*)malloc(sizeof(VFSnode) + (newNodeName->length + addInfoName->length) * sizeof(UChar));
	memset(newNode, 0, sizeof(VFSnode));

	newNode->name.string = (UChar*)(newNode + 1);
	newNode->addInfoName.string = newNode->name.string + newNodeName->length;
	
	newNode->name.length = newNodeName->length;
	newNode->addInfoName.length = addInfoName->length;
	
	memcpy(newNode->name.string, newNodeName->string, newNodeName->length * sizeof(UChar));
	memcpy(newNode->addInfoName.string, addInfoName->string, addInfoName->length * sizeof(UChar));
	
	newNode->pathHash = SimpleHashTUString(registerPath);
	newNode->registeredDriver = sender;
	newNode->mountID = mountID;
	
	newNode->root = NULL;
	newNode->tail = NULL;

	return newNode;
}


CALLBACK RegisterNode(RMISERIAL sender, REGNODE reg, UINT32 msgID) {
	VFSnode* found;
	VFSnode* exists;
	
	TUString regPath = CreateStringFromRMIStruct(reg, registerPath);
	TUString newNodeName = CreateStringFromRMIStruct(reg, newNodeName);
	TUString addInfo = CreateStringFromRMIStruct(reg, addInfoName);
	
	found = GetNodeByPath(root, regPath);
	
	if(found) {
		/* check if already registered */
		exists = GetNodeByPath(found->root, newNodeName);
		if (!exists) {
			AddToDir(CreateNode(sender, &regPath, &newNodeName, &addInfo, reg->mountID), found);
			reg->errCode = VFS_SUCCESS;
		} else {
			reg->errCode = VFS_ERR_OCCUPIED;
		}
	} else {
		reg->errCode = VFS_ERR_INVALID_PATH;
	}
	
	CReturn((RMIMESSAGE)reg, msgID, TRUE);
}

CALLBACK MakeDir(RMISERIAL sender, NEWDIR dir, UINT32 msgID) {
	ThreadExit(-1);
}

ACCOBJ FSAccessFile(ACCOBJ access, VFSnode* driver) {
	RMIMESSAGE result;
	access->mountID = driver->mountID;
	RAY_RMI rmiStatus = RMCallMessage(driver->registeredDriver, FS_EXPORT_ACCESSFILE, (RMIMESSAGE)access, &result, RTYPE_SAME, 0);
	switch (rmiStatus) {
	case RMI_SUCCESS:
		return (ACCOBJ)result;
		break;
	default:
		access->errCode = VFS_ERR_COMMUNICATION;
		access->extendedErrorInfo = rmiStatus;
		return access;
	}
}

CALLBACK ReadFileAt(RMISERIAL sender, FILESTREAM stream, UINT32 msgID) {
	HANDLEINFO info;
	RMIMESSAGE result;
	VFS_BUFFER buffer;
	
	info = (HANDLEINFO)HashRetrieve(handleHash, stream->handle.handleUID);
	if (info) {
		switch(RMCallMessage(info->dir->registeredDriver, FS_EXPORT_READFILEAT, (RMIMESSAGE)stream, &result, RTYPE_STRING, sizeof(struct _VFS_BUFFER) + stream->length)) {
		case RMI_SUCCESS:
			CReturn((RMIMESSAGE)result, msgID, TRUE);
			break;
		default:
			buffer = (VFS_BUFFER)AllocateMessageBuffer(sizeof(struct _VFS_BUFFER), RTYPE_STRING);
			buffer->errCode = VFS_ERR_COMMUNICATION;
			CReturn((RMIMESSAGE)buffer, msgID, TRUE);
		}
	} else {
		buffer = (VFS_BUFFER)AllocateMessageBuffer(sizeof(struct _VFS_BUFFER), RTYPE_STRING);
		buffer->errCode = VFS_ERR_INVALID_HANDLE;
		CReturn((RMIMESSAGE)buffer, msgID, TRUE);
	}
	
}

CALLBACK FileInfo(RMISERIAL sender, VFS_HANDLE handle, UINT32 msgID) {
	HANDLEINFO info;
	RMIMESSAGE result;
	VFS_INFO fileInfo;
	
	info = (HANDLEINFO)HashRetrieve(handleHash, handle.handleUID);
	if (info) {
		switch(RMCall(info->dir->registeredDriver, FS_EXPORT_FILEINFO, handle.handleUID, &result, RTYPE_STRING, 2048)) {
			case RMI_SUCCESS:
				CReturn((RMIMESSAGE)result, msgID, TRUE);
				break;
			default:
				fileInfo = (VFS_INFO)AllocateMessageBuffer(sizeof(struct _VFS_INFO), RTYPE_STRING);
				fileInfo->errCode = VFS_ERR_COMMUNICATION;
				CReturn((RMIMESSAGE)fileInfo, msgID, TRUE);
		}
	} else {
		fileInfo = (VFS_INFO)AllocateMessageBuffer(sizeof(struct _VFS_INFO), RTYPE_STRING);
		fileInfo->errCode = VFS_ERR_INVALID_HANDLE;
		CReturn((RMIMESSAGE)fileInfo, msgID, TRUE);
	}
}

static VFS_INFO FindNextVFSDir(VFSnode** dir) {
	VFS_INFO dirInfo;
	
	if (*dir) {
	
		dirInfo = (VFS_INFO)AllocateMessageBuffer(sizeof(struct _VFS_INFO) + (*dir)->name.length * sizeof(UChar), RTYPE_STRING);
		
		RMINewStructure(rmiStruct, dirInfo, dirInfo + 1);
		CreateRMIStructFromString(rmiStruct, dirInfo, name, (*dir)->name);
		
		dirInfo->aOwner.is = VFS_ATTR_DIRECTORY;
		VFS_DONT_USE_ATTRIBUTES(dirInfo->aGroup);
		VFS_DONT_USE_ATTRIBUTES(dirInfo->aOthers);
		VFS_NO_OWNER(dirInfo->owner);
		dirInfo->fileSize = 0;
		dirInfo->errCode = VFS_SUCCESS;
		
		(*dir) = (*dir)->next;
	} else {
		dirInfo = (VFS_INFO)AllocateMessageBuffer(sizeof(struct _VFS_INFO), RTYPE_STRING);
		dirInfo->errCode = VFS_ERR_FILE_NOT_FOUND;
	}
	
	return dirInfo;
}

CALLBACK FindNextFile(RMISERIAL sender, UINT32 handle, UINT32 msgID) {
	HANDLEINFO info;
	VFS_INFO fileInfo;
	RMIMESSAGE result;
	
	info = (HANDLEINFO)HashRetrieve(handleHash, handle);
	if (info) {
		if (info->attributes & HANDLE_PFS) {
			/* physical file, contact driver */
			switch (RMCall(info->dir->registeredDriver, FS_EXPORT_FINDNEXTFILE, handle, &result, RTYPE_STRING, 2048)) {
			case RMI_SUCCESS:
				//CReturn((RMIMESSAGE)result, msgID, TRUE);
				fileInfo = (VFS_INFO)result;
				break;
			default:
				fileInfo = (VFS_INFO)AllocateMessageBuffer(sizeof(struct _VFS_INFO), RTYPE_STRING);
				fileInfo->errCode = VFS_ERR_COMMUNICATION;
			}
		} else {
			/* vfs entry to search */
			fileInfo = FindNextVFSDir(&info->dir);
		}
	} else {
		fileInfo = (VFS_INFO)AllocateMessageBuffer(sizeof(struct _VFS_INFO), RTYPE_STRING);
		fileInfo->errCode = VFS_ERR_INVALID_HANDLE;
	}
	
	CReturn((RMIMESSAGE)fileInfo, msgID, TRUE);
}

CALLBACK AccessFile(RMISERIAL sender, ACCOBJ access, UINT32 msgID) {
	VFSnode *fsDriver, *fsEntry;
	HANDLEINFO info;
	
	/* close or open a file? */
	if (access->mode == VFS_ACC_CLOSE) {
		info = (HANDLEINFO)HashRetrieve(handleHash, access->handle.handleUID);
		if (info) {
			if (info->attributes & HANDLE_PFS) {
				/* contact the fs driver to release the handle */
				access = FSAccessFile(access, info->dir);
			}
			
			/* free memory and remove from handle hash */
			HashDelete(handleHash, access->handle.handleUID);
			free(info);
			access->errCode = VFS_SUCCESS;
		} else {
			/* handle not found */
			access->errCode = VFS_ERR_INVALID_HANDLE;
		}
	} else {
		access->handle.handleUID = GetNewHandle();
		info = (HANDLEINFO)malloc(sizeof(struct _HANDLEINFO));
		memset(info, 0, sizeof(struct _HANDLEINFO));
		
		if (info) {
			TUString path = CreateStringFromRMIStruct(access, objPath) ;
			fsDriver = GetFSNode(root, &path);
			
			if (fsDriver) {
				/* registered file system found - it's a physical fs */
				info->attributes |= HANDLE_PFS;
				if (path.length < access->_objPath.length) {
					// correct shifted pointer accordingly
					access->_objPath.string += access->_objPath.length - path.length;
					access->_objPath.length = path.length;
				}
				
				access = FSAccessFile(access, fsDriver);
				if (access->errCode == VFS_SUCCESS) {
					/* access a file or a folder? */
					if (access->mode != VFS_ACC_LIST) {
						info->attributes |= HANDLE_FILE;
					}
					info->dir = fsDriver;
					info->mode = access->mode;
					HashInsert(handleHash, access->handle.handleUID, (UINT32)info);
					access->errCode = VFS_SUCCESS;
				} else {
					/* file not found or similar - exact error has been set by FS driver */
					free(info);
					access->handle.handleUID = 0;
				}
			} else {
				/* VFS entry meant? */
				if (access->mode == VFS_ACC_LIST) {
					fsEntry = GetNodeByPath(root, path);
					if (fsEntry) {
						/* valid VFS node */
						info->dir = fsEntry->root;
						info->mode = access->mode;
						HashInsert(handleHash, access->handle.handleUID, (UINT32)info);
						access->errCode = VFS_SUCCESS;
					} else {
						/* path not found in VFS */
						free(info);
						access->handle.handleUID = 0;
						access->errCode = VFS_ERR_FILE_NOT_FOUND;
					}
				} else {
					free(info);
					access->handle.handleUID = 0;
					access->errCode = VFS_ERR_FILE_NOT_FOUND;
				}
			}
				
		} else {
			access->errCode = VFS_ERR_MEMORY;
			access->handle.handleUID = 0;
		}
	}
	
	CReturn((RMIMESSAGE)access, msgID, TRUE);
}

int KernelModuleEntry(char *arguments) {
	BarrierCreate("vfs", TRUE);
	
	CreateRootNode();
	
	handleHash = (PHASH)malloc(sizeof(HASH));
	HashCreate(handleHash, MAX_HANDLES);
	HashInit(handleHash);
	
	currHandle = 1;
	
	RMISetup(DRV_VFS, 6);
	RMIRegister(VFS_EXPORT_REGISTERNODE, RegisterNode, 255, FALSE, RTYPE_STRING, 2048);
	RMIRegister(VFS_EXPORT_MAKEDIR, MakeDir, 255, FALSE, RTYPE_STRING, 2048);
	RMIRegister(VFS_EXPORT_ACCESSFILE, AccessFile, 255, FALSE, RTYPE_STRING, 2048);
	RMIRegister(VFS_EXPORT_READFILEAT, ReadFileAt, 255, FALSE, RTYPE_STRING, sizeof(struct _FILESTREAM));
	RMIRegisterValue(VFS_EXPORT_FINDNEXTFILE, FindNextFile, 255, FALSE);
	RMIRegisterValue(VFS_EXPORT_FILEINFO, FileInfo, 255, FALSE);
	
	
	BarrierGo("vfs", TRUE);
	for(;;) Sleep();
}
