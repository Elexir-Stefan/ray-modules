#include <raykernel.h>
#include <syscall.h>
#include <tdm/io.h>
#include <sct/tdm.h>

RAY_TDM SYSTEM RequestIOPort(volatile UINT16 port) {
	UINT32 result;
	volatile UINT32 port32 = port;
	SysCall(SYS_REQUEST_IO_PORT, &port32, result);
	return (RAY_TDM)result;
}
