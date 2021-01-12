#include <raykernel.h>
#include <syscall.h>
#include <debug/debug.h>
#include <sct/debug.h>

void SYSTEM _BREAKPOINT() {
    SysCallN(SYS_BREAKPOINT, 0);
}

void SYSTEM ProfilingEnable(UINT32 entries) {
    SysCallN(SYS_PROFILE_SETUP, &entries);
}

void SYSTEM ProfilingStart() {
    SysCallN(SYS_PROFILE_START, 0);
}

void SYSTEM ProfilingFlush() {
    SysCallN(SYS_PROFILE_FLUSH, 0);
}

void SYSTEM ProfilingStop() {
    SysCallN(SYS_PROFILE_STOP, 0);
}

void SYSTEM ProfilingReset() {
    SysCallN(SYS_PROFILE_RESET, 0);
}

UINT32 SYSTEM ProfilingGetSize() {
    UINT32 res;
    SysCall(SYS_PROFILE_SIZE, 0, res);
    return res;
}

UINT32 SYSTEM ProfilingGetUsed(){
    UINT32 res;
    SysCall(SYS_PROFILE_RCOUNT, 0, res);
    return res;
}
