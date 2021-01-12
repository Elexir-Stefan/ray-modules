#ifndef _DRIVES_H
#define _DRIVES_H

#include "partitions.h"

typedef struct {
	BOOL isPresent;
	UINT64 capacity;
	
	UINT16 basePort;
	UINT16 ctrlPort;
	UINT8 irq;
	UINT16 isSlave;
	UINT8 drive;
	
	PARTITION_INFO *partitions;
	UINT32 numPartitions;
} HDD;


/**
 * usefull functions
 */
#define SPECCOMMAND(controller, port) (controller->basePort + port)

#define SECTOR_SIZE	512
#define NUM_HDD		4
#define NUM_PARTITIONS	24


#endif
