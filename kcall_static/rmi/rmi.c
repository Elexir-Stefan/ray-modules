#include <raykernel.h>
#include <syscall.h>
#include <rmi/rmi.h>
#include <sct/ipc.h>

/* system calls */

volatile RAY_RMI SYSTEM AllocateMessageBuffer(UINT32 reqSize, MSG_TYPE messageType) {
	UINT32 result;
	//UINT32 arguments[] = {reqSize, messageType};
	SysCall(SYS_ALLOC_MB, &reqSize, result);
	return result;
}
RAY_RMI SYSTEM FreeMessageBuffer(RMIMESSAGE messageBuffer) {
	UINT32 result;
	SysCall(SYS_FREE_MB, &messageBuffer, result);
	return result;
}
	
RAY_RMI SYSTEM _RMInvoke(RMISERIAL appSerial, RMIFUNCTION exportID, UINT32 messageSend, BOOL blocking, RMIMESSAGE *messageRecv, MSG_TYPE expectedResultType, UINT32 maxResultLength) {
	UINT32 result;
	//UINT32 arguments[] = {(UINT32) appSerial, (UINT32)exportID, messageSend, (UINT32)blocking, (UINT32)messageRecv, (UINT32)expectedResultType, maxResultLength};
	SysCall(SYS_RM_INVOKE, &appSerial, result);
	return result;
}

RAY_RMI SYSTEM _RMPassMessage(RMISERIAL appSerial, RMIFUNCTION exportID, RMIMESSAGE messageSend, BOOL blocking, RMIMESSAGE *messageRecv, MSG_TYPE expectedResultType, UINT32 maxResultLength) {
	UINT32 result;
	//UINT32 arguments[] = {(UINT32)appSerial, (UINT32)exportID, (UINT32) messageSend, (UINT32) blocking, (UINT32) messageRecv, (UINT32) expectedResultType, maxResultLength};
	SysCall(SYS_RM_PASS_MSG, &appSerial, result);
	return result;
}
RAY_RMI SYSTEM RMISetup(UINT32 mySerial, UINT32 count) {
	UINT32 result;
	SysCall(SYS_RMI_SETUP, &mySerial, result);
	return result;
}

RAY_RMI SYSTEM RMIRegister(RMIFUNCTION funcUID, void *entryPoint, PRIVLEVEL minPriv, BOOL partnership, MSG_TYPE expectedArgumentType, UINT32 maxArgumentLength) {
	UINT32 result;
	//UINT32 arguments[] = {(UINT32) funcUID, (UINT32)entryPoint, (UINT32)minPriv,(UINT32)partnership, (UINT32) expectedArgumentType, maxArgumentLength};
	SysCall(SYS_RMI_REGISTER, &funcUID, result);
	return result;
}

void SYSTEM CReturn(RMIMESSAGE message, UINT32 msgUID, BOOL isMemory) {
	//UINT32 arguments[] = {(UINT32)message, msgUID, (UINT32)isMemory};
	SysCallN(SYS_RETURN, &message);
	for(;;);
}
