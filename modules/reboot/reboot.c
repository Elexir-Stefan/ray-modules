#include <raykernel.h>
#include <kdisplay/kprintf.h>
#include <threads/sleep.h>
#include <rmi/rmi.h>

#include <pid/pids.h>

#include <drivers/keyboard/raykeybind.h>
#include "../i8042/exports.h"
#include "../i8042/usrkbd.h"

#define EXPORT_GET_CH	0

CALLBACK getCh(RMISERIAL sender, UINT32 keyCode, UINT32 msgID) {
	if ((keyCode & ADDKEYS_CTRL) && (keyCode & ADDKEYS_ALT) && ((keyCode & 0xff) == SP_KEY_DEL)) {
		KPrintf ("Rebooting ",0,0);
		RMInvoke(DRV_KEYBOARD, DO_REBOOT, 0);
	}
	
	ThreadExit(0);
}

RAYENTRY KernelModuleEntry(void) {
	int i;
	
	RMISetup(DRV_REBOOT, 1);
	
	for (i = 0; i < 5; i++) {
		Relinquish();
	}
	
	switch(RMInvoke(DRV_KEYBOARD, RMIProcessAttach, EXPORT_GET_CH)) {
	case RMI_SUCCESS:
		break;
			
	default:
		KPrintf ("Reboot: Error while contacting keyboard driver.Exiting\n",0,0);
		exit(2);
	}
	
	RMIRegisterValue(EXPORT_GET_CH, getCh, 255, FALSE);
	
	for(;;) {
		Sleep();
	}
}
