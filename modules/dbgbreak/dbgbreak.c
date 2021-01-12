#include <raykernel.h>
#include <kdisplay/kprintf.h>
#include <threads/sleep.h>
#include <rmi/rmi.h>
#include <debug/debug.h>
#include <threads/threads.h>
#include <pid/pids.h>

#include <drivers/keyboard/raykeybind.h>
#include "../i8042/exports.h"
#include "../i8042/usrkbd.h"

#define EXPORT_GET_CH	0

void ProfileStart() {
    // ~5 MB
	ProfilingEnable(436906);
	ProfilingStart();
}

void stealSemaphore() {
    IPLOCK stolen = SemaphoreGet("testlock");
    switch (stolen) {
	case SEMAPHORE_ILLEGAL:
	    KPrintf ("Semaphore not valid!\n");
	    break;
	case SEMAPHORE_DENIED:
	    KPrintf ("Could not steal semaphore. Access denied!\n");
	    break;
	default:
	    SemaphoreEnter(stolen);
    }
}

void releaseSemaphore() {
    IPLOCK stolen = SemaphoreGet("testlock");
    switch (stolen) {
	case SEMAPHORE_ILLEGAL:
	    KPrintf ("Semaphore not valid!\n");
	    break;
	case SEMAPHORE_DENIED:
	    KPrintf ("Could not steal semaphore. Access denied!\n");
	    break;
	default:
	    SemaphoreLeave(stolen);
    }
}

CALLBACK getCh(RMISERIAL sender, UINT32 keyCode, UINT32 msgID) {
	if ((keyCode & ADDKEYS_CTRL) && ((keyCode & 0xff) == 'c')) {
		_BREAKPOINT();
	} else if ((keyCode & ADDKEYS_CTRL) && ((keyCode & 0xff) == 's')) {
		KPrintf ("Stealing semaphore...\n");
		stealSemaphore();
	} else if ((keyCode & ADDKEYS_CTRL) && ((keyCode & 0xff) == 'd')) {
		KPrintf ("Releasing semaphore...\n");
		releaseSemaphore();
	} else if ((keyCode & ADDKEYS_CTRL) && ((keyCode & 0xff) == '+')) {
		KPrintf ("#");
		ProfileStart();
	} else if ((keyCode & ADDKEYS_CTRL) && ((keyCode & 0xff) == '-')) {
		KPrintf ("[FLUSHING...");
		ProfilingFlush();
		KPrintf ("done]\n");
	}
	
	ThreadExit(0);
}

RAYENTRY KernelModuleEntry(void) {
	
	RMISetup(0x76F2756C, 1);
	
	BarrierArrive(KEYBOARD_BARRIER);
	RMIRegisterValue(EXPORT_GET_CH, getCh, 255, FALSE);
	
	RAY_RMI result = RMInvoke(DRV_KEYBOARD, RMIProcessAttach, EXPORT_GET_CH);
	switch(result) {
	case RMI_SUCCESS:
		break;
			
	default:
		KPrintf ("dbgbreak: Error %u while contacting keyboard driver.Exiting\n",result);
		exit(2);
	}
	
	for(;;) {
		Sleep();
	}
}
