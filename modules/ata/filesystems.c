#include <raykernel.h>
#include <ray/arguments.h>
#include <drivers/dmesg/dmesg.h>
#include <tdm/io.h>
#include <tdm/int.h>
#include <threads/sleep.h>
#include <threads/threads.h>
#include <rmi/rmi.h>
#include <pid/pids.h>
#include <memory/memory.h>
#include <string.h>

#include "ataio.h"
#include "drives.h"

#include "../fs/charimports.h"
#include "../fs/fserr.h"
#include "../fs/charerr.h"
#include "../fs/exports.h"

extern volatile FS_DRIVER *fileSystems;
extern volatile HDD *storage;

static INIT_MOUNT* MountFS(PARTITION_INFO *partition, INIT_MOUNT* toMount, volatile FS_DRIVER *fs);

CALLBACK RegisterFileSystem(RMISERIAL sender, FS_REGISTER *fsData, UINT32 msgID) {
	if (fsData->systemID < 255) {
		if (fileSystems[fsData->systemID].driverID == 0) {
			fileSystems[fsData->systemID].driverID = sender;
			memcpy(fileSystems[fsData->systemID].fileSystemName, fsData->fileSystemName, 12);
			
			// diagnostic messages
			char regMsg[49] = "Registered filesystem with ID 0x00 (            )";
			char hex[16] = {"0123456789ABCDEF"};
			
			char* idString = regMsg + 32;
			char* nameString =regMsg + 36;
			
			*idString = hex[fsData->systemID / 16];
			*(idString+1) = hex[fsData->systemID % 16];
			
			memcpy(nameString, fsData->fileSystemName, 12);
			strcat(regMsg, ")");
			
			dmesg(regMsg);
			
			FreeMessageBuffer((RMIMESSAGE)fsData);
			CReturn((RMIMESSAGE)CHAR_ERR_SUCCESS, msgID, FALSE);
		} else {
			/* already registered */
			FreeMessageBuffer((RMIMESSAGE)fsData);
			CReturn((RMIMESSAGE)CHAR_ERR_FS_ALREADY_REGISTERED, msgID, FALSE);
		}
	} else {
		/* out of bounds */
		FreeMessageBuffer((RMIMESSAGE)fsData);
		CReturn((RMIMESSAGE)CHAR_ERR_FS_NOT_SUPPORTED, msgID, FALSE);
	}
	ThreadExit(0);
}

static INIT_MOUNT* MountFS(PARTITION_INFO *partition, INIT_MOUNT* mountParam, volatile FS_DRIVER *fs) {
	RMIMESSAGE result;
	
	mountParam->firstSector = partition->partitionStart;
	mountParam->partitionLength = partition->partitionSize;
	mountParam->errCode = 0;
	strcpy(mountParam->identifier, "(unknown)");
	
	
	switch (RMCallMessage(fs->driverID, FS_EXPORT_MOUNT_FS, (RMIMESSAGE)mountParam , &result, RTYPE_STRING, 2048)) {
	case RMI_SUCCESS:
		return (INIT_MOUNT*)result;
		break;
	case RMI_NOT_SUPPORTED:
		mountParam->errCode = CHAR_ERR_DRIVER_UNEXPECTED_END;
		return mountParam;
		break;
	default:
		mountParam->errCode = CHAR_ERR_MOUNTING;
		return mountParam;
	}
}


CALLBACK MountPartition(RMISERIAL sender, INIT_MOUNT* toMount, UINT32 msgID) {
	volatile FS_DRIVER *fs;

	if ((toMount->mountInfo.hardDrive < NUM_HDD) && (storage[toMount->mountInfo.hardDrive].isPresent)) {
		if (toMount->mountInfo.partition <= storage[toMount->mountInfo.hardDrive].numPartitions) {
			UINT32 systemID = storage[toMount->mountInfo.hardDrive].partitions[toMount->mountInfo.partition].systemID;
			fs = &fileSystems[systemID];
			
			if (fs->driverID) {
				CReturn((RMIMESSAGE)MountFS(storage[toMount->mountInfo.hardDrive].partitions + toMount->mountInfo.partition, toMount, fs), msgID, TRUE);
			} else {
				toMount->errCode = CHAR_ERR_UNSUPPORTED_FS;
				toMount->extErrInfo = systemID;
				CReturn((RMIMESSAGE)toMount, msgID, TRUE);
			}
			
		} else {
			toMount->errCode = CHAR_ERR_INVALID_PARTITION;
			CReturn((RMIMESSAGE)toMount, msgID, TRUE);
		}
	} else {
		toMount->errCode = CHAR_ERR_INVALID_HDD;
		CReturn((RMIMESSAGE)toMount, msgID, TRUE);
	}
	ThreadExit(0);
}

CALLBACK GetClaimedDrivers(RMISERIAL sender, UINT32 dummy, UINT32 msgID) {
	RMIMESSAGE drivers;
	
	drivers = (RMIMESSAGE)AllocateMessageBuffer(256 * sizeof(FS_DRIVER), RTYPE_STRING);
	if (drivers) {
		CReturn((RMIMESSAGE)CHAR_ERR_OUT_OF_MEMORY, msgID, FALSE);
	} else {
		memcpy((POINTER)drivers, fileSystems, 256 * sizeof(FS_DRIVER));
		CReturn(drivers, msgID, TRUE);
	}
	ThreadExit(0);
}
