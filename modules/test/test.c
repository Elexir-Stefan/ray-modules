#include <raykernel.h>
#include <kdisplay/kprintf.h>
#include <threads/sleep.h>
#include <threads/threads.h>
#include <rmi/rmi.h>
#include <ray/arguments.h>
#include <memory/memory.h>
#include <string.h>
#include <debug/debug.h>

#include <pid/pids.h>
#include "../i8042/exports.h"
#include "../dmesg/exports.h"

#include "../fs/charexports.h"
#include "../fs/fsregister.h"
#include "../fs/charerr.h"

#include "../i8042/usrkbd.h"
#include "../pci/pcitypes.h"

#include "../vfs/exports.h"
#include "../vfs/vfstypes.h"

#include "vendors.h"
#include "elf.h"

#include <drivers/keyboard/raykeybind.h>

#define EXPORT_GET_CH	0

#define CPUIDread(n,w,x,y,z) asm("cpuid" : "=a" (w), "=b" (x), "=c" (y), "=d" (z) : "0" (n))
#define CPUextended 0x80000000UL
#define CPUmodelName 0x80000002UL

#define READTSC(var) __asm__ __volatile__("rdtsc":"=a"(var)::"edx")

#define StringDirAttrib(var, attr, ind, chr) if (var & attr) acr[ind] = chr; else acr[ind] = '-';

char *inputBuf;
UINT8 bufPos = 0;

IPLOCK sharedLock;

// Benchmark stuff
typedef void(*DoFileFunc)(unsigned char*, UINT32);
UINT64 yieldBenchmarkSum = 0L;
UINT64 yieldBenchmarkTime;
UINT64 processCreationSum = 0L;
UINT32 processTSCResult;

volatile UINT32 sharedResource = 0;
UINT32 tickerLine = 5;
UINT32 tickerPause = 50;
PID_HANDLE tickerHandles[25];
UINT32 numTickers = 0;

volatile UINT32 executionCounter[3];


void printCPUInfo() {
	UINT32 vendorString[13];

	CPUIDread(0, vendorString[0], vendorString[1], vendorString[3], vendorString[2]);
	vendorString[4] = 0;

	KPrintf ("CPU: %s ", (UINT32)vendorString+4);

	CPUIDread(CPUextended, vendorString[0], vendorString[1], vendorString[2], vendorString[3]);

	CPUIDread(CPUmodelName, vendorString[0], vendorString[1], vendorString[2], vendorString[3]);
	CPUIDread(CPUmodelName+1, vendorString[4], vendorString[5], vendorString[6], vendorString[7]);
	CPUIDread(CPUmodelName+2, vendorString[8], vendorString[9], vendorString[10], vendorString[11]);
	vendorString[12] = 0;
	KPrintf ("(Model %s)\n", (UINT32)vendorString);
}

void PrintPCIResource(PCI_RESOURCES *resource) {
	switch (resource->IOType) {
	case PCIResourceIsIO:
		KPrintf(" -    IO: %x - %x\n", resource->range.rangeIO.IOStart, resource->range.rangeIO.IOStart + resource->range.rangeIO.IOLength - 1);
		break;
	case PCIResourceIsMemory:
		KPrintf(" -Memory: %x - %x ", resource->range.rangeMem.MemoryStart, resource->range.rangeMem.MemoryStart + resource->range.rangeMem.MemoryLength - 1);
		switch (resource->range.rangeMem.MemoryType) {
			case PCIMemTypeStandard:
				KPrintf("(Standard)\n");
				break;
			case PCIMemTypePrefetchable:
				KPrintf("(Prefetchable)\n");
				break;
		}
		break;
	case PCIResourceUnused:
		/* resource not used */
		break;
	}
}

void ShowPCIdevices() {
	RMIMESSAGE devices;
	PCIDeviceArray *pciDevices;
	int i, j;
	int list;
	PCIDevice *deviceList;

	KPrintf ("PCI device list:\n");
	switch (RMCall(DRV_PCI, 0, 0, &devices, RTYPE_STRING, 4096)) {
			case RMI_SUCCESS:
				if (devices) {
					pciDevices = (PCIDeviceArray*)devices;
					deviceList = RMIGetOffset(pciDevices, PCIDevice, list);
					for (i = 0; i < pciDevices->numPCIDevices; i++) {
						for (list = 0; list < PCI_DEVTABLE_LEN; list++) {
							if ((PciDevTable[list].VenID == deviceList[i].configSpace.PCIHeader.vendorID) && (PciDevTable[list].DevID == deviceList[i].configSpace.PCIHeader.deviceID)) {
								KPrintf ("%u: %s\n", i, (UINT32)PciDevTable[list].CardName);
								for (j = 0; j < 6; j++) {
									PrintPCIResource(&deviceList[i].resource[j]);
								}
								if (deviceList[i].configSpace.PCIHeader.interruptLine) {
									KPrintf (" -   IRQ: %d\n", deviceList[i].configSpace.PCIHeader.interruptLine);
								}
								break;
							}
						}
						if (list == PCI_DEVTABLE_LEN) {
							/* end of list - not found */
							KPrintf("%u: Unknown device %x\n", i, deviceList[i].configSpace.PCIHeader.vendorID);
						}
					}
					FreeMessageBuffer(devices);
				} else {
					KPrintf ("PCI database encountered error!\n");
				}
				break;
			case RMI_NOT_SUPPORTED:
				KPrintf ("PCI driver seems not to be loaded!\n");
				break;
			default:
				KPrintf ("An error occured while contacting the pci driver.\n");
			}
}

SINT32 IPLockThread(UINT32 argument) {
	int i, j;

	
	for (i = 0;i < 100;i++) {
		SemaphoreEnter(sharedLock);
		for (j = 0; j < 100; j++) {
			KPrintf ("[%u] ",argument);
			sharedResource++;
		}
		SemaphoreLeave(sharedLock);
		
	}
	
	SEMAPHORE_STATUS status = SemaphoreStatus(sharedLock);
	KPrintf ("Thread #%u: sharedResource is now %u -> lock status is: ", argument, sharedResource, status);
	switch(status) {
	    case SEMAPHORE_READY:
		KPrintf("SEMAPHORE_READY\n");
		break;
	    case SEMAPHORE_IN_USE:
		KPrintf("SEMAPHORE_IN_USE\n");
		break;
	    case SEMAPHORE_ILLEGAL:
		KPrintf("SEMAPHORE_ILLEGAL\n");
		break;
	    default:
		KPrintf ("(unknown)\n");
	}
	
	/*if (sharedResource == 50000) {
		KPrintf ("I'm the last thread. ATTENTION!!! I'm destroying the IP-lock now!");
		switch (SemaphoreDestroy(sharedLock)) {
			case 0:
				KPrintf ("Lock not found!\n");
				break;
			case 1:
				KPrintf ("Lock successfully destroyed.\n");
				break;
			default:
				KPrintf ("Lock could not be destroyed. Still a few threads in critical section!\n");
				break;
		}
	}*/
	
	return argument;
}

void SpinThread1() {
	UINT8 c[] = {'/', '-', '\\', '|'};
	UINT8 index = 0;
	// 193
	for(;;) {
		VideoWriteAttribute(79, 49, 0x1400 + c[index]);
		index = (index +1) % sizeof(c);
		Pause(100);
	}
}

static UINT32 getRand(int range) {
	UINT32 var;
	
	READTSC(var);
	return var % range;
}

void MatrixThread() {
	
	UINT32 dark = 0x1200;
	UINT32 light = 0x1a00;
	
	UINT32 pos[80];
	UINT32 sym[80];
	int i, j;
	
	for (i = 0; i < 80; i++) {
		pos[i] = 0;
		sym[i] = 255 - getRand(30);
	}
	
	for(;;) {
		for (i = 0; i < 80; i++) {
			// draw vertical line
			for (j = 0; j < pos[i]; j++) {
				VideoWriteAttribute(i, j, dark + sym[i] + j - pos[i]);
			}
			VideoWriteAttribute(i, j, light + sym[i] - j);
			
			pos[i] = (pos[i] + getRand(2)) % 50;
		}
		
		Pause(50);
	}
}

void TickerThread(UINT32 line) {
	char *message = "+++ This is the ticker thread +++";
	UINT32 length = strlen(message);
	UINT32 i;
	SINT32 col, absPos;
	
	col = 79;
	
	for(;;) {
	
		// print the message
		for (i = 0; i < length; i++) {
			absPos = col + i;
			
			if ((absPos >= 0) && (absPos <= 79)) {
				VideoWriteAttribute(absPos, line, 0x7800 + message[i]);
			}
		}
		
		col--;
		
		if (col + length == 0) col = 79;
		Pause(tickerPause);
	}
}

void uptimeThread() {
	UINT8 upTime[4] = {0,0,0,0};
	int i;
	
	for(;;) {
		for (i = 3; i >= 0; i--) {
			if (upTime[i] == 10) {
				upTime[i-1]++;
				upTime[i] = 0;
			}
			VideoWriteAttribute(76+i, 2, 0x3800 + '0' + upTime[i]);
		}
		upTime[3]++;
		Pause(1000);
	}
}


void StartThread(void *target, PRIORITY prio, char* name) {
	ARGUMENTS args;
	
	args.count = 0;
	args.values = NULL;
	
	PID_HANDLE handle;
	
	ThreadCreate(target, prio, &args, name, &handle);
}

void StartThreadArgument(void *target, UINT32 argument, PRIORITY prio, char* name) {
	ARGUMENTS args;
	UINT32 valueList[1] = {argument};
	
	args.count = 1;
	args.values = valueList;
	
	PID_HANDLE handle;
	
	ThreadCreate(target, prio, &args, name, &handle);
}


void LockTest() {
	int i;
	PID_HANDLE handles[5];
	
	for (i = 0; i < 5; i++) {
		ARGUMENTS args;
		UINT32 valueList[1] = {i};
		args.count = 1;
		args.values = valueList;
		
		ThreadCreate(IPLockThread, RAY_PRIO_NORMAL, &args, "lockthread", &handles[i]);
	}
	
	SINT32 result = ThreadJoin(&handles[4]);
	KPrintf ("Thread 5 exited with code: %d\n", result);
}

void SchedulerWorker(UINT32 index) {
    int i;
    
    IPLOCK syncMutex = SemaphoreGet("schedulertest");
    double d, e;
    
    switch (syncMutex) {
    case SEMAPHORE_DENIED:
	KPrintf ("Access denied!\n");
	break;
    case SEMAPHORE_ILLEGAL:
	KPrintf ("Illegal semaphore!\n");
	break;
    default:

	while(TRUE) {
	    // too fast, slow down a little bit
	    for(i = 0; i < 10000; i++) {
		d = (double)i * 0.28732;
		e = d + i * 0.82637 * d;
	    }
	    
	    SemaphoreEnter(syncMutex);
		executionCounter[index]++;
	    SemaphoreLeave(syncMutex);
	}
    }

    ThreadExit(0);
}

void SynchronousCounter() {
    int i;
    UINT32 sum;
    IPLOCK syncMutex = SemaphoreGet("schedulertest");
    switch (syncMutex) {
	case SEMAPHORE_DENIED:
	    KPrintf ("Access denied!\n");
	    break;
	case SEMAPHORE_ILLEGAL:
	    KPrintf ("Illegal semaphore!\n");
	    break;
	default:

	    while(TRUE) {
		Pause(4000);
		
		SemaphoreEnter(syncMutex);
		
		sum = 0;
		for(i = 0; i < 3; i++) {
		    sum += executionCounter[i];
		}
		
		KPrintf("Thread\tExecution\tPercent\n===============================\n");
		for(i = 0; i < 3; i++) {
		    
		    KPrintf("%u\t%u\t\t%u%%\n", i, executionCounter[i], executionCounter[i] * 100 / sum);
		    executionCounter[i] = 0;
		}
		KPrintf("\n");
		
		SemaphoreLeave(syncMutex);
	    }


    }
    ThreadExit(0);
}

void SchedulerTest() {

    int i;
    // reset counter
    for(i = 0; i < 3; i++) {
	executionCounter[i] = 0;
    }
    
    IPLOCK syncMutex = MutexCreate("schedulertest", FALSE);
    if (syncMutex == SEMAPHORE_IN_USE) {
	KPrintf ("Semaphore is already in use! Coult not be created.\n");
    } else {
    
	StartThreadArgument(SchedulerWorker, 0, 2, "SchedulerWorker0");
	StartThreadArgument(SchedulerWorker, 1, 4, "SchedulerWorker1");
	StartThreadArgument(SchedulerWorker, 2, 8, "SchedulerWorker2");
	StartThread(SynchronousCounter, 15, "SynchronousCounter");
    }
}


void ProfileTest() {
    // ~5 MB
    ProfilingEnable(436906);
    ProfilingStart();
}

void ProfileStatus() {
    UINT32 entries = ProfilingGetSize();
    if (entries) {
	UINT32 used = ProfilingGetUsed();
	KPrintf ("Profiling enabled: %u of %u entries used.\n", used, entries);
    } else {
	KPrintf("Profiling disabled!\n");
    }
}


void ShowDMesg() {
	RMIMESSAGE data;
	char *string;

	switch (RMCall(DRV_DMESG, EXPORT_GETBUFFER, 0, &data, RTYPE_STRING, 4096)) {
	case RMI_SUCCESS:
		if (data) {
			if ((UINT32)data < 10) {
				KPrintf ("Error reading dmesg!\n");
				return;
			}
			string = (char*)data;
			KPrintf (string);
			KPrintf ("\n");
			FreeMessageBuffer(data);
		} else {
			KPrintf ("Error reading dmesg buffer!\n");
		}
		break;
	case RMI_NOT_SUPPORTED:
		KPrintf ("dmesg driver not loaded!\n");
		break;
	default:
		KPrintf ("An error occured while contacting the dmesg driver!\n");
	}
}

VFS_BUFFER ReadFileAt(VFS_HANDLE handle, UINT64 position, UINT64 length) {
	FILESTREAM stream;
	RMIMESSAGE result;
	
	stream = (FILESTREAM)AllocateMessageBuffer(sizeof(struct _FILESTREAM), RTYPE_STRING);
	stream->position = position;
	stream->length = length;
	stream->handle = handle;
	
	if (stream) {
		switch (RMCallMessage(DRV_VFS, VFS_EXPORT_READFILEAT, (RMIMESSAGE)stream, &result, RTYPE_STRING, sizeof(struct _VFS_BUFFER) + length)) {
		case RMI_SUCCESS:
			return (VFS_BUFFER)result;
			break;
		default:
			return NULL;
			break;
		}
	} else {
		KPrintf ("memory allocation failed!");
		return NULL;
	}
}

VFS_INFO FileInfo(VFS_HANDLE handle) {
	RMIMESSAGE result;
	switch (RMCall(DRV_VFS, VFS_EXPORT_FILEINFO, handle.handleUID, &result, RTYPE_STRING, 2048)) {
		case RMI_SUCCESS:
			return (VFS_INFO)result;
			break;
		default:
			return NULL;
			break;
	}
}

inline VFS_HANDLE VFSOpen(PTUString path, VFS_ACCESS mode) {
	RMIMESSAGE result;
	ACCOBJ access;
	VFS_HANDLE failed;
	
	access = (ACCOBJ)AllocateMessageBuffer(sizeof(struct _ACCOBJ) + sizeof(UChar) * path->length, RTYPE_STRING);
	
	RMINewStructure(rmiStruct, access, access + 1);
	CreateRMIStructFromStringP(rmiStruct, access, objPath, path);
	
	access->mode = mode;
	
	switch (RMCallMessage(DRV_VFS, VFS_EXPORT_ACCESSFILE, (RMIMESSAGE)access, &result, RTYPE_STRING, 2048)) {
	case RMI_SUCCESS:
		access = (ACCOBJ)result;
		if (access->errCode == VFS_SUCCESS) {
			failed = access->handle;
		} else {
			KPrintf ("Error %u [Additinal error info: %u]!\n", access->errCode, access->extendedErrorInfo);
			failed.handleUID = 0;
		}
		FreeMessageBuffer((RMIMESSAGE)result);
		break;
	default:
		failed.handleUID = 0;
		FreeMessageBuffer((RMIMESSAGE)access);
		return failed;
	}
	return failed;
}

VFS_HANDLE OpenFile(PTUString path) {
	return VFSOpen(path, VFS_ACC_READ);
}

VFS_HANDLE OpenDir(PTUString path) {
	return VFSOpen(path, VFS_ACC_LIST);
}

BOOL CloseFile(VFS_HANDLE handle) {
	RMIMESSAGE result;
	ACCOBJ access;

	access = (ACCOBJ)AllocateMessageBuffer(sizeof(struct _ACCOBJ), RTYPE_STRING);
	memset(access, 0, sizeof(struct _ACCOBJ));

	access->handle = handle;
	access->mode = VFS_ACC_CLOSE;

	switch (RMCallMessage(DRV_VFS, VFS_EXPORT_ACCESSFILE, (RMIMESSAGE)access, &result, RTYPE_STRING, 2048)) {
	case RMI_SUCCESS:
		access = (ACCOBJ)result;
		if (access->errCode == VFS_SUCCESS) {
			FreeMessageBuffer((RMIMESSAGE)result);
			return TRUE;
		} else {
			FreeMessageBuffer((RMIMESSAGE)result);
			return FALSE;
		}
		break;
	default:
		return FALSE;
	}
}

VFS_INFO FindNextFile(VFS_HANDLE handle) {
	RMIMESSAGE result;

	switch (RMCall(DRV_VFS, VFS_EXPORT_FINDNEXTFILE, handle.handleUID, &result, RTYPE_STRING, 2048)) {
	case RMI_SUCCESS:
		return (VFS_INFO)result;
		break;
	default:
		KPrintf ("Error contacting vfs");
		return NULL;
	}
}

void readtest(char *path) {
	PTUString unicodePath = ConvertASCIItoTUString(path);
	VFS_HANDLE fileHandle;
	VFS_BUFFER buff;
	
	fileHandle = OpenFile(unicodePath);
	free(unicodePath);
	
	if (fileHandle.handleUID) {
		buff = ReadFileAt(fileHandle, 0, 175);
		if (buff) {
			if (buff->errCode == VFS_SUCCESS) {
				// termination;
				unsigned char *content = (unsigned char*)((UINT32)buff->buffer + (UINT32)buff);
				content[buff->bufferLength-1] = 0;
				KPrintf("File content:\n%s\n", content);
			} else {
				KPrintf ("Error %u reading file.\n", buff->errCode);
			}
			FreeMessageBuffer((RMIMESSAGE)buff);
		} else {
			KPrintf ("Null-Pointer returned!\n");
		}
		CloseFile(fileHandle);
	} else {
		KPrintf ("Could not open file: %s!\n", (UINT32)path);
	}
	
}

char *getFileName(char *path) {
	char *lastSlash = path;
	
	while (*path) {
		if (*path++ == '/') {
			lastSlash = path;
		}
	}
	return lastSlash;
}

void DoWholeFile(char* path, DoFileFunc func)
{
	PTUString unicodePath = ConvertASCIItoTUString(path);
	VFS_HANDLE fileHandle;
	VFS_BUFFER buff;
	UINT32 fileSize;
	fileHandle = OpenFile(unicodePath);
	free(unicodePath);
	
	unsigned char *content = NULL;
	
	if (fileHandle.handleUID) {
		
		// get file size
		VFS_INFO info;
		info = FileInfo(fileHandle);
		if (info) {
			if (info->aOwner.is & VFS_ATTR_DIRECTORY) {
				KPrintf ("%s is a directory.\n", path);
				return;
			}
			fileSize = info->fileSize;
			FreeMessageBuffer((RMIMESSAGE)info);
			
			buff = ReadFileAt(fileHandle, 0, fileSize);
			if (buff) {
				if (buff->errCode == VFS_SUCCESS) {
					// termination;
					content = (unsigned char*)((UINT32)buff->buffer + (UINT32)buff);
					func(content, fileSize);
				} else {
					KPrintf ("Error %u reading file.\n", buff->errCode);
				}
				FreeMessageBuffer((RMIMESSAGE)buff);
			} else {
				KPrintf ("Null-Pointer returned!\n");
			}
			CloseFile(fileHandle);
		} else {
			KPrintf ("Null-Pointer returned!\n");
		}
	} else {
		KPrintf ("Could not open file: %s!\n", (UINT32)path);
	}
}

void StartExecutable(char *path, char *command) {
	PTUString unicodePath = ConvertASCIItoTUString(path);
	VFS_HANDLE fileHandle;
	VFS_BUFFER buff;
	UINT32 fileSize;
	fileHandle = OpenFile(unicodePath);
	free(unicodePath);
	
	if (fileHandle.handleUID) {
		
		// get file size
		VFS_INFO info;
		info = FileInfo(fileHandle);
		if (info) {
			if (info->aOwner.is & VFS_ATTR_DIRECTORY) {
				KPrintf ("%s is a directory.\n", path);
				return;
			}
			fileSize = info->fileSize;
			FreeMessageBuffer((RMIMESSAGE)info);
			
			buff = ReadFileAt(fileHandle, 0, fileSize);
			if (buff) {
				if (buff->errCode == VFS_SUCCESS) {
					// termination;
					unsigned char *content = (unsigned char*)((UINT32)buff->buffer + (UINT32)buff);
					if (!ELF32LoadFile(content, fileSize, getFileName(path), command)) {
						KPrintf ("Error loading %s.\n", path);
					}
				} else {
					KPrintf ("Error %u reading file.\n", buff->errCode);
				}
				FreeMessageBuffer((RMIMESSAGE)buff);
			} else {
				KPrintf ("Null-Pointer returned!\n");
			}
			CloseFile(fileHandle);
		} else {
			KPrintf ("Null-Pointer returned!\n");
		}
	} else {
		KPrintf ("Could not open file: %s!\n", (UINT32)path);
	}
	
}

void ParseStart(char *string) {
	char *path = string;
	char *command = strchr(string, ' ');
	if (command == NULL) {
		command = "";
	} else {
		*command = '\0';
		command++;
	}
	StartExecutable(path, command);
}


void fileSize(char *path) {
	PTUString unicodePath = ConvertASCIItoTUString(path);
	VFS_HANDLE fileHandle;
	VFS_INFO info;
	
	fileHandle = OpenFile(unicodePath);
	free(unicodePath);
	
	if (fileHandle.handleUID) {
		info = FileInfo(fileHandle);
		if (info) {
			if (info->aOwner.is & VFS_ATTR_DIRECTORY) {
				KPrintf ("%s is a directory.\n", path);
			} else {
				KPrintf ("File size: %u bytes, attributes: %u\n", info->fileSize, info->aOwner);
			}
			FreeMessageBuffer((RMIMESSAGE)info);
		} else {
			KPrintf ("Null-Pointer returned!\n");
		}
		CloseFile(fileHandle);
	} else {
		KPrintf ("Could not open file: %s!\n", path);
	}
	
}

void ls(char *path) {
	VFS_HANDLE rootDir;
	VFS_INFO object;
	char *temp;
	
	char acr[] = "-----";
	PTUString unicodePath;

	KPrintf ("Contents of %s:\n", (UINT32)path);

	unicodePath = ConvertASCIItoTUString(path);
	rootDir = OpenDir(unicodePath);
	free(unicodePath);

	if (rootDir.handleUID) {
		//KPrintf ("Directory successfully opened (handle %u)\n", rootDir.handleUID, 0);
		object = FindNextFile(rootDir);
		if (object) {
			while(object->errCode == VFS_SUCCESS) {
				TUString currFileName = CreateStringFromRMIStruct(object, name);
				temp = ConvertTUStringToASCII(&currFileName);

				// list directories in a different way
				StringDirAttrib(object->aOwner.is, VFS_ATTR_DIRECTORY, 0, 'd');
				StringDirAttrib(object->aOwner.is, VFS_ATTR_READABLE, 1, 'r');
				StringDirAttrib(object->aOwner.is, VFS_ATTR_WRITABLE, 2, 'w');
				StringDirAttrib(object->aOwner.is, VFS_ATTR_EXECUTABLE, 3, 'x');
				StringDirAttrib(object->aOwner.is, VFS_ATTR_HIDDEN, 4, 'h');
				KPrintf(" %s\t%s\t%u Bytes\n", acr, temp, object->fileSize);
				

				free(temp);
				FreeMessageBuffer((RMIMESSAGE)object);
				object = FindNextFile(rootDir);
			}
			//KPrintf ("Last FindNextFileError was: %u\n", object->errCode, 0);
			FreeMessageBuffer((RMIMESSAGE)object);

		} else {
			KPrintf ("object was NULL!\n");
		}
		if (!CloseFile(rootDir)) {
			KPrintf ("Closing file failed!\n");
		}
	} else {
		KPrintf ("Could not open directory '%s'\n", (UINT32)path);
	}

}

void ShowMountUsage() {
	KPrintf ("Usage: mount DIRECTORY DISK PARTITION\n");
	KPrintf ("\tDIRECTORY\tPath where 1st partition will be mounted to\n");
	KPrintf ("\tDISK\t\tNumber of hard drive\n");
	KPrintf ("\tPARTITION\tPartition number on that hard drive\n");
}

String GetFSName(UINT8 systemID) {
    switch (systemID) {
	case 0x00:
	    return "Partition empty";
	case 0x06:
	    return "FAT16";
	case 0x07:
	    return "NTFS";
	case 0x82:
	    return "Linux Swap";
	case 0x83:
	    return "Ext2";
	case 0x0B:
	case 0x0C:
	    return "FAT32";
	default:
	    return "unknown";
    }
}

void MountHDD(char *dir, UINT32 drive, UINT32 partition) {
	RMIMESSAGE result;
	INIT_MOUNT* toMount;
	PTUString regNode = ConvertASCIItoTUString("/");
	PTUString nodeName = ConvertASCIItoTUString(dir);
	PTUString info = ConvertASCIItoTUString("Testmount");

	/* ============= prepare strings ================== */
	toMount = (INIT_MOUNT*)AllocateMessageBuffer(sizeof(INIT_MOUNT) + (regNode->length + nodeName->length + info->length) * sizeof(UChar), RTYPE_STRING);

	RMINewStructure(rmiStruct, toMount, toMount + 1);
	CreateRMIStructFromStringP(rmiStruct, toMount, registerPath, regNode);
	CreateRMIStructFromStringP(rmiStruct, toMount, nodeName, nodeName);
	CreateRMIStructFromStringP(rmiStruct, toMount, addInfo, info);

	/* ================================================ */

	toMount->mountInfo.hardDrive = drive;
	toMount->mountInfo.partition = partition;
	toMount->mountInfo.forceFSType = FS_AUTODETECT;

	switch (RMCallMessage(DRV_ATA, CHAR_EXPORT_MOUNT, (RMIMESSAGE)toMount, &result, RTYPE_SAME, 0)) {
	case RMI_SUCCESS:
		toMount = (INIT_MOUNT*)result;
		switch (toMount->errCode) {
		case CHAR_ERR_SUCCESS:
			KPrintf ("Successfully mounted (%s)!\n",(UINT32)toMount->identifier);
			break;
		default:
			KPrintf ("Error mounting device: ");
			switch(toMount->errCode) {
				case 2: KPrintf ("Out of memory\n");break;
				case 3: KPrintf ("Setup failed\n");break;
				case 4: KPrintf ("Read error\n");break;
				case 5: KPrintf ("Request error\n");break;
				case 100: KPrintf ("Already registered\n");break;
				case 101: KPrintf ("Unsupported VFS function\n");break;
				case 102: KPrintf ("File system not supported: %s.\n", GetFSName((UINT8)toMount->extErrInfo));break;
				case 120: KPrintf ("Invalid partition\n");break;
				case 121: KPrintf ("Invalid disk drive\n");break;
				case 122: KPrintf ("Mounting error\n");break;
				case 123: KPrintf ("Unexpected end\n");break;
				case 124: KPrintf ("No drives found\n");break;
				default: KPrintf ("(unknown)\n");
			}
		}
		break;
	case RMI_NOT_SUPPORTED:
		KPrintf ("ata driver not loaded!\n");
		break;
	default:
		KPrintf ("An error occured while contacting the ata driver!\n");
	}

	FreeMessageBuffer((RMIMESSAGE)toMount);
}

UINT32 StringToInt(char *string) {
	UINT32 result = 0;
	
	while(*string) {
		if ((*string >= '0') && (*string <= '9')) {
			result *= 10;
			result += *string - '0';
			string++;
		} else {
			return 0;
		}
	}
	
	return result;
}

void ParseMount(char *string) {
	char *dir = string;
	char *drive, *part;
	
	drive = strchr(string, ' ');
	if (drive == NULL) {
		ShowMountUsage();
		return;
	}
	
	*drive = '\0';	// terminate "dir"
	drive++;	// skip space
	
	part = strchr(drive, ' ');
	if (part == NULL) {
		ShowMountUsage();
		return;
	}
	*part = '\0';
	part++;
	
	UINT32 drvNum, partNum;
	
	drvNum = StringToInt(drive);
	partNum = StringToInt(part);
	
	MountHDD(dir, drvNum, partNum);
	
}

void PercentToString(float flt, char *string) {	
	UINT32 number = (UINT32)(flt * 1000.f);
	string[4] = 0;
	string[3] = (number % 10)+ '0'; number /= 10;
	string[2] = '.';
	string[1] = (number % 10)+ '0'; number /= 10;
	string[0] = (number % 10)+ '0'; number /= 10;
}


void my_itoa(UINT32 number, char *string) {	
	string[2] = 0;
	string[1] = (number % 10) + '0';
	number /= 10;
	string[0] = (number % 10) + '0';
	
}

void ProcessStatistics() {
	THREAD_INFO_LIST *current;
	THREAD_INFO_LIST *last = NULL;
	
	UINT32 i, j;
	char *states[5] = {"RUN", "IDLE", "WAIT", "MSG", "EVENT"};
	
	do {
	    current = GetThreadInfo(THREAD_INFO_ALL);
	    if (current) {
		    if (last) {
			KPrintf("PID\tTID\t%%CPU\t%%KERNEL\tSTATE\tNAME\n");
			KPrintf("========================================\n");
			UINT64 totalCycles = 0;
			for (i = 0; i < current->numThreads; i++) {
			    for(j = 0; j < last->numThreads; j++) {
				if ( (last->thread[j].pid == current->thread[i].pid) 
				&& (last->thread[j].threadNum == current->thread[i].threadNum)) {
				    totalCycles += current->thread[i].cpuCycles - last->thread[j].cpuCycles;
				}
			    }
			}
			
			UINT64 threadWork = 0;
			double totalWork;
			int perMilleWork;
			UINT64 threadWorkK = 0;
			double totalWorkK;
			int perMilleWorkK;
			for (i = 0; i < current->numThreads; i++) {
			
				// find in old structure
				for(j = 0; j < last->numThreads; j++) {
				    if ( (last->thread[j].pid == current->thread[i].pid) 
					  && (last->thread[j].threadNum == current->thread[i].threadNum)) {
			
					KPrintf("%u\t\%u", current->thread[i].pid, current->thread[i].threadNum);
					threadWork = current->thread[i].cpuCycles - last->thread[j].cpuCycles;
					totalWork = (double)threadWork / (double)totalCycles;
					perMilleWork = (int)(totalWork * 10000);

					// Kernel
					threadWorkK = current->thread[i].privCycles - last->thread[j].privCycles;
					totalWorkK = threadWork > 0 ? (double)threadWorkK / (double)threadWork : 0;
					perMilleWorkK = (int)(totalWorkK * 10000);
					KPrintf("\t%u.%u%u\t%u.%u%u\t%s\t%s\n", 
							perMilleWork / 100, 
							(perMilleWork / 10 ) % 10,
							perMilleWork % 10,
							perMilleWorkK / 100, 
							(perMilleWorkK / 10) % 10,
							perMilleWorkK % 10, 
       							states[current->thread[i].state],
       							current->thread[i].ident);
					
					break;
				    }
				}
			}
			free(last);
		    }
		    last = current;
		    Pause(4000);
	    } else {
		    break;
	    }
	} while(1);
	ThreadExit(0);
}

void PrintThreadInfo(THREAD_INFO *info, BOOL extended) {
    char *states[5] = {"RUN", "IDLE", "WAIT", "MSG", "EVENT"};
    if (extended) {
	KPrintf ("%u\t%u\t", info->pid, info->threadNum);
	KPrintf ("%uK\t%u\t%u:%u\t", info->usedPages << 2, info->memAllocs, info->lastFitPointerCode, info->lastFitPointerData);
	KPrintf ("%s\t%s\n", states[info->state], info->ident);
    } else {
	KPrintf ("%u\t%u\t\t\t\t%s\t%s\n", info->pid, info->threadNum, states[info->state], info->ident);
    }
}

void ProcessInfo(PID pid) {
	THREAD_INFO_LIST *info;
	UINT32 i, j;

	if (pid) {
		info = GetThreadInfo(pid);
	} else {
		info = GetThreadInfo(THREAD_INFO_ALL);
	}

	if (info) {
		KPrintf ("%u threads currently running.\nKernel Memory %u bytes.\n\n", info->numThreads, info->kernelMemory);
		KPrintf ("PID\tTID\tMEM\tALLOCS\t\tSTATUS\tNAME\n");
		KPrintf ("======= ======= ======= =============== ======= ===============\n");
		for (i = 0; i < info->numThreads; i++) {
			// if it's a parent thread (process)
			if (info->thread[i].threadNum == 0) {
			    PrintThreadInfo(info->thread + i, TRUE);
			    
			    // print all the threads
			    for (j = 0; j < info->numThreads; j++) {
				if ((info->thread[i].pid == info->thread[j].pid) && (info->thread[j].threadNum > 0)) {
				    PrintThreadInfo(info->thread + j, FALSE);
				}
			    }
			    KPrintf("\n");
			}
		}
		KPrintf ("Memory usage: %u bytes (%u Bytes free)\n", info->memUsageTotal, info->memFree);
		UINT32 memTotal = info->memUsageTotal + info->memFree;
		double perCUSEDd = (double)info->memUsageTotal / (double)memTotal;
		UINT32 perCUsed = (UINT32)(perCUSEDd * 100.0);
		KPrintf ("              %u MB total (%u%% used)\n", memTotal >> 20, perCUsed);

		free(info);
	} else {
		KPrintf ("Error getting process information!\n");
	}
}

UINT64 ReadTSC() {
	UINT32 eax, edx;
	UINT64 result;
	
	__asm__ __volatile__("rdtsc":"=a"(eax), "=d"(edx));
	result = edx;
	result <<= 32;
	return result + eax;
}

void ABICallBenchmark() {
	
	UINT32 i;
	
	UINT64 sum = 0L;
	
	for(i = 0; i < 1000; i++) {
		UINT64 start = ReadTSC();
		GetPID();
		UINT64 stop = ReadTSC();	
		
		sum += (stop - start);
	}
	
	sum /= 1000;
	
	KPrintf ("ABI Call (1000x) average: %u\n\n", (UINT32)(sum & 0xFFFFFFFF));
	
	
}

void IPCBenchmark() {
	RMIMESSAGE benchResult = 0;
	
	switch (RMCall(0xDEADBEEF, 0, 0, &benchResult, RTYPE_STRING, 128)) {
		case RMI_SUCCESS:
			if (benchResult) {
				KPrintf ("IPC Benchmark : %s\n\n", (UINT32)benchResult);
				FreeMessageBuffer(benchResult);
			} else {
				KPrintf ("Interrupted!\n");
			}
			break;
		case RMI_NOT_SUPPORTED:
			KPrintf ("Bemchmark driver seems not to be loaded!\n");
			break;
		default:
			KPrintf ("An error occured while contacting the Benchmark driver.\n");
	}
}

void YieldBenchmarkThread() {
	UINT32 i;
	
	for(i = 0; i < 500; i++) {
		SemaphoreEnter(sharedLock);
		if (i) {
			UINT64 stop = ReadTSC();
			yieldBenchmarkSum += stop - yieldBenchmarkTime;
		}
		
		yieldBenchmarkTime = ReadTSC();
		SemaphoreLeave(sharedLock);
	}
}

void ThreadYieldBenchmark() {
	yieldBenchmarkSum = 0L;
	
	ARGUMENTS args;
	PID_HANDLE thread1, thread2;
	
	args.count = 0;
	args.values = NULL;
	
	ThreadCreate(YieldBenchmarkThread, RAY_PRIO_NORMAL, &args, "yieldthread", &thread1);
	ThreadCreate(YieldBenchmarkThread, RAY_PRIO_NORMAL, &args, "yieldthread", &thread2);
	
	
	SINT32 result = ThreadJoin(&thread1);
	result = ThreadJoin(&thread2);
	
	yieldBenchmarkSum /= 1000;
	KPrintf("Thread yield (1000x) average: %u\n\n", (UINT32)yieldBenchmarkSum);
}

CALLBACK ReadTSCResult(RMISERIAL sender, UINT32 tscBits, UINT32 msgID) {
	processTSCResult = tscBits;
	
	BarrierGo("tscbarrier", FALSE);
	ThreadExit(0);
}

UINT32 MeasureProcessCreateDiff(unsigned char* content, UINT32 fileSize) {
	
	// measure the start time and actually create the process
	UINT32 startTime = ELF32LoadFile2(content, fileSize, "readtsc", "");
	if (startTime == 0) {
		KPrintf ("Error loading benchmark program!");
		return 0;
	}
	
	// wait for the process to signal the value
	BarrierArrive("tscbarrier");
	BarrierClose("tscbarrier");
	
	return processTSCResult - startTime;
}

void BenchmarkProcessCreation(unsigned char* content, UINT32 fileSize) {
	UINT32 i;
	
	for(i = 0; i < 200; i++) {
		UINT32 timeDiff = MeasureProcessCreateDiff(content, fileSize);
		if (timeDiff == 0) {
			return;
		}
		
		processCreationSum += timeDiff;
		KPrintf("[%u] ", timeDiff);
		Pause(50);
	}
	
	processCreationSum /= 200;
	
}

void CreateProcessBenchmark() {
	
	processCreationSum = 0L;
	processCreationSum /= 200;
	
	
	
	DoWholeFile("/disk/boot/modules/readtsc", &BenchmarkProcessCreation);
	
	KPrintf("Process Create (200x) average: %u\n", (UINT32)processCreationSum);
}

void StartBenchMark() {
	KPrintf ("Please don't press any keys in order not to interfer with the exact cycle count.\nStarting benchmark...\n");
	
	// IPC benchmark
	IPCBenchmark();
	
	// ABI call benchmark
	ABICallBenchmark();
	
	ThreadYieldBenchmark();
	
	CreateProcessBenchmark();
	
}

void ShowUsage() {
	KPrintf ("NURNware tinyShell v0.2\n");
	KPrintf ("Available commands are:\n");
	KPrintf ("\tver\t\tShow version\n");
	KPrintf ("\thelp\t\tShow this help\n");
	KPrintf ("\tcpuinfo\t\tCPU information\n");
	KPrintf ("\tpci\t\tList PCI devices\n");
	KPrintf ("\tlock\t\tCreate 4 threads that heavily access a shared \n\t\t\tresource using semaphores\n");
	KPrintf ("\tmypid\t\tPrint own PID\n");
	KPrintf ("\tscheduler\tStress test the scheduler and print statistics\n");
	KPrintf ("\tprofile\t\tProfile kernel activities into kernel memory\n");
	KPrintf ("\tprofile stop\tStop recording profiling data\n");
	KPrintf ("\tprofile status\tPrint current profiling status\n");
	KPrintf ("\tprofile reset\tErase profiling data\n");
	KPrintf ("\tprofile flush\tFlush profiling data stored in kernel memory to COM2\n");
	KPrintf ("\tps\t\tProcess and thread info\n");
	KPrintf ("\tmount\t\tTry to mount 1st file system on 1st partition\n");
	KPrintf ("\tcat\t\tRead file to console\n");
	KPrintf ("\tsize\t\tSize of a file in bytes\n");
	KPrintf ("\tstart\t\tStart an ELF executable\n");
	KPrintf ("\tdmesg\t\tShow diagnostic messages\n");
	KPrintf ("\tqueue\t\tTest the message queue\n");
	KPrintf ("\tbenchmark\tBenchmark how many CPU cycles 2000 IPC messages take\n");
	KPrintf ("\tticker\t\tStart ticker thread\n");
	KPrintf ("\tstack\t\tStack execution test (security feature)\n");
	KPrintf ("\tarray\t\tArray boundary test (security feature)\n");
	KPrintf ("\tmprot\t\tKernel memory allocation protection (security feature)\n");
	KPrintf ("\texit\t\tExit this process(shell)\n");

	KPrintf ("\treboot\t\tReboot computer\n");
}

void stackTest() {
	// x86 instructions : nop, nop, nop, ret
	unsigned char stack_instructions[] = {0x90, 0x90, 0x90, 0xc3};
	
	KPrintf ("Executing the stack...\n");
	
	// declare function
	void (*dataFunction)();
	
	// set it to the data containing processor instructions
	dataFunction = (void*)stack_instructions;
	
	// call them
	dataFunction();
	
	KPrintf ("Did not work! Testing something else...");
	__asm__ __volatile__ ("hlt");
		
	// if we get here... oh oh
	KPrintf ("Still alive. This shouldn't happen\n");
}

void memProtTest() {
	KPrintf ("Memory allocation protection test:\n");
	UINT32 *array = (UINT32*)malloc(100 * sizeof(UINT32));
	UINT32 *behind = (UINT32*)malloc(4096);
	
	KPrintf ("Writing behind allocated position in next array...\n");
	array[100] = 666;
	KPrintf ("Memory protection seems to be switched off! ");
	KPrintf ("Value of subsequent array: %u\n", behind[0]);
	
	free(behind);
	free(array);
}

void arrayTest() {
	KPrintf ("Allocating array of size 42...\n");
	unsigned int *array = (unsigned int*)malloc(42 * sizeof(unsigned int));
	
	KPrintf ("array is at 0x%x to 0x%x. ", array, array + 42 * sizeof(unsigned int));
	KPrintf ("Writing at element 43...\n");
	array[42] = 666;
	
	KPrintf ("Still there. I've just overwritten some memory... Good luck!\n");
}

void ExitProgram() {
	
	switch(RMInvoke(DRV_KEYBOARD, RMIProcessDetach, 0)) {
	case RMI_SUCCESS:
		break;
	case RMI_NOT_SUPPORTED:
		KPrintf ("Keyboard observer not found! Exiting.\n");
	default:
		KPrintf ("Error while contacting keyboard driver.Exiting\n");
		exit(4);
	}
	exit(0);
}

void displayVersion() {
    KPrintf ("NURNware tinyShell version 0.2\nCompiled on %s, %s\n", __DATE__, __TIME__);
    KPrintf ("Copyright (C) 2009 by NURNware Technologies [stefan@nurnware.de]\n");
}

void blueScreen() {
    KPrintf ("\nA problem has been detected and Windows has been shut down to prevent damage\n");
    KPrintf ("to your computer\n\n");
    KPrintf ("IRQL_NOT_LESS_OR_EQUAL\n\n");
    KPrintf ("If this is the first time you've sen this Stop error screen,\n");
    KPrintf ("restart your computer. If this screen appears again, follow\n");
    KPrintf ("these steps:\n\n");
    KPrintf ("Check to make shure that any new hardware or software is properly installed.\n");
    KPrintf ("If this is a new installation, ask your hardware or software manufacturer\n");
    KPrintf ("for any windows updates you might need.\n\n");
    KPrintf ("If problems continue, disable or remove any newly installed hardware\n");
    KPrintf ("or software. Disable BIOS memory options such as caching or shadowing.\n");
    KPrintf ("If you need to use Safe Mode to remove or disable components, restart\n");
    KPrintf ("your computer, press F8 to select Advanced Startup Options, and then\n");
    KPrintf ("select Safe Mode.\n\n");
    KPrintf ("Technical information:\n\n");
    KPrintf ("*** STOP: 0x000000A (0x00000000, 0xF782AC88, 0x000000008, 0xC000000)\n\n\n");
    KPrintf ("*** WIN32K.SYS - Adress F78A82A2 base at F7821000, DateStamp 36B02A71\n");
    KPrintf ("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");


}

void queuePump(UINT32 id) {
    RMIMESSAGE testMsg = (RMIMESSAGE)AllocateMessageBuffer(1024, RTYPE_STRING);
    
    String message = (String)testMsg;
    strcpy(message, " - Worked");
    message[0] = '0' + id;
    
    int i;
    for(i = 0; i < 5; i++) {
    
	KPrintf ("[%u(%u) ...", id, i);
	RAY_RMI result = RMCallMessage(0x9876A7C9, 0, testMsg, &testMsg, RTYPE_STRING, 1024);
	if (result != RMI_SUCCESS) {
	    KPrintf ("Error sending to 'deferredq'!\n");
	    FreeMessageBuffer(testMsg);
	    //return;
	    ThreadExit(-1);
	}
	
	KPrintf ("... %u(%u) = '%s']\n", id, i, (String)testMsg);
    }
    FreeMessageBuffer(testMsg);
    ThreadExit(0);
}

void queueTest() {
    UINT32 threadCount = 5;
    KPrintf ("Starting %u threads that continuously send messages...\n", threadCount);
    int i;
    for(i = 0; i < threadCount; i++) {
	StartThreadArgument(queuePump, i + 1, RAY_PRIO_NORMAL, "queueTest");
    }
}

void StartTicker() {
	
	ARGUMENTS args;
	UINT32 valueList[1] = {tickerLine++};
	
	args.count = 1;
	args.values = valueList;
	
	
	ThreadCreate(TickerThread, RAY_PRIO_HIGHER, &args, "ticker", &tickerHandles[numTickers]);
	numTickers++;
}

void StopTickers() {
	int i;
	if (numTickers == 0) {
		KPrintf ("No ticker running.\n");
	} else {
		KPrintf ("Stopping all the %u tickers...", numTickers);
		
		for (i = 0; i < numTickers; i++) {
			if (!ThreadAbort(&tickerHandles[i])) {
				KPrintf ("[Stopping #%u FAILED!]", i);
			}
		}
		KPrintf ("\n");
		
		// reset number
		numTickers = 0;
		tickerLine = 5;
	}
}

void shellCommand(char *command) {

	if (strcmp(command, "exit") == 0) {
		ExitProgram();
	} else if (strcmp(command, "cpuinfo") == 0) {
		printCPUInfo();
	} else if (strcmp(command, "mypid") == 0) {
		KPrintf ("My pid is %u\n", GetPID());
	} else if (strcmp(command, "lock") == 0) {
		LockTest();
	} else if (strcmp(command, "scheduler") == 0) {
		SchedulerTest();
	} else if (strcmp(command, "profile") == 0) {
		ProfileTest();
	} else if (strcmp(command, "profile flush") == 0) {
		ProfilingFlush();
	} else if (strcmp(command, "profile status") == 0) {
		ProfileStatus();
	} else if (strcmp(command, "start") == 0) {
		KPrintf ("Usage: start FILENAME [ARGUMENTS]\n");
		KPrintf ("\tFILENAME\t\tAbsolute file name of the executable to start\n");
		KPrintf ("\tARGUMENTS\tString passed to the executable\n");
	} else if (strncmp(command, "start ", 6) == 0) {
		ParseStart(command + 6);
	} else if (strcmp(command, "pci") == 0) {
		ShowPCIdevices();
	} else if (strcmp(command, "dmesg") == 0) {
		ShowDMesg();
	} else if (strcmp(command, "mount") == 0) {
		ShowMountUsage();
	} else if (strncmp(command, "mount ", 6) == 0) {
		ParseMount(command + 6);
	} else if (strcmp(command, "ps") == 0) {
		ProcessInfo(0);
	} else if (strcmp(command, "myinfo") == 0) {
		ProcessInfo(GetPID());
	} else if (strcmp(command, "pstats") == 0) {
		StartThread(ProcessStatistics, RAY_PRIO_NORMAL, "pstats");
	} else if (strcmp(command, "benchmark") == 0) {
		StartBenchMark();
	} else if (strcmp(command, "reboot") == 0) {
		RMInvoke(DRV_KEYBOARD, DO_REBOOT, 0);
	} else if (strcmp(command, "help") == 0) {
		ShowUsage();
	} else if (strncmp(command, "ls ", 3) == 0) {
		ls(command + 3);
	} else if (strcmp(command, "cat") == 0) {
		KPrintf ("Usage: cat FILENAME\n");
	} else if (strncmp(command, "cat ", 4) == 0) {
		readtest(command + 4);
	} else if (strcmp(command, "size") == 0) {
		KPrintf ("Usage: size FILENAME\n");
	} else if (strncmp(command, "size ", 5) == 0) {
		fileSize(command + 5);
	} else if (strcmp(command, "ticker") == 0) {
		StartTicker();
	} else if (strcmp(command, "stopticker") == 0) {
		StopTickers();
	} else if (strcmp(command, "stack") == 0) {
		stackTest();
	} else if (strcmp(command, "array") == 0) {
		arrayTest();
	} else if (strcmp(command, "mprot") == 0) {
		memProtTest();
	} else if (strcmp(command, "queue") == 0) {
		queueTest();
	} else if (strcmp(command, "shit") == 0) {
		blueScreen();
	} else if (strcmp(command, "ver") == 0) {
		displayVersion();
	} else if (strcmp(command, "matrix") == 0) {
		StartThread(MatrixThread, RAY_PRIO_NORMAL, "matrix");
	}else if (strcmp(command, "break") == 0) {
		_BREAKPOINT();
	} else if (strcmp(command, "+") == 0) {
		tickerPause += 10;
		KPrintf ("Ticker pause is now set to %u\n", tickerPause);
	} else if (strcmp(command, "-") == 0) {
		tickerPause -= 10;
		KPrintf ("Ticker pause is now set to %u\n", tickerPause);
	} else {
		KPrintf ("Unknown command. Try 'help' to get help.\n");
	}
}

CALLBACK getCh(RMISERIAL sender, UINT32 keyCode, UINT32 msgID) {
	char currChar = (char)keyCode;
	static char running = 0;
	
	if (running) ThreadExit(-1);
	
	UINT16 extendedCode = keyCode & 0xFFFF;
	if (extendedCode > 256)
	{
		KPrintf (" Extended unknown key pressed (%x) ", extendedCode);
		ThreadExit(-2);
	}

	switch (currChar) {
		case SP_KEY_BACKSPACE:
			if (bufPos) {
				bufPos--;
				inputBuf[bufPos] = 0;
			}
			KPrintf("\r>%s ", inputBuf);
			break;
		case SP_KEY_RETURN:
			KPrintf ("\n");
			if (bufPos) {
				running = 1;
				shellCommand(inputBuf);
				running = 0;
				bufPos = 0;
				inputBuf[0] = 0;
			}
			break;
		default:
			if (currChar > 31) {
				inputBuf[bufPos] = currChar;
				bufPos++;
				inputBuf[bufPos] = 0;
			}
	}

	KPrintf ("\r>%s", (UINT32)inputBuf);
	ThreadExit(0);
}

RAYENTRY KernelModuleEntry(char *arguments) {

	RMISetup(0x12345678, 2);

	// Wait for keyboard...
	BarrierArrive(KEYBOARD_BARRIER);
	switch(RMInvoke(DRV_KEYBOARD, RMIProcessAttach, EXPORT_GET_CH)) {
		case RMI_SUCCESS:
			break;
		case RMI_NOT_SUPPORTED:
			KPrintf ("Keyboard observer not found! Exiting.\n");
		default:
			KPrintf ("Error while contacting keyboard driver.Exiting\n");
			exit(4);
	}

	inputBuf = (char*)malloc(256);

	*inputBuf = 0;
	RMIRegisterValue(EXPORT_GET_CH, getCh, 255, FALSE);
	RMIRegisterValue(1, ReadTSCResult, 255, FALSE);
	KPrintf ("\n%s\n>", arguments);
	
	//StartThread(uptimeThread, RAY_PRIO_NORMAL, "uptime");
	BarrierCreate("tscbarrier", FALSE);
	
	sharedLock = MutexCreate("testlock", TRUE);
	if (sharedLock == SEMAPHORE_IN_USE) {
		KPrintf("Error creating inter-process lock!");
	}
	
	Sleep();
	exit(5);
}
