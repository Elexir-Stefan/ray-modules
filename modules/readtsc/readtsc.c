#include <raykernel.h>
#include <rmi/rmi.h>
#include <threads/sleep.h>

#define READTSC(var) __asm__ __volatile__("rdtsc":"=a"(var)::"edx")

RAYENTRY KernelModuleEntry(void) {
	UINT32 stop;
	READTSC(stop);
	
	RMISetup(0xD82BDA27, 1);
	RMInvoke(0x12345678, 1, stop);
	
	exit(0);
}
