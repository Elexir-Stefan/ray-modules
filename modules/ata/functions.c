#include <raykernel.h>
#include <drivers/dmesg/dmesg.h>
#include <rmi/rmi.h>
#include <threads/sleep.h>
#include <memory/memory.h>
#include <string.h>
#include <pid/pids.h>

#include "../fs/charerr.h"
#include "../fs/fsregister.h"
#include "../fs/chardev.h"

#include "drives.h"
#include "lowlevel.h"


extern volatile HDD *storage;

static void RegisterPartitions(volatile HDD *controller, UINT64 sector, BOOL recursive);

UINT32 DetectDrives() {
	UINT32 numDrives = 0;
	UINT32 ctrl;
	
	InitializeControllers();
	
	for (ctrl = 0; ctrl < NUM_HDD; ctrl++) {
		numDrives += DetectController(storage + ctrl);
	}
	
	return numDrives;
}

static void RegisterPartitions(volatile HDD *controller, UINT64 sector, BOOL recursive) {
	UINT8 *data;
	MBR *partitionTable;
	UINT32 i;
	
	char partitionsFound[47] = {"ata: Drive % Partition % has file system ID %%"};
	char hex[16] = {"0123456789ABCDEF"};
	char *drivePtr = partitionsFound + 11;
	char* partitionPtr = partitionsFound + 23;
	char* systemIDPtr = partitionsFound + 44;

	
	data = (UINT8*)malloc(SECTOR_SIZE);
	
	if(data) {
		if (ReadSector(controller, sector, 1, data)) {
			// check for validity
			if ((data[510] == 0x55) && (data[511] == 0xAA)) {
				partitionTable = (MBR*)(data + 0x1BE);
				for (i = 0; i < 4; i++) {
					if (partitionTable->partition[i].type) {
						if ((partitionTable->partition[i].type == 0x0f) && (recursive)) {
							RegisterPartitions(controller, partitionTable->partition[i].sectorStart, FALSE);
						} else {
							controller->partitions[controller->numPartitions].bootable = (partitionTable->partition[i].bootable == 0x80);
							
							controller->partitions[controller->numPartitions].partitionStart = partitionTable->partition[i].sectorStart;
							controller->partitions[controller->numPartitions].partitionSize = partitionTable->partition[i].partitionSize;
							controller->partitions[controller->numPartitions].systemID = partitionTable->partition[i].type;
							
							*drivePtr = '0' + controller->drive;
							*partitionPtr = '0' + controller->numPartitions;
							*systemIDPtr = hex[partitionTable->partition[i].type / 16];
							*(systemIDPtr+1) =  hex[partitionTable->partition[i].type % 16] ;
							dmesg(partitionsFound);
							
							controller->numPartitions++;
						}
					}
				}
			}
		} else {
			dmesg ("ata: error reading sector - cannot read partition info");
		}
		free(data);
	}
}

void SavePartitionLayout() {
	UINT32 hdd;
	
	for (hdd = 0; hdd < NUM_HDD; hdd++) {
		if (storage[hdd].isPresent) {
			switch (hdd) {
			case 0: dmesg("ata: Primary master found");break;
			case 1: dmesg("ata: Primary slave found");break;
			case 2: dmesg("ata: Secondary master found");break;
			case 3: dmesg("ata: Secondary slave found");break;
			}
			RegisterPartitions(storage + hdd, 0, TRUE);
		}
	}
	FireCharBarrier(DRV_ATA);
}

CALLBACK CHARReadSector(RMISERIAL sender, PCHAR_DISK disk, UINT32 msgID) {
	UINT8 *buffer;
	
	if (disk->mount.hardDrive < NUM_HDD) {
		if (storage[disk->mount.hardDrive].isPresent) {
			if (disk->mount.partition < storage[disk->mount.hardDrive].numPartitions) {
			
				/* reserve space */
				buffer = (UINT8*)AllocateMessageBuffer(512 * disk->count, RTYPE_STRING);
				if (buffer) {
					/* correct position relatively to start of partition*/
					disk->sector += storage[disk->mount.hardDrive].partitions[disk->mount.partition].partitionStart;
					/* say: We want to get data */
					BOOL success = ReadSector(storage + disk->mount.hardDrive, disk->sector , disk->count, buffer);
					FreeMessageBuffer((RMIMESSAGE)disk);
					
					if (success) {
						// return the data
						CReturn((RMIMESSAGE)buffer, msgID, TRUE);
					} else {
						// as we don't send the buffer and don't need the data we have to free the allocated buffer
						free(buffer);
						CReturn((RMIMESSAGE)CHAR_ERR_READ_ERROR, msgID, FALSE);
					}
				} else {
					/* out of memory */
					CReturn((RMIMESSAGE)CHAR_ERR_OUT_OF_MEMORY, msgID, FALSE);
				}
			} else {
				CReturn((RMIMESSAGE)CHAR_ERR_INVALID_PARTITION, msgID, FALSE);
			}
		
		} else {
			CReturn((RMIMESSAGE)CHAR_ERR_INVALID_HDD, msgID, FALSE);
		}
	} else {
		CReturn((RMIMESSAGE)CHAR_ERR_INVALID_HDD, msgID, FALSE);
	}
}
