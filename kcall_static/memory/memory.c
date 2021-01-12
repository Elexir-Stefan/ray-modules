#include <raykernel.h>
#include <syscall.h>
#include <memory/memory.h>
#include <sct/memory.h>

void* SYSTEM malloc(UINT32 size) {
	void* result;
	SysCall(SYS_MALLOC, &size, result);
	return result;
}

void* SYSTEM free(void *pointer) {
	void* result;
	SysCall(SYS_FREE, &pointer, result);
	return result;
}
