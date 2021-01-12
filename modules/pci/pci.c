#include <raykernel.h>
#include <tdm/io.h>
#include <rmi/rmi.h>
#include <memory/memory.h>
#include <threads/sleep.h>
#include <drivers/dmesg/dmesg.h>
#include <string.h>
#include <pid/pids.h>

#include "pcitypes.h"
#include "pci.h"
#include "pciio.h"

#define UNUSED __attribute__((unused))

PCIDeviceArray *devices;

BOOL PCIInit() {
	
	/* detect PCI */
	if (PCIDirectDetect()) {
		return FALSE;
	}
		
	devices = PCIScanBuses(&devices);
	
	/* check for errors */
	return (devices != NULL);
}

CALLBACK PCIgetDeviceList (RMISERIAL sender, UINT32 argument, UINT32 msgID) {
	RMIMESSAGE pciList;
	PCIDeviceArray *newList;
	
	pciList = (RMIMESSAGE)AllocateMessageBuffer(sizeof(PCIDeviceArray) + sizeof(PCIDevice) * devices->numPCIDevices, RTYPE_STRING);
	if (!pciList) {
		CReturn(0, msgID, FALSE);
		return;
	}
	
	/* copy array structure */
	newList = (PCIDeviceArray*)pciList;
	memcpy(newList, devices, sizeof(PCIDeviceArray));
	
	RMINewStructure(listPos, newList, newList + 1);
	
	RMIAddToStructure(listPos, newList, PCIDevice, list, sizeof(PCIDevice) * devices->numPCIDevices);
	
	/* copy list itself */
	memcpy(RMIGetOffset(newList, PCIDevice, list), devices->list, sizeof(PCIDevice) * devices->numPCIDevices);
		
	CReturn(pciList, msgID, TRUE);
}


RAYENTRY KernelModuleEntry(char *arguments) {
	int portRange;
	
	for (portRange = 0; portRange < 0x04; portRange++) {
		RequestIOPort(CONFIG1_ADR + portRange);
		RequestIOPort(CONFIG1_DATA + portRange);
	}
		
	if (PCIInit()) {
	
		RMISetup(DRV_PCI, 1);
		RMIRegisterValue(0, PCIgetDeviceList, 255, FALSE);
		
		for (;;) {
			Sleep();
		}
	} else {
		dmesg("No pci-bus found");
		exit(ERR_PCI_NO_BUS_FOUND);
	}
}
