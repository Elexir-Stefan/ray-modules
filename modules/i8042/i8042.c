#include <raykernel.h>
#include <tdm/io.h>
#include <tdm/int.h>
#include <drivers/dmesg/dmesg.h>
#include <memory/memory.h>

#include <rmi/rmi.h>
#include <threads/sleep.h>
#include <threads/threads.h>

#include "8042.h"
#include "i8042io.h"
#include "keymap_german.h"

/* other exports */
#include <pid/pids.h>

#include "exports.h"
#include "keyboard.h"


#define TIMEOUT 10000

#define ENODEV	2
#define EIO	3

#define SETBIT(var, bit, onoff) var = (onoff)? var | (bit) : var & (~(bit))

UINT8 I8042InitialCtr;
UINT8 I8042Ctr;
UINT8 I8042Reset = 0;

THREADLIST *listRoot = NULL;

static void NotifyObservers(UINT32 value);

static UINT32 I8042Flush(void) {
	UINT16 tries = TIMEOUT;
	while((I8042ReadStatus() & I8042_STR_OBF) && (tries--)) {
		I8042ReadData();
	}
	return TIMEOUT - tries;
	
}

static SINT32 I8042WaitRead(void) {
	UINT16 tries = TIMEOUT;
	while((~I8042ReadStatus() & I8042_STR_OBF) && (tries--));
	return -(tries == 0);
}

static SINT32 I8042WaitWrite(void) {
	UINT16 tries = TIMEOUT;
	while((I8042ReadStatus() & I8042_STR_IBF) && (tries--));
	return -(tries == 0);
}

static SINT32 I8042Command (UINT8 *param, UINT32 command) {
	SINT32 i, retval, auxerr = 0;

	if ((retval = I8042WaitWrite())) {
		return retval;
	}

	I8042WriteCommand(command & 0xff);

	for (i = 0; i < ((command >> 12) & 0xf); i++) {
		if ((retval = I8042WaitWrite())) {
			return retval;
		}
		I8042WriteData(param[i]);
	}

	for (i = 0; i < ((command >> 8) & 0xf); i++) {
		if ((retval = I8042WaitRead())) {
			return retval;
		}

		if (command == I8042_CMD_AUX_LOOP &&
		    !(I8042ReadStatus() & I8042_STR_AUXDATA)) {
			retval = auxerr = -1;
			return retval;
		}

		param[i] = I8042ReadData();
	}

	if (retval) {
		dmesg("i8042 (auxerr)");
	}

	return retval;
}

/*
 * i8042_enable_kbd_port enables keybaord port on chip
 */

static SINT32 I8042EnableKBDPort(void) {
	I8042Ctr &= ~I8042_CTR_KBDDIS;
	I8042Ctr |= I8042_CTR_KBDINT;

	if (I8042Command(&I8042Ctr, I8042_CMD_CTL_WCTR)) {
		dmesg("i8042.c: Failed to enable KBD port");
		return -EIO;
	}

	return 0;
}

/*
 * i8042_enable_aux_port enables AUX (mouse) port on chip
 */

static __attribute__((unused)) SINT32 I8042EnableAuxPort(void) {
	I8042Ctr &= ~I8042_CTR_AUXDIS;
	I8042Ctr |= I8042_CTR_AUXINT;

	if (I8042Command(&I8042Ctr, I8042_CMD_CTL_WCTR)) {
		dmesg("i8042.c: Failed to enable AUX port");
		return -EIO;
	}

	return 0;
}

static SINT32 I8042ControllerCheck(void) {

	if (I8042Flush() == I8042_BUFFER_SIZE) {
		dmesg("i8042.c: No controller found");
		return -ENODEV;
	}

	return 0;

}

static SINT32 I8042ControllerSelftest(void) {
	unsigned char param;
	
	if (!I8042Reset)
		return 0;

	if (I8042Command(&param, I8042_CMD_CTL_TEST)) {
		dmesg("i8042.c: i8042 controller self test timeout");
		return -ENODEV;
	}

	if (param != I8042_RET_CTL_TEST) {
		dmesg("i8042.c: i8042 controller selftest failed");
		return -EIO;
	}

	return 0;
}


static SINT32 I8042Probe(void) {
	int error;

	error = I8042ControllerSelftest();
	if (error) return error;

	error = I8042ControllerInit();
	if (error) return error;
	
	error = I8042EnableKBDPort();
	if (error) return error;

	return 0;
}

static SINT32 I8042Init(void) {
	int err;


	err = I8042ControllerCheck();
	if (err) return err;

	return 0;
}

static SINT32 I8042ControllerInit(void) {
	UINT8 i8042_direct = 0;
	
	/*
	* Save the CTR for restoral on unload / reboot.
	*/

	if (I8042Command(&I8042Ctr, I8042_CMD_CTL_RCTR)) {
		dmesg("i8042.c: Can't read CTR while initializing i8042");
		return -EIO;
	}

	I8042InitialCtr = I8042Ctr;

	/*
	* Disable the keyboard interface and interrupt.
	*/

	I8042Ctr |= I8042_CTR_KBDDIS;
	I8042Ctr &= ~I8042_CTR_KBDINT;

	/*
	* Handle keylock.
	*/

	if (~I8042ReadStatus() & I8042_STR_KEYLOCK) {
			dmesg("i8042.c: Warning: Keylock active");
	}

	/*
	* If the chip is configured into nontranslated mode by the BIOS, don't
	* bother enabling translating and be happy.
	*/

	if (~I8042Ctr & I8042_CTR_XLATE)
		i8042_direct = 1;

	/*
	* Set nontranslated mode for the kbd interface if requested by an option.
	* After this the kbd interface becomes a simple serial in/out, like the aux
	* interface is. We don't do this by default, since it can confuse notebook
	* BIOSes.
	*/

	if (i8042_direct)
		I8042Ctr &= ~I8042_CTR_XLATE;

	/*
	* Write CTR back.
	*/

	if (I8042Command(&I8042Ctr, I8042_CMD_CTL_WCTR)) {
		dmesg("i8042.c: Can't write CTR while initializing i8042");
		return -EIO;
	}

	return 0;
}


/*
 * Reset the controller and reset CRT to the original value set by BIOS.
 */

static void I8042ControllerReset(void) {
	I8042Flush();

	/*
	* Reset the controller if requested.
	*/

	I8042ControllerSelftest();

	/*
	* Restore the original control register setting.
	*/

	if (I8042Command(&I8042InitialCtr, I8042_CMD_CTL_WCTR))
		dmesg("i8042.c: Can't restore CTR");
}


CALLBACK doReboot(RMISERIAL sender, UINT32 args, UINT32 msgID) {
	I8042ControllerReset();
	I8042Flush();
	I8042WriteCommand(I8042_CMD_REBOOT);
}


/**
 * Acknowledge the recieval
 */

void Recieved(void) {
	UINT8 oldVal, disabledVal;
	
	 oldVal = InPortB(KBD_PORT_DIS);
	disabledVal = oldVal | 0x80;
	OutPortB(KBD_PORT_DIS, disabledVal);	// Disable
	OutPortB(KBD_PORT_DIS, oldVal);		// Enable again
	
}

/**
 * will be called by the ISR
 */
CALLBACK KeyboardHandler(UINT32 irqNum, UINT32 pending, UINT32 msgID) {
	static UINT32 addKeys = 0;
	static BOOL escaped = FALSE;
	UINT8 scanCode;
	UINT32 keyCode;
	BOOL keyUp;
	unsigned char translatedCode;
		
	I8042Command(NULL, I8042_CMD_KBD_DISABLE);
	
	while(I8042ReadStatus() & I8042_STR_OBF) {
		scanCode = I8042ReadData();
		keyCode = SP_KEY_NONE;
		keyUp = FALSE;
		
		Recieved();
		
		if (scanCode == I8042_CODE_ESCAPE) {
			escaped = TRUE;
		} else {
		
			// if key released, clear the bit in order to properly
			// identify the scan code
			if(scanCode & EVNT_KEYUP) {
				keyUp = TRUE;
				scanCode &= ~EVNT_KEYUP;
			}
			
			// escaped scan code?
			if (escaped) {
				keyCode += 256;
				escaped = FALSE;
			}
			
			switch (scanCode) {
			case KEY_CTRL:
				SETBIT(addKeys, ADDKEYS_CTRL, !keyUp);
				break;
			case KEY_ALT:
				SETBIT(addKeys, ADDKEYS_ALT, !keyUp);
				break;
			case KEY_SHIFT:
				SETBIT(addKeys, ADDKEYS_SHIFT, !keyUp);
				break;
			default:
				if (addKeys & ADDKEYS_SHIFT) {
					translatedCode = keyMapShifted[scanCode];
				} else {
					translatedCode = keyMapUnshifted[scanCode];
				}
				
				if (!translatedCode) {
					// use scan code instead
					translatedCode = scanCode;
				}
				keyCode |= translatedCode;
			}
				
			keyCode += addKeys;
			
			// currently only notify them on key down
			if(!keyUp) {
				NotifyObservers(keyCode);
			}
		}
	}
	
	I8042Command(NULL, I8042_CMD_KBD_ENABLE);

	InterruptDone(irqNum);
}

CALLBACK AddObserver(RMISERIAL sender, UINT32 exportID, UINT32 msgID) {
	THREADLIST *newObs;
	
	newObs = (THREADLIST*)malloc(sizeof(THREADLIST));
	newObs->process = sender;
	newObs->funcExport = exportID;
	newObs->next = listRoot;
	
	listRoot = newObs;
	
	ThreadExit(0);
}

CALLBACK DelObserver(RMISERIAL sender, UINT32 value, UINT32 msgID) {
	/* scan linear through list ... */
	THREADLIST *scan = listRoot;
	THREADLIST *prev = NULL;
	
	while (scan) {
		if (scan->process == sender) {
			if (prev) {
				prev->next = scan->next;
			} else {
				if (listRoot) {
					listRoot = scan->next;
				}
			}
			free(scan);
			ThreadExit(0);
		} else {
			prev = scan;
			scan = scan->next;
		}
	}
	
	ThreadExit(0);
}

static void NotifyObservers(UINT32 value) {
	THREADLIST *receiver = listRoot;
	
	
	/* walk through list and notify all observers */
	while (receiver) {
		RMInvoke(receiver->process, receiver->funcExport, value);
		receiver = receiver->next;
	}
}

static void InitDriver()
{
	RequestIOPort(KBD_PORT_DATA);
	RequestIOPort(KBD_PORT_DIS);
	RequestIOPort(KBD_PORT_CMD);
	
	I8042Init();
	I8042Probe();
	
	RegisterIRQ(1, KeyboardHandler);
	RegisterIRQ(12, KeyboardHandler);
	
	BarrierGo(KEYBOARD_BARRIER, TRUE);
}

static void InitializeThread()
{
	ARGUMENTS args;
	
	args.count = 0;
	args.values = NULL;
	
	PID_HANDLE handle;
	
	ThreadCreate(InitDriver, RAY_PRIO_NORMAL, &args, "InitKeyboard", &handle);
	
}


RAYENTRY KernelModuleEntry(char *arguments) {

	BarrierCreate(KEYBOARD_BARRIER, FALSE);
		
	RMISetup(DRV_KEYBOARD, 3);
	RMIRegisterValue(DO_REBOOT, doReboot, 255, FALSE);
	RMIRegisterValue(RMIProcessAttach, AddObserver, 255, FALSE);
	RMIRegisterValue(RMIProcessDetach, DelObserver, 255, FALSE);
	
	InitializeThread();
	
	dmesg ("i8042 driver loaded.");
	
	for(;;) Sleep();
	
}
