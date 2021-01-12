#include <raykernel.h>
#include <drivers/dmesg/dmesg.h>
#include <tdm/io.h>
#include <tdm/int.h>
#include <threads/sleep.h>
#include <rmi/rmi.h>
#include <pid/pids.h>
#include <memory/memory.h>
#include <string.h>

#include "ataio.h"
#include "../fs/charerr.h"
#include "drives.h"

#include "functions.h"
#include "lowlevel.h"

#include "../fs/charexports.h"
#include "../fs/charimports.h"
#include "../fs/chardev.h"


volatile HDD *storage;
volatile FS_DRIVER *fileSystems;

extern IPLOCK waitForDataP;
extern IPLOCK waitForDataS;


#define ALLOC_CONTROLLER(controller) \
controller.partitions = malloc(NUM_PARTITIONS * sizeof(PARTITION_INFO)); \
if (!controller.partitions) return FALSE; \
memset(controller.partitions, 0, NUM_PARTITIONS * sizeof(PARTITION_INFO)); \


/* local prototypes */
/**
 * Request all privileges needed to SPECCOMMAND I/O-ports and wait for interrupts 
 */
static UINT32 PrepareKernel();

/**
 * Fill internal data structures
 */
static void DriverSetup();


static void DriverSetup() {
	/* primary master */
	storage[0].basePort = ATA_PRIMARY;
	storage[0].irq = 14;
	storage[0].ctrlPort = ATA_CONTROL_REG_PRIM;
	storage[0].isSlave = CTRL_MASTER;
	storage[0].drive = 0;
	
	/* primary slave */
	storage[1].basePort = ATA_PRIMARY;
	storage[1].irq = 14;
	storage[1].ctrlPort = ATA_CONTROL_REG_PRIM;
	storage[1].isSlave = CTRL_SLAVE;
	storage[1].drive = 1;
	
	/* secondary master */
	storage[2].basePort = ATA_SECONDARY;
	storage[2].irq = 15;
	storage[2].ctrlPort = ATA_CONTROL_REG_SECD;
	storage[2].isSlave = CTRL_MASTER;
	storage[2].drive = 0;
	
	/* secondary slave */
	storage[3].basePort = ATA_SECONDARY;
	storage[3].irq = 15;
	storage[3].ctrlPort = ATA_CONTROL_REG_SECD;
	storage[3].isSlave = CTRL_SLAVE;
	storage[3].drive = 1;
	
}

static UINT32 PrepareKernel() {
	UINT32 result = 0;
	int range;
	
	for (range = 0; range < 8; range++) {
		result += RequestIOPort(storage[0].basePort + range);
		result += RequestIOPort(storage[2].basePort + range);
	}
	
	result += RequestIOPort(ATA_CONTROL_REG_PRIM);
	result += RequestIOPort(ATA_CONTROL_REG_SECD);
	
	return result;
}

static BOOL SetupStructures() {
	
	fileSystems = (FS_DRIVER*)malloc(256 * sizeof(FS_DRIVER));
	if (!fileSystems) return FALSE;
	memset(fileSystems, 0, 256 * sizeof(FS_DRIVER));
	
	storage = (HDD*)malloc(NUM_HDD * sizeof(HDD));
	if (!storage) return FALSE;
	memset(storage, 0, NUM_HDD * sizeof(HDD));
	
	ALLOC_CONTROLLER(storage[0]);
	ALLOC_CONTROLLER(storage[1]);
	ALLOC_CONTROLLER(storage[2]);
	ALLOC_CONTROLLER(storage[3]);
		
	return TRUE;
}

static void SavePartitionLayoutThread() {
	ARGUMENTS args;
	
	args.count = 0;
	args.values = NULL;
	
	PID_HANDLE handle;
	
	ThreadCreate(SavePartitionLayout, RAY_PRIO_NORMAL, &args, "SavePartitionLayout", &handle);
	// Cannot wait for thread to join, as it is not allowed by the RAY thread model
	// to change the state of the thread while it is locked (for example while waiting for
	// a join). As the SavePartitionLayout function causes an IRQ, the irq-handler would
	// try to change the state of this (waiting) thread, which is not allowed.
	//ThreadJoin(&handle);
}

RAYENTRY KernelModuleEntry(void) {
	
	CreateCharBarrier(DRV_ATA);

	RMISetup(DRV_ATA, 4);
	
	RMIRegister(CHAR_EXPORT_READSEC, CHARReadSector, 255, FALSE, RTYPE_STRING, sizeof(CHAR_DISK));
	RMIRegister(CHAR_EXPORT_REGISTER_FS, RegisterFileSystem, 255, FALSE, RTYPE_STRING, sizeof(FS_REGISTER));
	RMIRegisterValue(CHAR_EXPORT_CLAIMED_DRIVERS, GetClaimedDrivers, 255, FALSE);
	RMIRegister(CHAR_EXPORT_MOUNT, MountPartition, 255, FALSE, RTYPE_STRING, 2048);
	
	// register semaphores
	waitForDataP = SemaphoreCreate("ataIRQ14", 0, FALSE);
	waitForDataS = SemaphoreCreate("ataIRQ15", 0, FALSE);
	
	
	if (!SetupStructures()) {
		dmesg ("ata: Out of Memory!");
		exit(CHAR_ERR_OUT_OF_MEMORY);
	}
	
	DriverSetup();
	
	if (PrepareKernel()) {
		dmesg ("ata: I/O setup failed.");
		exit(CHAR_ERR_SETUP_FAILED);
	}
	
	RegisterIRQ(14, ataIRQ);
	RegisterIRQ(15, ataIRQ);
	
	if(!DetectDrives()) {
		dmesg ("ata: No ata-drives found!");
		exit(CHAR_ERR_NO_DRIVES_FOUND);
	}
	
	SavePartitionLayoutThread();

	for(;;) {
		Sleep();
	}
}
