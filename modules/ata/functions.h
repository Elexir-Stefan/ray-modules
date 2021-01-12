#ifndef _FUNCTIONS_H
#define _FUNCTIONS_H

/**
 * Detects ata drives and returns the number of found drives
 */
UINT32 DetectDrives();

/**
 * Writes information about found drives
 */
void SavePartitionLayout();

CALLBACK CHARReadSector(RMISERIAL sender, UINT32 sector, UINT32 msgID);


#endif
