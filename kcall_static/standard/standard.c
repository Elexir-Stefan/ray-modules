#include <raykernel.h>
#include <syscall.h>
#include <standard.h>
#include <sct/standard.h>

/**
 * Standard exit-function 
 */
void SYSTEM exit(volatile UINT32 result) {
	volatile UINT32 resPtr = result;
	SysCallN(SYS_EXIT, &resPtr);
	
	// GENERATE PAGE FAULT
	UINT32* crash = (UINT32*)NULL;
	*crash = 666;
	
	// keep compiler happy (about noreturn)
	for(;;);
}
