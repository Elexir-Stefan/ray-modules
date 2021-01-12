#include <raykernel.h>
#include <syscall.h>
#include <tdm/tdm.h>
#include <sct/tdm.h>

UINT32 SYSTEM GetPrivLevel(void) {
	UINT32 result;
	SysCall(SYS_GET_PRIV_LEVEL, 0, result);
	return result;
}
