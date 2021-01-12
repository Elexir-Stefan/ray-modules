#ifndef _LOWLEVEL_H
#define _LOWLEVEL_H

#include "drives.h"

/**
 * Detects wether a controller is present or not
 * @param controller structure identifying the controller to test
 * @return 0 if not found, 1 if found
 * @note BOOL not used here to easily add found controllers
 */
UINT32 DetectController(volatile HDD *controller);

BOOL ReadSector(volatile HDD *controller, UINT64 sectorAddress, UINT16 sectorCount, UINT8 *buffer);
void ataIRQ(UINT32 irqNum, UINT32 pending, UINT32 msgID);
void InitializeControllers();

#endif
