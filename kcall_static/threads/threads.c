#include <raykernel.h>
#include <syscall.h>
#include <threads/threads.h>
#include <sct/threads.h>

void __exitThreadCallBack();

void __ExitThreadHelper() {
    // uses the return value (in eax) as function argument to ExitThread
    __asm__ __volatile__ ("__exitThreadCallBack:\n"
			    "pushl %eax\n"
			    "call ThreadExit");
}

BOOL ThreadCreate(POINTER entryPoint, PRIORITY prio, ARGUMENTS *args, String threadName, PID_HANDLE* handle) {
    return ThreadCreateReturn(entryPoint, __exitThreadCallBack, prio, args, threadName, handle);
}

BOOL SYSTEM ThreadCreateReturn(POINTER entryPoint, POINTER returnAddress, PRIORITY prio, ARGUMENTS *args, String threadName, PID_HANDLE* handle) {
	UINT32 result;
	SysCall(SYS_THREAD_CREATE, &entryPoint, result);
	return (BOOL)result;
}

void SYSTEM ThreadExit(UINT32 exitCode) {
	SysCallN(SYS_THREAD_EXIT, &exitCode);
}

THREAD_INFO_LIST* SYSTEM GetThreadInfo(UINT32 pid) {
	THREAD_INFO_LIST* result;
	SysCall(SYS_THREAD_INFO, &pid, result);
	return result;
}

SINT32 SYSTEM ThreadJoin(PID_HANDLE* handle) {
	SINT32 result;
	SysCall(SYS_THREAD_JOIN, &handle, result);
	return result;
}

BOOL SYSTEM ThreadAbort(PID_HANDLE* handle) {
	UINT32 result;
	SysCall(SYS_THREAD_ABORT, &handle, result);
	return (BOOL)result;
}

BOOL SYSTEM IsThreadAlive(PID_HANDLE* handle) {
	UINT32 result;
	SysCall(SYS_THREAD_ALIVE, &handle, result);
	return (BOOL)result;	
}

BOOL SYSTEM ThreadNotify(PID_HANDLE* handle) {
	UINT32 result;
	SysCall(SYS_THREAD_NOTIFY, &handle, result);
	return (BOOL)result;
}

void SYSTEM ThreadGet(PID_HANDLE* handle) {
	SysCallN(SYS_THREAD_GET, &handle);
}
