#include <raykernel.h>
#include <kdisplay/kprintf.h>
#include <memory/memory.h>
#include <threads/sleep.h>


RAYENTRY KernelModuleEntry(UINT32 value1, UINT32 value2, char *arguments) {
	
	int *test = malloc(100);
	
	KPrintf ("Hello, world!\n");
	KPrintf ("Overwriting test...\n");
	
	
	test[100] = 666;
	
	KPrintf ("Hm.... succeeded!");
	
	exit(0);
}
