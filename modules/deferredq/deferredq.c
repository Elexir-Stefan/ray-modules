#include <raykernel.h>
#include <threads/sleep.h>
#include <rmi/rmi.h>
#include <string.h>

static BOOL kindOfLock = FALSE;

CALLBACK defer(RMISERIAL sender, RMIMESSAGE msg, UINT32 msgID) {

	String message = (String)msg;
	
	if (kindOfLock == TRUE) {
	    strcpy(message, "!!! ### ERROR ### !!!");
	}
	kindOfLock = TRUE;
	   
	
	UINT32 len = strlen(message);
	message[len] = '0' + (len % 10);
	len++;
	message[len] = '\0';
	
	Pause(50 + 100 * (len % 10));
	
	kindOfLock = FALSE;
	CReturn(msg, msgID, TRUE);
}

RAYENTRY KernelModuleEntry(void) {
	
	RMISetup(0x9876A7C9, 1);
	
	RMIRegister(0, defer, 255, FALSE, RTYPE_STRING, 1024);
	
	for(;;) {
		Sleep();
	}
}
