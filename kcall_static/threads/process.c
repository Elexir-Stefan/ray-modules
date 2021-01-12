#include <raykernel.h>
#include <syscall.h>
#include <threads/threads.h>
#include <threads/process.h>
#include <sct/process.h>

BOOL SYSTEM ProcessLoad(LOAD_PROCESS *procInfo) {
	UINT32 result;
	SysCall(SYS_LOAD_PROCESS, &procInfo, result);
	return (BOOL)result;
}

UINT32 SYSTEM GetPID() {
	UINT32 result;
	SysCall(SYS_GET_PID, 0, result);
	return result;
}
