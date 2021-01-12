#include <raykernel.h>
#include <syscall.h>
#include <threads/sleep.h>
#include <sct/tasks.h>

void SYSTEM Sleep(void) {
	SysCallN(SYS_SLEEP, 0);
}


void SYSTEM Relinquish() {
	SysCallN(SYS_RELINQUISH, 0);
}

void SYSTEM Pause(UINT32 msecs) {
	SysCallN(SYS_SLEEP_INTV, &msecs);
}
