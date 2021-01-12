#include <raykernel.h>
#include <threads/sleep.h>

RAYENTRY KernelModuleEntry(void) {
	
	for(;;) {
		Sleep();
	}
}
