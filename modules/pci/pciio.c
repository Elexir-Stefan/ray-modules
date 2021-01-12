#include <raykernel.h>
#include <tdm/io.h>
#include <memory/memory.h>
#include <string.h>

#include "pcitypes.h"
#include "pciio.h"

#include "hwmakros.h"

void PCIReadConfigB8(UINT16 bus, UINT8 device, UINT8 function, UINT8 reg, UINT8 *value) {
	OutPortL(CONFIG1_ADR, PCI_DIRECT_ACC | (bus << 16) | (device << 11) | (function << 8) | (reg & ~3));
	*value = InPortB(CONFIG1_DATA + (reg & 3));
}

void PCIWriteConfigB8(UINT16 bus, UINT8 device, UINT8 function, UINT8 reg, UINT8 value) {
	OutPortL(CONFIG1_ADR, PCI_DIRECT_ACC | (bus << 16) | (device << 11) | (function << 8) | (reg & ~3));
	OutPortB(CONFIG1_DATA + (reg & 3), value);
}

void PCIReadConfigB16(UINT16 bus, UINT8 device, UINT8 function, UINT8 reg, UINT16 *value) {
	OutPortL(CONFIG1_ADR, PCI_DIRECT_ACC | (bus << 16) | (device << 11) | (function << 8) | (reg & ~3));
	*value = InPortW(CONFIG1_DATA + (reg & 3));
}

void PCIWriteConfigB16(UINT16 bus, UINT8 device, UINT8 function, UINT8 reg, UINT16 value) {
	OutPortL(CONFIG1_ADR, PCI_DIRECT_ACC | (bus << 16) | (device << 11) | (function << 8) | (reg & ~3));
	OutPortW(CONFIG1_DATA + (reg & 3), value);
}

void PCIReadConfigB32(UINT16 bus, UINT8 device, UINT8 function, UINT8 reg, UINT32 *value) {
	OutPortL(CONFIG1_ADR, PCI_DIRECT_ACC | (bus << 16) | (device << 11) | (function << 8) | (reg & ~3));
	*value = InPortL(CONFIG1_DATA + (reg & 3));
}

void PCIWriteConfigB32(UINT16 bus, UINT8 device, UINT8 function, UINT8 reg, UINT32 value) {
	OutPortL(CONFIG1_ADR, PCI_DIRECT_ACC | (bus << 16) | (device << 11) | (function << 8) | (reg & ~3));
	OutPortL(CONFIG1_DATA + (reg & 3), value);
}


/*
 * Higher level functions
 */

UINT32 PCIDirectDetect(void) {
	UINT32 temp;
	UINT32 temp2;
	
	OutPortB(CONFIG1_ADR + 3, 0x01); /* why? */
	temp = InPortB(CONFIG1_ADR);
	OutPortL(CONFIG1_ADR, 0x80000000L);
	temp2 = InPortB(CONFIG1_ADR);
	if(temp2 == 0x80000000L)
	{
		OutPortL(CONFIG1_ADR, temp);
		return 0;
	}
	return 1;
}

/**
 * Appends a device to the devices list
 * @param list to add the device to
 * @param device to add
 */
static void AddDevice(PCIDeviceList *list, PCIDevice *device) {	
	if (list->tail != NULL) {
		/* list already exists, simply append */
		list->tail->next = device;
		list->tail = device;
	} else {
		/* create new list */
		list->root = device;
		list->tail = device;
	}
	list->numPCIDevices++;
}

static inline UINT32 GetAddressSize(PCIDevice *pci, UINT8 regNumber) {
	UINT32 size, temp;
	
	/* save former value */
	temp = pci->configSpace.PCIHeader.baseAddressReg[regNumber];
	
	if (temp == 0) {
		return 0;
	}
	
	/* write size command */
	PCIWriteConfigB32(pci->bus, pci->device, pci->function, PCIREG_BASE + PCIREG_SIZE * regNumber, 0xFFFFFFF0);
	PCIReadConfigB32(pci->bus, pci->device, pci->function, PCIREG_BASE + PCIREG_SIZE * regNumber, &size);
	
	/* write back old value */
	PCIWriteConfigB32(pci->bus, pci->device, pci->function, PCIREG_BASE + PCIREG_SIZE * regNumber, temp);
	
	if(temp & 0x00000001) {
		/* I/O */
		size |= 0xFFFF0000;
	}
	
	return -(size & ~0x0F); 
}

static inline void GetResources(PCIDevice *pci, UINT8 regNumber) {
	UINT32 resSize;
	UINT32 resStart = 0xFFFFFFFF;
	UINT32 regValue;
	
	resSize = GetAddressSize(pci, regNumber);
	if (resSize) {
		regValue = pci->configSpace.PCIHeader.baseAddressReg[regNumber];
		if (regValue & 0x00000001) {
			/* I/O space */
			pci->resource[regNumber].IOType = PCIResourceIsIO;
			
			resStart = (regValue & ~3);
			
			pci->resource[regNumber].range.rangeIO.IOStart = resStart;
			pci->resource[regNumber].range.rangeIO.IOLength = resSize;
		} else {
		/* memory space */
			pci->resource[regNumber].IOType = PCIResourceIsMemory;
			
			switch ((regValue & 6) >> 1) {
				case 0:
					resStart = (regValue & 0xFFFFFFF0L);
					break ;
				case 1:
					resStart = (regValue & 0x00FFFFF0L);
					break ;
				case 2:
					// printf("base=%8.08lX%8.08lX ",nextbase,(base & ~0x0F)) ;
					if (regNumber < 6) {
						resStart = (pci->configSpace.PCIHeader.baseAddressReg[regNumber] << 16) + (regValue & ~0x0F);
					}
					break ;
				case 3:
					/* reserved */
					break ;
			}
			if ((regValue & 8) != 0) {
				pci->resource[regNumber].range.rangeMem.MemoryType = PCIMemTypePrefetchable;
			} else {
				pci->resource[regNumber].range.rangeMem.MemoryType = PCIMemTypeStandard;
			}
			
			pci->resource[regNumber].range.rangeMem.MemoryStart = resStart;
			pci->resource[regNumber].range.rangeMem.MemoryLength = resSize;
		}
	} else {
		pci->resource[regNumber].IOType = PCIResourceUnused;
		pci->resource[regNumber].range.rangeIO.IOStart = 0;
		pci->resource[regNumber].range.rangeIO.IOLength = 0;
	}
}

/**
 * Creates a new PCIDevice strtucture containing the 64 byte header of the config space
 * @param busNum PCI bus number
 * @param device PCI device number
 * @param function PCI function number of the device
 * @return pointer to the PCIDevice structure on success, NULL otherwise
 */
static PCIDevice *CreatePCIDevice(UINT8 busNum, UINT8 device, UINT8 function) {
	PCIDevice *pci = malloc(sizeof(PCIDevice));
	UINT32 dword;
	UINT32 value;
	UINT8 res;
	
	if (pci) {
		pci->bus = busNum;
		pci->device = device;
		pci->function = function;
		pci->next =  NULL;
		
		/* copy whole config space to structure */
		/** @note Config space is 64 bytes (= 16 DWords) */
		for (dword = 0; dword < 16; dword++) {
			PCIReadConfigB32(busNum, device, function, dword << 2, &value);
			pci->configSpace.configDWords[dword] = value;
		}
		
		/* evaluate resources */
		for (res = 0; res < 6; res++) {
			GetResources(pci, res);
		}
		
		return pci;
		
	} else {
		return NULL;
	}
}

PCIDeviceArray *ConvertToArray(PCIDeviceList *list) {
	PCIDeviceArray *array;
	PCIDevice *arrayIndex, *oldDevice, *nextDevice;
	
	UINT32 currDev;
	
	/* create array structure */
	array = malloc(sizeof(PCIDeviceArray));
	if (!array) {
		/* out of memory */
		return FALSE;
	}
	
	
	
	array->numPCIDevices = list->numPCIDevices;
	array->list = malloc(sizeof(PCIDevice) * array->numPCIDevices);
	if (array->list) {
		arrayIndex  = array->list;
		oldDevice = list->root;
		
		/* copy list to contigous space (array) */
		for (currDev = 0; currDev < array->numPCIDevices; currDev++) {
			memcpy(arrayIndex, oldDevice, sizeof(PCIDevice));
			
			/* delete old pointer */
			arrayIndex->next = NULL;
			
			/* preserve pointer to next element, as we're going to free the memory of the current one */
			nextDevice = oldDevice->next;
			
			/* free memory */
			free(oldDevice);
			
			oldDevice = nextDevice;
			arrayIndex++;	
		}
		
		return array;
	} else {
		/* out of memory */
		return NULL;
	}
}


PCIDeviceArray *PCIScanBuses() {
	UINT8 busNum, device, function = 0;
	
	UINT8 hdrType = 0;
	UINT32 vendorDevice;
	UINT8 numFunctions;
	
	PCIDevice *pci;
	PCIDeviceList pciDevices = {.root = NULL, .tail = NULL, .numPCIDevices = 0};

	for (busNum = 0; busNum < 8; busNum++) {
		for (device = 0; device < 32; device++) {
			/* valid device? */
			PCIReadConfigB32(busNum, device, 0, REG_VENDOR, &vendorDevice);
			if (!INVALID(vendorDevice)) {
				/* check for multi function device */
				PCIReadConfigB8(busNum, device, 0, REG_HEADER, &hdrType);
				
				if (BITSET(hdrType, BIT_MULTI)) {
					/* it's a multi-function device */
					numFunctions = 8;
				} else {
					numFunctions = 1;
				}
				
				for (function = 0; function < numFunctions; function++) {
					PCIReadConfigB32(busNum, device, function, REG_VENDOR, &vendorDevice);
					if (!INVALID(vendorDevice)) {
						pci = CreatePCIDevice(busNum, device, function);
						if (pci) {
							AddDevice(&pciDevices, pci);
						} else {
							/* an error occurred */
							return NULL;
						}
					}
					
				}
			}
		}
	}
	return ConvertToArray(&pciDevices);
}
