#ifndef _RAY_PROCESS_H
#define _RAY_PROCESS_H

#include <ray/arguments.h>
#define MEM_ATTRIB_READABLE	0x01
#define MEM_ATTRIB_WRITABLE	0x02
#define MEM_ATTRIB_EXECUTABLE	0x04

typedef struct {
	UINT32 *segmentStart;
	UINT32 segmentLength;
	UINT32 loadAddress;
	UINT16 memoryAttributes;
}__attribute__((packed)) LOAD_MEM_SEGMENT;

typedef struct {
	UINT32 allocPointer;
	UINT16 memoryAttributes;
}__attribute__((packed)) LOAD_ADD_MEM;

typedef struct {
	char name[64];
	PRIORITY priority;
	PRIVLEVEL privilegeLevel;
	UINT32 entryPoint;
	ARGUMENTS arguments;
}__attribute__((packed)) LOAD_PROCESS_INFO;

typedef struct {
	UINT16 numMemSegments;
	UINT16 numAddMemory;
	LOAD_MEM_SEGMENT *loadableSegments;
	LOAD_ADD_MEM *additionalMemory;
	LOAD_PROCESS_INFO processInfo;
}__attribute__((packed)) LOAD_PROCESS;

BOOL ProcessLoad(LOAD_PROCESS *procInfo);

#endif
