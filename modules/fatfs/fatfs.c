#include <raykernel.h>
#include <rmi/rmi.h>
#include <threads/sleep.h>
#include <kdisplay/kprintf.h>
#include <memory/memory.h>


#include "../fs/fs.h"
#include "fat16.h"
#include "fatfs.h"

#include "myfs.h"

#define SECTORSNEEDED(length) (((length) >> 9) + (((length) & 0x1ff) != 0))
#define MIN(a,b) ((a)<(b)?a:b)

UChar *longFileName;

UINT32 ParseHexNumber (char *commandline) {
	char *arguments;
	UINT32 number = 0;
	
	arguments = strchr(commandline, ' ');
	
	if (arguments == NULL) return 0;
	
	while (*++arguments) {
		if ((*arguments >= '0') && (*arguments <= '9')) {
			number *= 10;
			number += ((UINT32)*arguments) - '0';
		} else {
			return 0;
		}
	}
	return number;
}

unsigned char LFNchecksum(const unsigned char *fileName) {
	int i;
	unsigned char sum=0;
	
	for (i=11; i; i--) {
		sum = ((sum & 1)<<7) + (sum >> 1) + *fileName++;
	}
	return sum;
}

PCHAR_DISK CreateReadCommand(UINT64 sector, UINT32 count, MOUNT* mount) {
	PCHAR_DISK readCommand;
	readCommand = (PCHAR_DISK)AllocateMessageBuffer(sizeof(CHAR_DISK), RTYPE_STRING);
	
	if (!readCommand) return NULL;
	
	memcpy(&readCommand->mount, mount, sizeof(MOUNT));
	readCommand->sector = sector;
	readCommand->count = count;
	
	return readCommand;
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

FAT_FILE FindObject(PTUString fileName, UINT64 rootEntry, MOUNT_STORE* store) {
	unsigned char *data = 0;
	FAT16ENTRY *file;
	LONG_FILENAME *lfn;
	UINT32 sn, i;
	UINT8 checkSum = 0;
	int entryNum;
	FAT_FILE found;
	TUString currFileName;
	FAT_DRIVER *driver = store->fsSpecific;
	
	currFileName.string = longFileName;
	currFileName.length = 0;
	
	
scanDir:
	
	do {
		ReadSectors(CreateReadCommand(rootEntry, 1, &store->mount), &data);
		
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
									rootEntry = CLUSTERSECTOR(driver, file->firstClusterLow);
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
					currFileName.length = (sn + 1) * 13;
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
	found.index = 0;
	return found;
}

BOOL ReadFile(OPENFILE file, FILESTREAM stream, unsigned char *content) {
	UINT32 toRead = 0;
	FAT_DRIVER* driver = file->store->fsSpecific;
	UINT32 oneCluster = driver->sectorsPerCluster << 9;
	UINT32 startCluster;
	UINT32 offset;
	UINT32 toCopy, length;
	
	
	if (stream->position >= file->bufferPositionInFile) {
		// data might be in the cache
		if (stream->length <= file->bufferLength) {
			// data IS in the cache
			memcpy(content, file->clusterBuffer + (stream->position - file->bufferPositionInFile), stream->length);
			return TRUE;
		} else {
			// copy the data we already have to the output buffer
			memcpy(content, file->clusterBuffer + (stream->position - file->bufferPositionInFile), file->bufferLength);
			stream->position += file->bufferLength;
			stream->length -= file->bufferLength;
			content += file->bufferLength;
		}
		
	} 
	// we will have to read further sectors, calculate from where to read
	startCluster = (UINT32)stream->position / oneCluster;
	while (startCluster--) {
		file->currCluster = driver->clusterChain[file->currCluster];
	}
	
	offset = (UINT32)stream->position % oneCluster;
	toRead = stream->length;
	file->bufferPositionInFile = startCluster * 512;

	/* while not finished reading file */
	while (toRead) {
		// read whatever is needed but a maximum of one cluster
		length = MIN(SECTORSNEEDED(offset + toRead), driver->sectorsPerCluster);
		/* read data */
		FreeMessageBuffer((RMIMESSAGE)file->clusterBuffer);
		if (ReadSectors(CreateReadCommand(CLUSTERSECTOR(driver, file->currCluster),length, &file->store->mount), &file->clusterBuffer) == FS_ERR_SUCCESS) {
			
			// decide wether we have read to much (we can only read with a granularity of 512 bytes [one sector])
			file->bufferLength = length << 9;
			toCopy = MIN(toRead, file->bufferLength);
			
			/**
			* @todo memcpy is very inefficient!
			*/
			memcpy(content, file->clusterBuffer + offset, toCopy);
			offset = 0;
			toRead -= toCopy;
			content += toCopy;
			file->bufferPositionInFile += toCopy;
			
			file->currCluster = driver->clusterChain[file->currCluster];
			if (file->currCluster == 0xFFFF) {
				/* end of file */
				if (toRead) {
					return FALSE;
				} else {
					return TRUE;
				}
			}
		} else {
			KPrintf ("Reading sector failed!\n",0,0);
			return FALSE;
		}
	}
	KPrintf ("fatfs: Should not get here",0,0);
	return TRUE;
	
}
CALLBACK UnMountFileSystem(RMISERIAL ataDriver, INIT_MOUNT *unMount, UINT32 msgID) {
	// TODO: really unmount
	unMount->errCode = FS_ERR_CORRUPT;
	
	CReturn((RMIMESSAGE)unMount, msgID, TRUE);
	ThreadExit(0);
}

CALLBACK MountFileSystem(RMISERIAL ataDriver, INIT_MOUNT *toMount, UINT32 msgID) {
	UINT32 result;
	unsigned char *sector = 0, *fileTable = 0;
	FAT16_BOOT_SECTOR *bootSector;
	UINT32 fsSize;
	UINT32 FATsize;
	MOUNT_STORE* store;
	FAT_DRIVER *driver;
	PCHAR_DISK readCommand = CreateReadCommand(0, 1, &toMount->mountInfo);
	
	
	/* read first sector of partition */
	result = ReadSectors(readCommand, &sector);
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
			
			store = AddMount(toMount->mountInfo, ataDriver, sizeof(FAT_DRIVER));
			if (store) {
				driver = store->fsSpecific;
				driver->chainLength = FATsize;
				driver->rootSector = bootSector->reservedSectors + bootSector->numFATs * bootSector->sectorsPerFAT;
				driver->firstCluster = driver->rootSector + (bootSector->rootEntries * sizeof(FAT16ENTRY)+511)/512;
				driver->sectorsPerCluster = bootSector->sectorsPerCluster;
				
				/* read File Allocation Table */
				readCommand = CreateReadCommand(bootSector->reservedSectors, bootSector->sectorsPerFAT, &toMount->mountInfo);
				ReadSectors(readCommand, &fileTable);
				driver->clusterChain = (UINT16*)fileTable;
				
				TUString regPath = CreateStringFromRMIStruct(toMount, registerPath);
				TUString nodeName = CreateStringFromRMIStruct(toMount, nodeName);
				TUString addInfo = CreateStringFromRMIStruct(toMount, addInfo);
				
				switch (RegisterVFSNode(regPath, nodeName, addInfo, store->uID)) {
				case VFS_SUCCESS:
					toMount->errCode = 0;
					break;
				case VFS_ERR_OCCUPIED:
				default:
					/**
					 * @todo Test wether all variables are freed by 'fault injection' ;-) 
					 */
					free(driver->clusterChain);
					free(store);
					toMount->errCode = CHAR_ERR_SETUP_FAILED;
				}
				
			} else {
				toMount->errCode = FS_ERR_OUT_OF_MEMORY;
			}
			
			
			
		} else {
			/* not a valid partition */
			toMount->errCode = FS_ERR_CORRUPT;
		}
		FreeMessageBuffer((RMIMESSAGE)sector);
	} else {
		toMount->errCode = result;
	}
	
	CReturn((RMIMESSAGE)toMount, msgID, TRUE);
	ThreadExit(0);
}

CALLBACK AccessFile(RMISERIAL sender, ACCOBJ open, UINT32 msgID) {
	OPENFILE newFile;
	FAT_FILE found;
	
	/* open or close a file? */
	if (open->mode == VFS_ACC_CLOSE) {
		newFile = (OPENFILE)HashRetrieve(handleHash, open->handle.handleUID);
		if (newFile) {
			FreeMessageBuffer((RMIMESSAGE)newFile->clusterBuffer);
			free(newFile);
			HashDelete(handleHash, open->handle.handleUID);
			open->errCode = VFS_SUCCESS;
		} else {
			open->errCode = VFS_ERR_INVALID_HANDLE;
		}
	} else {
		/* open */
		
		newFile = (OPENFILE)malloc(sizeof(struct _OPENFILE));
		
		/* get mount info according to mount ID */
		newFile->store = (MOUNT_STORE*)HashRetrieve(mountHash, open->mountID);
		
		if (newFile->store) {
			TUString objPath = CreateStringFromRMIStruct(open, objPath);
			/* find file */
			if ((objPath.length == 0) || (objPath.length == 1 && objPath.string[0] == DIRSEPERATOR)) {
				// root entry
				newFile->file.firstClusterLow = 0;
				newFile->sector = ((FAT_DRIVER*)(newFile->store->fsSpecific))->rootSector;
				newFile->currIndex = 0;
				newFile->currCluster = 0;
				newFile->sectorClusterIndex = 0;
				// read one cluster, so it's read when needed (provides constant access times the NextFile-function)
				ReadSectors(CreateReadCommand(newFile->sector, ((FAT_DRIVER*)(newFile->store->fsSpecific))->sectorsPerCluster, &newFile->store->mount), &newFile->clusterBuffer);
				HashInsert(handleHash, open->handle.handleUID, (UINT32)newFile);
				
				open->errCode = VFS_SUCCESS;
			} else {
				found = FindObject(&objPath, ((FAT_DRIVER*)(newFile->store->fsSpecific))->rootSector, newFile->store);
	
				if (found.found) {
					if ((open->mode == VFS_ACC_LIST) && ((found.found + found.index)->attributes != FAT_ATTR_DIRECTORY)) {
						/* directory listing only possible on directories */
						free(newFile);
						open->errCode = VFS_ERR_NOT_A_DIR;
					} else {
						/* ready to list via FindNextFile */
						memcpy(&newFile->file, found.found + found.index, sizeof(FAT16ENTRY));
						newFile->sector = CLUSTERSECTOR(((FAT_DRIVER*)(newFile->store->fsSpecific)), newFile->file.firstClusterLow);
						newFile->currIndex = 0;
						newFile->bufferPositionInFile = 0;
						newFile->bufferLength = (((FAT_DRIVER*)(newFile->store->fsSpecific))->sectorsPerCluster) << 9;
						newFile->sectorClusterIndex = 0;
						newFile->currCluster = newFile->file.firstClusterLow;
						FreeMessageBuffer((RMIMESSAGE)found.found);
						ReadSectors(CreateReadCommand(newFile->sector, ((FAT_DRIVER*)(newFile->store->fsSpecific))->sectorsPerCluster, &newFile->store->mount), &newFile->clusterBuffer);
						HashInsert(handleHash, open->handle.handleUID, (UINT32)newFile);
						
						open->errCode = VFS_SUCCESS;
					}
				} else {
					free(newFile);
					open->errCode = VFS_ERR_FILE_NOT_FOUND;
				}
			}
		} else {
			/* mount ID not found */
			free(newFile);
			open->errCode = VFS_ERR_INVALID_MOUNT;
		}
	}
	
	/* return result */
	CReturn((RMIMESSAGE)open, msgID, TRUE);
	
}

static VFS_ATTRIBUTES VFSAttributes(UINT8 fatAttribs) {
	VFS_ATTRIBUTES vfs;
	
	vfs.is = VFS_ATTR_NOMETA | VFS_ATTR_READABLE;
	
	if (!(fatAttribs & FAT_ATTR_READONLY)) vfs.is |= VFS_ATTR_WRITABLE;
	if (fatAttribs & FAT_ATTR_DIRECTORY) vfs.is |= VFS_ATTR_DIRECTORY;
	
	return vfs;
}

/**
 * Returns the next file found in the fat entry of the current sector according to dir
 * @param dir Structure which stores information where last file read was and which folder (sector) to read
 */
static VFS_INFO NextFile(OPENFILE dir) {
	FAT16ENTRY *file = NULL;
	LONG_FILENAME *lfn;
	UINT32 sn, i;
	UINT8 checkSum = 0;
	FAT_DRIVER* driver = dir->store->fsSpecific;
	TUString currFileName;
	VFS_INFO info;
	
	// initialize
	currFileName.string = longFileName;
	currFileName.length = 0;
	
	
readFileName:
	// if last file last time
	if (dir->currIndex == 16) {
		dir->sectorClusterIndex++;
		dir->currIndex = 0;
		
		// do we have to read a new cluster?
		if ((dir->sectorClusterIndex % driver->sectorsPerCluster) == 0) {
			dir->sectorClusterIndex = 0;
			
			if (dir->currCluster) {
				// advance normally according to fragmentation
				dir->currCluster = driver->clusterChain[dir->currCluster];
				dir->sector = CLUSTERSECTOR(driver, dir->currCluster);
			} else {
				// normally root-directory
				dir->sector+= driver->sectorsPerCluster;
			}
			
			FreeMessageBuffer((RMIMESSAGE)dir->clusterBuffer);
			ReadSectors(CreateReadCommand(dir->sector, driver->sectorsPerCluster, &dir->store->mount), &dir->clusterBuffer);
		}
	}
	
	
	file = ((FAT16ENTRY*)(dir->clusterBuffer + (dir->sectorClusterIndex << 9))) + dir->currIndex++;
	// no file present (last file was before)
	if (!file->fileName[0]) {
		info = (VFS_INFO)AllocateMessageBuffer(sizeof(struct _VFS_INFO), RTYPE_STRING);
		info->errCode = VFS_ERR_FILE_NOT_FOUND;
		return info;
	}
	// check for LFN-Attribute
	if (file->attributes == 0x0f) {
		lfn = (LONG_FILENAME*)file;
		sn = FAT_LFN_NUMBER(lfn->sequenceNumber) - 1;
		memcpy(longFileName + (13 * sn), lfn->unicodeName1, 5 * sizeof(UINT16));
		memcpy(longFileName + (13 * sn + 5), lfn->unicodeName2, 6 * sizeof(UINT16));
		memcpy(longFileName + (13 * sn + 11), lfn->unicodeName3, 2 * sizeof(UINT16));
		if (lfn->sequenceNumber & FAT_LFN_LAST) {
			/* mark termination */
			longFileName[(sn + 1)  * 13] = 0;
			currFileName.length = (sn + 1) * 13;
			for (i = sn * 13; i < (sn + 1) * 13; i++) {
				if (longFileName[i] == 0) {
					currFileName.length = i;
					break;
				}
			}
		}
		// if a cecksum of the last file was set, check if it is identical (all LFN file-chunks have the same checksum)
		if (checkSum) {
			if (lfn->checkSum != checkSum) {
				KPrintf ("ERROR READING FAT! Checksum error!\n",0,0);
			}
		}
		checkSum = lfn->checkSum;
		goto readFileName;
	} else {
		
		// if last checksum of LFN does not correspond to the current entry, overwrite currFileName with 8.3 name
		if (checkSum != LFNchecksum(file->fileName)) {
			TerminatedUnicodeDOSName(file->fileName, file->extension, &currFileName);
		}
		checkSum = 0;
		if (file->fileName[0] != 0xE5) { // if not deleted
			info = (VFS_INFO)AllocateMessageBuffer(sizeof(struct _VFS_INFO) + sizeof(UChar) * currFileName.length, RTYPE_STRING);
			
			if (info == NULL) {
				KPrintf ("AllocateMessageBuffer() returned NULL!\n",0,0);
			}
			
			RMINewStructure(rmiStruct, info, info + 1);
			CreateRMIStructFromString(rmiStruct, info, name, currFileName);
			
			info->fileSize = file->fileSize;
			info->aOwner = VFSAttributes(file->attributes);
			VFS_DONT_USE_ATTRIBUTES(info->aGroup);
			VFS_DONT_USE_ATTRIBUTES(info->aOthers);
			VFS_NO_OWNER(info->owner);
			info->errCode = VFS_SUCCESS;
			return info;
		} else {
			goto readFileName;
		}
	
	}
}

CALLBACK FindNextFile(RMISERIAL sender, VFS_HANDLE handle, UINT32 msgID) {
	OPENFILE list;
	VFS_INFO fileInfo;
	
	list = (OPENFILE)HashRetrieve(handleHash, handle.handleUID);
	if (list) {
		CReturn((RMIMESSAGE)NextFile(list), msgID, TRUE);
	} else {
		fileInfo = (VFS_INFO)AllocateMessageBuffer(sizeof(struct _VFS_INFO), RTYPE_STRING);
		fileInfo->errCode = VFS_ERR_INVALID_HANDLE;
		CReturn((RMIMESSAGE)fileInfo, msgID, TRUE);
	}
}

CALLBACK FileInfo(RMISERIAL sender, VFS_HANDLE handle, UINT32 msgID) {
	OPENFILE toRead;
	VFS_INFO info;
	
	toRead = (OPENFILE)HashRetrieve(handleHash, handle.handleUID);
	if (toRead) {
		info = (VFS_INFO)AllocateMessageBuffer(sizeof(struct _VFS_INFO), RTYPE_STRING);
		if (info) {
			FAT16ENTRY *file = &toRead->file;
			info->fileSize = file->fileSize;
			info->aOwner = VFSAttributes(file->attributes);
			VFS_DONT_USE_ATTRIBUTES(info->aGroup);
			VFS_DONT_USE_ATTRIBUTES(info->aOthers);
			VFS_NO_OWNER(info->owner);
		} else {
			CReturn((RMIMESSAGE)0, msgID, FALSE);
			return;
		}
	} else {
		info = (VFS_INFO)AllocateMessageBuffer(sizeof(struct _VFS_INFO), RTYPE_STRING);
		info->errCode = VFS_ERR_INVALID_HANDLE;
	}
	
	CReturn((RMIMESSAGE)info, msgID, TRUE);
}

CALLBACK ReadFileAt(RMISERIAL sender, FILESTREAM file, UINT32 msgID) {
	OPENFILE toRead;
	VFS_BUFFER buffer;
	
	toRead = (OPENFILE)HashRetrieve(handleHash, file->handle.handleUID);
	if (toRead) {
		buffer = (VFS_BUFFER)AllocateMessageBuffer(sizeof(struct _VFS_BUFFER) + file->length, RTYPE_STRING);
		if (buffer) {
			buffer->bufferLength = file->length;
			if (ReadFile(toRead, file, (unsigned char*)(buffer + 1))) {
				buffer->errCode = FS_ERR_SUCCESS;
				buffer->buffer = (void*)sizeof(struct _VFS_BUFFER);
			} else {
				buffer->errCode = FS_ERR_READ;
				buffer->buffer = NULL;
			}
			FreeMessageBuffer((RMIMESSAGE)file);
		} else {
			FreeMessageBuffer((RMIMESSAGE)file);
			CReturn((RMIMESSAGE)0, msgID, FALSE);
			return;
		}
	} else {
		FreeMessageBuffer((RMIMESSAGE)file);
		buffer = (VFS_BUFFER)AllocateMessageBuffer(sizeof(struct _VFS_BUFFER), RTYPE_STRING);
		buffer->errCode = VFS_ERR_INVALID_HANDLE;
	}
	
	CReturn((RMIMESSAGE)buffer, msgID, TRUE);
	
}

RAYENTRY KernelModuleEntry(char *connectTo) {
	UINT32 charDevice;
	
	FSInit(FS_SERIAL, 10, 256);
	
	charDevice = ParseHexNumber(connectTo);
	if (charDevice) {
		
		longFileName = malloc(261 * sizeof(UChar));

		WaitForCharDevice(charDevice);
		
		if (RegisterDriver(charDevice, 0x06, "fat16")) {
		} else {
			KPrintf ("could not register FAT-driver!\n",0,0);
		}
	} else {
		exit(1000);
	}
	
	
	for(;;) {
		Sleep();
	}
}
