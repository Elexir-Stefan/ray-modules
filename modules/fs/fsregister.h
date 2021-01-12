#ifndef _FS_REGISTER_H
#define _FS_REGISTER_H

#include <unicode.h>

typedef struct {
	RMISERIAL driverID;
	char fileSystemName[12];
} FS_DRIVER;

typedef struct {
	UINT8 systemID;
	char fileSystemName[12];
} FS_REGISTER;

typedef struct {
	UINT8 hardDrive;
	UINT8 partition;
	UINT8 forceFSType;
	UINT8 reserved;
} __attribute__((packed)) MOUNT;

typedef struct {
	UINT64 firstSector;
	UINT64 partitionLength;
	
	MOUNT mountInfo;
	
	char identifier[32];
	UINT32 errCode;
	UINT32 extErrInfo;
	
	TUString _registerPath;
	TUString _nodeName;
	TUString _addInfo;
} __attribute__ ((packed)) INIT_MOUNT;

#define FS_AUTODETECT	0

#endif
