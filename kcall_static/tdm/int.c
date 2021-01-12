#include <raykernel.h>
#include <syscall.h>
#include <tdm/int.h>
#include <sct/tdm.h>

RAY_TDM SYSTEM RegisterIRQ(UINT32 irqNum, void* functionAddress) {
	UINT32 result;
	SysCall(SYS_REGISTER_IRQ, &irqNum, result);
	return (RAY_TDM)result;
}

void SYSTEM InterruptDone(UINT32 irqNum) {
	SysCallN(SYS_INT_DONE, &irqNum);
}
