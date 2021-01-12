#include "../../../kernel/include/ray/typedefs.h"
#include "../../../kernel/include/unicode.h"
typedef UINT32 RMIMESSAGE;
typedef UINT32 RMISERIAL;

#include "../../fs/fserr.h"
#include "../../fs/fsregister.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../fat16.h"

#define CLUSTERSECTOR(n) (driver.firstSector + driver.rootSector + 16 + n * driver.sectorsPerCluster)

FILE *fh;

FAT_DRIVER driver;
UChar *longFileName;



void CReturn(RMIMESSAGE msg, UINT32 msgID, BOOL isMemory);
void FreeMessageBuffer(RMIMESSAGE address);

PTUString ConvertASCIItoTUString(char *input) {
	UINT32 length, i;
	PTUString limitedUnicodeString;
	UChar *string;
	
	length = strlen(input);
	
	limitedUnicodeString = (PTUString)malloc(sizeof(TUString) + length * sizeof(UChar));
	string = (UChar*)(limitedUnicodeString + 1);
	
	limitedUnicodeString->string = string;
	limitedUnicodeString->length = length;
	
	for (i = 0; i < length; i++) {
		string[i] = input[i];
	}
	
	return limitedUnicodeString;
}

char *ConvertTUStringToASCII(PTUString unicode) {
	char *ascii;
	UINT32 i;
	
	ascii = (char*)malloc(unicode->length + 1);
	
	for (i = 0; i < unicode->length; i++) {
		ascii[i] = (unicode->string[i] & 0xFF);
	}
	ascii[unicode->length] = '\0';
	
	return ascii;
}

void FreeMessageBuffer(RMIMESSAGE address) {
	//printf ("Freeing address @%x\t[now #%u allocs]\n", address, memoryUsage);
	free((void*)address);
}

void CReturn(RMIMESSAGE msg, UINT32 msgID, BOOL isMemory) {
	/*if (isMemory) {
		printf ("\tSending memory %x msg to %x.\n", msg, msgID);
	} else {
		printf ("\tReturning value %u to %x\n", msg, msgID);
	}*/
}

UINT32 ReadSectors(UINT8 hardDrive, UINT64 sector, UINT32 length, unsigned char **buffer) {
	*buffer = malloc(512 * length);
	fseek(fh, sector * 512, SEEK_SET);
	fread(*buffer, 512, length, fh);
	
	return FS_ERR_SUCCESS;
}

void KPrintf(char *string, UINT32 argument1, UINT32 argument2) {
	printf(string, argument1, argument2);
}


void MountFileSystem(RMISERIAL ataDriver, INIT_MOUNT *toMount, UINT32 msgID) {
	UINT32 result;
	unsigned char *sector = 0, *fileTable = 0;
	FAT16_BOOT_SECTOR *bootSector;
	UINT32 fsSize;
	UINT32 FATsize;
	
	result = ReadSectors(toMount->mountInfo.hardDrive, toMount->firstSector, 1, &sector);
	if (result == FS_ERR_SUCCESS) {
		if ((sector[254] = 0xAA) && (sector[255] = 0x55)) {
			bootSector = (FAT16_BOOT_SECTOR*)sector;
			
			
			memcpy(toMount->identifier, bootSector->volumeName, 11);
			toMount->identifier[11] = '\0';
			if (bootSector->diskSectorCount) {
				fsSize = bootSector->diskSectorCount;
			} else {
				fsSize = bootSector->sectorsTotal;
			}
			FATsize = fsSize / bootSector->sectorsPerCluster;
			fsSize *= bootSector->bytesPerSector;
			KPrintf ("Mounted '%s' (%u KB)\n", (UINT32)toMount->identifier, fsSize>>10);
			KPrintf ("FAT has %u entries starting at %u\n", FATsize, bootSector->reservedSectors + toMount->firstSector);
			
			driver.chainLength = FATsize;
			driver.rootSector = bootSector->reservedSectors + bootSector->numFATs * bootSector->sectorsPerFAT;
			
			driver.firstSector = toMount->firstSector;
			driver.partition = toMount->mountInfo.partition;
			driver.hardDrive = toMount->mountInfo.hardDrive;
			driver.sectorsPerCluster = bootSector->sectorsPerCluster;
			
			ReadSectors(toMount->mountInfo.hardDrive, toMount->firstSector + bootSector->reservedSectors, FATsize / 512, &fileTable);
			driver.clusterChain = (UINT16*)fileTable;
			
			toMount->errCode = 0;
			
		} else {
			/* not a valid partition */
			toMount->errCode = FS_ERR_CORRUPT;
		}
		FreeMessageBuffer((RMIMESSAGE)sector);
	} else {
		toMount->errCode = result;
	}
	
	CReturn((RMIMESSAGE)toMount, msgID, TRUE);
}

unsigned char LFNchecksum(const unsigned char *fileName) {
	int i;
	unsigned char sum=0;
	
	for (i=11; i; i--) {
		sum = ((sum & 1)<<7) + (sum >> 1) + *fileName++;
	}
	return sum;
}

void Unicode2ASCII(UINT16 *unicodeName, unsigned char *ASCIIname) {
	while ((*ASCIIname++ = ((*unicodeName++) & 0xFF)));
	
}

void TerminatedUnicodeDOSName(unsigned char *fileName, unsigned char *extension, PTUString completeName) {
	int i;
	completeName->length = 0;
	
	for (i = 0; i < 8; i++) {
		if(*fileName != ' ') {
			completeName->string[i] = *fileName++;
			completeName->length++;
		} else {
			break;
		}
	}
	if (*extension != ' ') {
		completeName->string[completeName->length++]  = '.';
	} else {
		completeName->string[completeName->length] = 0;
		return;
	}
	for (i = 0; i < 3; i++) {
		if(*extension != ' ') {
			completeName->string[completeName->length++] = *extension++;
		} else {
			break;
		}
	}
	completeName->string[completeName->length] = 0;
}

BOOL DifferentPrefix(PTUString first, PTUString second) {
	UINT32 length = first->length;
	
	while(length--) {
		if (first->string[length] != second->string[length]) return TRUE;
	}
	return FALSE;
}

FAT_FILE FindObject(PTUString fileName, UINT64 rootEntry) {
	unsigned char *data = 0;
	FAT16ENTRY *file;
	LONG_FILENAME *lfn;
	UINT32 sn, i;
	UINT8 checkSum = 0;
	int entryNum;
	FAT_FILE found;
	TUString currFileName;
	
	currFileName.string = longFileName;
	currFileName.length = 0;
	
	
scanDir:
	
	do {
		ReadSectors(driver.hardDrive, rootEntry, 1, &data);
		
		for (entryNum = 0; entryNum < 16; entryNum++) {
			file = ((FAT16ENTRY*)data) + entryNum;
			if (!file->fileName[0]) {
				break;
			}
			if (file->attributes != 0x0f) {
				
				if (checkSum != LFNchecksum(file->fileName)) {
					TerminatedUnicodeDOSName(file->fileName, file->extension, &currFileName);
				}
				checkSum = 0;
				if (file->fileName[0] != 0xE5) { /* if not deleted */
					if (fileName->length >= currFileName.length) {
						
						if (DifferentPrefix(&currFileName, fileName)) {
							continue;
						} else {
							/* prefix match */
							if (fileName->length == currFileName.length) {
								/* file or dir found */
									
								found.found = (FAT16ENTRY*)data;
								found.index = entryNum;
								return found;
							} else {
								if (fileName->string[currFileName.length] == DIRSEPERATOR) {
									rootEntry = CLUSTERSECTOR(file->firstClusterLow);
									currFileName.length++;
									fileName->string += currFileName.length;
									fileName->length -= currFileName.length;
									FreeMessageBuffer((RMIMESSAGE)data);
									goto scanDir;
								} else {
									continue;
								}
							}
						}
					} else {
						continue;
					}
				}
			} else {
				lfn = (LONG_FILENAME*)file;
				sn = FAT_LFN_NUMBER(lfn->sequenceNumber) - 1;
				memcpy(longFileName + (13 * sn), lfn->unicodeName1, 5 * sizeof(UINT16));
				memcpy(longFileName + (13 * sn + 5), lfn->unicodeName2, 6 * sizeof(UINT16));
				memcpy(longFileName + (13 * sn + 11), lfn->unicodeName3, 2 * sizeof(UINT16));
				if (lfn->sequenceNumber & FAT_LFN_LAST) {
					/* mark termination */
					longFileName[(sn + 1)  * 13] = 0;
					for (i = sn * 13; i < (sn + 1) * 13; i++) {
						if (longFileName[i] == 0) {
							currFileName.length = i;
							break;
						}
					}
				}
				if (checkSum) {
					if (lfn->checkSum != checkSum) {
						KPrintf ("ERROR READING FAT!\n",0,0);
					}
				}
				checkSum = lfn->checkSum;
			}
		}
		FreeMessageBuffer((RMIMESSAGE)data);
		rootEntry++;
	} while (entryNum == 16);
	
	/* not found */
	
	found.found = FINDRESULT(FIND_NOT_FOUND);
	return found;
}

BOOL ReadFile(UINT32 clusterNumber, UINT32 fileLength, unsigned char **content) {
	unsigned char *cluster;
	UINT32 currLength = 0;
	UINT32 length;
	UINT32 oneCluster = 512 * driver.sectorsPerCluster;
	
	*content = malloc(fileLength);
	
	if(content) {
		/* while not finished reading file */
		while (currLength < fileLength) {
			/* read cluster */
			if (ReadSectors(driver.hardDrive, CLUSTERSECTOR(clusterNumber), driver.sectorsPerCluster, &cluster) == FS_ERR_SUCCESS) {
				/* last cluster? */
				
				if (currLength + oneCluster > fileLength) {
					length = fileLength % oneCluster;
				} else {
					length = oneCluster;
				}
				memcpy(*content + currLength, cluster, length);
				currLength += oneCluster;
				free(cluster);
				clusterNumber = driver.clusterChain[clusterNumber];
				if (clusterNumber == 0xFFFF) {
					/* end if file */
					if (currLength < fileLength) {
						free(content);
						return FALSE;
					} else {
						return TRUE;
					}
				}
			} else {
				free(content);
				printf ("Reading sector failed!\n");
				return FALSE;
			}
		}
		return FALSE;
	} else {
		printf ("Memory allocation failed!\n");
		return FALSE;
	}
	
	
}

int main (int argc, char **argv) {
	INIT_MOUNT mountCommand;
	UINT32 offset;
	FAT_FILE file;
	
	if (argc != 3) {
		printf ("USAGE: fat IMAGEFILE OFFSET\n");
		printf ("\tIMAGEFILE\tFilename of a fat image (partition or harddirve, see offset)\n");
		printf ("\tOFFSET\tOffset in sectors in image (harddirve partitions usualy 63)\n");
		exit(1);
	}
	
	offset = atoi(argv[2]);
	
	fh = fopen(argv[1], "r");
	
	if (!fh) {
		printf ("Error opening %s!\n", argv[1]);
		exit(1);
	}
	
	mountCommand.firstSector = offset;
	mountCommand.mountInfo.hardDrive = 0;
	mountCommand.mountInfo.partition = 0;
	
	MountFileSystem(1234, &mountCommand, 9876);
	
	longFileName = malloc(261 * sizeof(UINT16));
	
	file = FindObject(ConvertASCIItoTUString("ReactOS/SYSTEM32/DRIVERS/ETC/SERVICES"), driver.rootSector + driver.firstSector);
	if (file.found) {
		if (file.found[file.index].attributes & FAT_ATTR_DIRECTORY) {
			printf ("Cannot read file content because it's a directory!\n");
		} else {
			unsigned char *content;
			printf ("Content of %8s:\n", file.found[file.index].fileName);
			
			/* read file content */
			if (ReadFile(file.found[file.index].firstClusterLow, file.found[file.index].fileSize, &content)) {
				printf ("%s\n", content);
				free(content);
			} else {
				printf ("Error reading file!\n");
			}
		}
		
		
	} else {
		printf ("File not found!\n");
	}
	
	free(file.found);
	
	free(longFileName);
	
	fclose(fh);
	
	return 0;
}
