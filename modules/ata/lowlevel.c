#include <raykernel.h>
#include <tdm/io.h>
#include <rmi/rmi.h>
#include <threads/sleep.h>
#include <drivers/dmesg/dmesg.h>
#include <memory/memory.h>
#include <tdm/int.h>

#include "drives.h"
#include "ataio.h"

volatile IPLOCK waitForDataP;
volatile IPLOCK waitForDataS;


void InitializeControllers() {
	UINT8 CtrlPrim, CtrlSec;
	
	CtrlPrim = InPortB(ATA_CONTROL_REG_PRIM);
	CtrlSec = InPortB(ATA_CONTROL_REG_SECD);
	
	/* enable interrupts */
	CtrlPrim &= ~CTRL_nIEN;
	CtrlSec &= ~CTRL_nIEN;
	
	OutPortB(ATA_CONTROL_REG_PRIM, CtrlPrim);
	OutPortB(ATA_CONTROL_REG_SECD, CtrlSec);
}

UINT32 DetectController(volatile HDD *controller) {
	UINT8 result;

	/* detect wether controller exists */
	OutPortB(SPECCOMMAND(controller, PORT_LBA), CMD_MAGIC);
	result = InPortB(SPECCOMMAND(controller, PORT_LBA));
	if (result == CMD_MAGIC) {
		
		// controller exists

		// drive select
		OutPortB(SPECCOMMAND(controller, PORT_DRIVE_SELECT), CMD_TEST | controller->isSlave);

		// wait 
		Pause(50);

		result = InPortB(SPECCOMMAND(controller, PORT_STATUS));

		if (result & FLAG_READY) {
			controller->isPresent = TRUE;
			return 1;
		} else {
			return 0;
		}
	} else {
		return 0;
	}

}

CALLBACK ataIRQ(UINT32 irqNum, UINT32 pending, UINT32 msgID) {
	if (irqNum == 14) SemaphoreLeave(waitForDataP);
	if (irqNum == 15) SemaphoreLeave(waitForDataS);
	return;
}


 static UINT8 Wait400nsStatus(volatile HDD* controller) {
	UINT8 temp;
	
	temp = InPortB(controller->ctrlPort);
	temp = InPortB(controller->ctrlPort);
	temp = InPortB(controller->ctrlPort);
	temp = InPortB(controller->ctrlPort);
	
	// The real read
	temp = InPortB(controller->ctrlPort);
	
	return temp;
}

BOOL ReadSector(volatile HDD *controller, UINT64 sectorAddress, UINT16 sectorCount, UINT8 *bufferC) {
	UINT16* buffer = (UINT16*)bufferC;
	UINT8 result;
	UINT32 i, s;
	
	do {
		result = InPortB(controller->ctrlPort);
	} while(result & FLAG_BUSY);
	// read LBA48 sector
	
	/*
	OutPortB(SPECCOMMAND(controller, PORT_FEATURE), 0x00);
	OutPortB(SPECCOMMAND(controller, PORT_FEATURE), 0x00);
	*/
	
	// 16 Bit sector count (high byte only)
	OutPortB(SPECCOMMAND(controller, PORT_SECTOR_COUNT), (unsigned char)(sectorCount >> 8));
	
	// Mix the sectorAddress to different ports each subsequent write (faster than two writes to the same port)
	OutPortB(SPECCOMMAND(controller, PORT_LBA), (unsigned char)(sectorAddress >> 24));		// 24-31
	OutPortB(SPECCOMMAND(controller, PORT_CYLINDER_LSB), (unsigned char)(sectorAddress >> 32));	// 32-39
	OutPortB(SPECCOMMAND(controller, PORT_CYLINDER_MSB), (unsigned char)(sectorAddress >> 40));	// 40-47
	
	// 16 Bit sector count (low byte only)
	OutPortB(SPECCOMMAND(controller, PORT_SECTOR_COUNT), (unsigned char)sectorCount);
	
	// Mix the sectorAddress to different ports each subsequent write (faster than two writes to the same port)
	OutPortB(SPECCOMMAND(controller, PORT_LBA), (unsigned char)(sectorAddress));			// 00-07
	OutPortB(SPECCOMMAND(controller, PORT_CYLINDER_LSB), (unsigned char)(sectorAddress >> 8));	// 08-15
	OutPortB(SPECCOMMAND(controller, PORT_CYLINDER_MSB), (unsigned char)(sectorAddress >> 16));	// 16-23
	
	OutPortB(SPECCOMMAND(controller, PORT_DRIVE_SELECT), (0x40 | (controller->drive << 4)));
	
	do {
		result = InPortB(controller->ctrlPort);
	} while(!(result & FLAG_READY));

	// issue read-sector command 
	OutPortB(SPECCOMMAND(controller, PORT_COMMAND), 0x24);
	
	for(s = 0; s < sectorCount; s++) {
	    result = Wait400nsStatus(controller);
	    if (result & FLAG_ERR) {
		    dmesg ("ata: Error reading from drive");
		    return FALSE;
	    }

	    if (controller->irq == 14) SemaphoreEnter(waitForDataP);
	    if (controller->irq == 15) SemaphoreEnter(waitForDataS);


	    InterruptDone(controller->irq);
	    // read status register to clear interrupt bit
	    result = InPortB(SPECCOMMAND(controller, PORT_STATUS));
	    for (i = 0; i < 256; i++) {
		    *buffer = InPortW(SPECCOMMAND(controller, PORT_DATA));
		    buffer++;
	    }
	    
	    
	    
	}
	

	return TRUE;
}
