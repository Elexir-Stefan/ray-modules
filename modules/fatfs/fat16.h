#ifndef _FAT_H
#define _FAT_H

#define FAT_ATTR_READONLY	0x01
#define FAT_ATTR_HIDDEN		0x02
#define FAT_ATTR_SYSTEM		0x04
#define FAT_ATTR_VOLUME		0x08
#define FAT_ATTR_DIRECTORY	0x10
#define FAT_ATTR_ARCHIVE	0x20

#define FAT_LFN_LAST		0x40
#define FAT_LFN_NUMBER(n) (n & 0x1F)

#define CLUSTERSECTOR(driver, n) (driver->firstCluster + (n-2) * driver->sectorsPerCluster)

typedef enum {
	Floppy1440K= 0xF0,
	Floppy720K = 0xF9,
	Floppy360K = 0xFD,
	Floppy320K = 0xFF,
	Floppy180K = 0xFC,
	Floppy160K = 0xFE,
	HardDrive  = 0xF8
} __attribute__((packed)) MEDIA_TYPE;


typedef struct {
	UINT8  bootJump[3];
	UINT8  OEMname[8];
	UINT16 bytesPerSector;
	UINT8  sectorsPerCluster;
	UINT16 reservedSectors;
	UINT8  numFATs;
	UINT16 rootEntries;
	UINT16 diskSectorCount;
	MEDIA_TYPE mediaType;
	UINT16 sectorsPerFAT;
	UINT16 sectorsPerCylinder;
	UINT16 numHeads;
	UINT32 numHiddenSectors;
	UINT32 sectorsTotal;
	UINT8  physicalDiskNumber;
	UINT8  currentHead;
	UINT8  signature;
	UINT32 serialNumber;
	char   volumeName[11];
	UINT8  systemID[8];
} __attribute__((packed))  FAT16_BOOT_SECTOR;

typedef struct {
	unsigned char   fileName[8];
	unsigned char   extension[3];
	UINT8  attributes;
	UINT8  reserved;
	UINT8  creationSecond;
	UINT32 creationTime;
	UINT16 lastAccesUnused;
	UINT16 firstClusterHigh;
	UINT32 lastAccesTime;
	UINT16 firstClusterLow;
	UINT32 fileSize;
} __attribute__((packed)) FAT16ENTRY;

typedef struct {
	UINT8  sequenceNumber;
	UINT16 unicodeName1[5];
	UINT8  attributes;
	UINT8  reserved1;
	UINT8  checkSum;
	UINT16 unicodeName2[6];
	UINT16 firstCluster;
	UINT16 unicodeName3[2];
} __attribute__((packed)) LONG_FILENAME;

/* driver specific */
typedef struct {
	UINT32 chainLength;
	UINT16 *clusterChain;
	UINT32 rootSector;
	UINT8  sectorsPerCluster;
	UINT32 firstCluster;
	
} FAT_DRIVER;

#define FINDRESULT(n) (void*)(n)
#define ISRESULT(n) (UINT32)(n)

#define FIND_NOT_FOUND	0
#define FIND_IS_DIR	1

#define DIRSEPERATOR	'/'

typedef struct {
	FAT16ENTRY *found;
	UINT8 index;
} FAT_FILE;

#endif
