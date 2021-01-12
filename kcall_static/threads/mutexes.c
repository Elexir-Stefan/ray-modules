#include <raykernel.h>
#include <syscall.h>
#include <threads/threads.h>
#include <sct/threads.h>

IPLOCK SYSTEM SemaphoreCreate(CString lockName, SINT32 initialValue, BOOL othersAllowed) {
	UINT32 result;
	//UINT32 arguments[] = {(UINT32)lockName, (UINT32)initialValue, (UINT32)othersAllowed};
	SysCall(SYS_IP_LOCK_CREATE, &lockName, result);
	return (IPLOCK)result;
}

IPLOCK SYSTEM SemaphoreGet(CString lockName) {
	UINT32 result;
	SysCall(SYS_IP_LOCK_GET, &lockName, result);
	return (IPLOCK)result;
}

SEMAPHORE_STATUS SYSTEM SemaphoreEnter(IPLOCK lockUID) {
	UINT32 result;
	SysCall(SYS_IP_LOCK_ENTER, &lockUID, result);
	return (SEMAPHORE_STATUS)result;
}

SEMAPHORE_STATUS SYSTEM SemaphoreLeave(UINT32 lockUID) {
	UINT32 result;
	SysCall(SYS_IP_LOCK_LEAVE, &lockUID, result);
	return (SEMAPHORE_STATUS)result;
}

SEMAPHORE_STATUS SYSTEM SemaphoreStatus(IPLOCK lockUID) {
	UINT32 result;
	SysCall(SYS_IP_LOCK_STATUS, &lockUID, result);
	return (SEMAPHORE_STATUS)result;
}

SEMAPHORE_STATUS SYSTEM SemaphoreDestroy(IPLOCK lockUID) {
	UINT32 result;
	SysCall(SYS_IP_LOCK_DESTROY, &lockUID, result);
	return (SEMAPHORE_STATUS)result;
}

BOOL SYSTEM BarrierCreate(CString barrierName, BOOL othersAllowed) {
	UINT32 result;
	SysCall(SYS_BARRIER_CREATE, &barrierName, result);
	return (BOOL)result;
}

BOOL SYSTEM BarrierArrive(CString barrierName) {
	UINT32 result;
	SysCall(SYS_BARRIER_ARRIVE, &barrierName, result);
	return (BOOL)result;
}

BOOL SYSTEM BarrierGo(CString barrierName, BOOL wakeForeignProcesses) {
	UINT32 result;
	//UINT32 arguments[] = {(UINT32)barrierName, (UINT32)wakeForeignProcesses};
	SysCall(SYS_BARRIER_GO, &barrierName, result);
	return (BOOL)result;
}

BOOL SYSTEM BarrierClose(CString barrierName) {
	UINT32 result;
	SysCall(SYS_BARRIER_CLOSE, &barrierName, result);
	return (BOOL)result;
}
