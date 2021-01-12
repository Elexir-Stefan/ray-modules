#ifndef _RAY_RMI_H
#define _RAY_RMI_H

#include <threads/thread_types.h>

/* callback definitions */
#define CALLBACK void __attribute__ ((stdcall))

#define RMINewStructure(adder, structure, start) UINT32 adder = (UINT32)(start) - (UINT32)(structure)
#define RMIAddToStructure(adder, structure, type, attribute, size) structure->attribute = (type*)adder; adder += size;
#define RMISetStructureOffset(structure, type, attribute, absoluteAddress) structure->attribute = (type*)((UINT32)(absoluteAddress) - (UINT32)(structure))
#define RMIGetOffset(structure, type, attribute) (type*)((UINT32)(structure->attribute) + (UINT32)(structure))

// more sophisticated macros
#define CreateStringFromRMIStruct(struct, name) {.length = struct->_##name.length, RMIGetOffset(struct, UChar, _##name.string)}
#define RMIGetStringOffset(struct, name) RMIGetOffset(struct, UChar, _##name.string)
#define CreateRMIStructFromStringP(adder, struct, name, original) struct->_##name.length = (original)->length; \
	RMIAddToStructure(adder, struct, UChar, _##name.string, sizeof(UChar) * (original)->length); \
	memcpy(RMIGetStringOffset(struct, name), original->string, sizeof(UChar) * (original)->length)
#define CreateRMIStructFromString(adder, struct, name, original) struct->_##name.length = (original).length; \
	RMIAddToStructure(adder, struct, UChar, _##name.string, sizeof(UChar) * (original).length); \
	memcpy(RMIGetStringOffset(struct, name), (original).string, sizeof(UChar) * (original).length)





/* special types needed */

/** Static type for thread identification - Don't mess up with PID which is dynamically set up */
typedef UINT32 RMISERIAL;
/** Pointer to a message that ca be sent between processes */
typedef UINT32* RMIMESSAGE;
/** Unique (per process) number for functions */
typedef UINT32 RMIFUNCTION;


/**
 * Possible return values for Remote Method Invokation
 */
typedef enum {
	RMI_SUCCESS = 0,		/**< everything's ok */
	RMI_EXPORT_NOT_FOUND = 1,	/**< There is no function with that number registered at the receiver's thread */
	RMI_TRANSMIT_ERROR = 2,		/**< General error concerning method invokation */
	RMI_GEN_ERROR = 3,		/**< An unknown error occured */
	RMI_WRONG_SETUP = 4,		/**< Too many functions exported for current RMISetup */
	RMI_NO_SETUP = 5,		/**< Must be initialized with RMISetup first! */
	RMI_NO_SERIAL = 6,		/**< Sender has not set up RMI! */
	RMI_INSUFF_RIGHTS = 7,		/**< Insufficient rights to send a message to that thread */
	RMI_NOT_SUPPORTED = 8,		/**< Remote thread has not set up RMI */
	RMI_OCCUPIED = 9,		/**< Exported function number already in use */
	RMI_OVERLOAD = 10,		/**< Calling remote thread would cause stack problems at remote thread */
	RMI_OUT_OF_MEMORY = 11,		/**< Out of memory */
	RMSG_INVALID_BUFFER = 12,
	RMSG_INVALID_HANDLE = 13,
	RMSG_OWNERSHIP_ERROR = 14
} RAY_RMI;

typedef enum {
	RTYPE_UINT8 = 1,
	RTYPE_SINT8 = 2,
	RTYPE_UINT16 = 3,
	RTYPE_SINT16 = 4,
	RTYPE_UINT32 = 5,
	RTYPE_SINT32 = 6,
	RTYPE_CHAR = 7,
	RTYPE_STRING = 8,
	RTYPE_NONE = 9,
	RTYPE_VALUE = 10,
	RTYPE_SAME = 11
} MSG_TYPE;



/* function prototypes */
RAY_RMI SYSTEM _RMInvoke	(RMISERIAL appSerial, RMIFUNCTION exportID, UINT32 messageSend, BOOL blocking, RMIMESSAGE *messageRecv, MSG_TYPE expectedResultType, UINT32 maxResultLength);
RAY_RMI SYSTEM _RMPassMessage	(RMISERIAL appSerial, RMIFUNCTION exportID, RMIMESSAGE messageSend, BOOL blocking, RMIMESSAGE *messageRecv, MSG_TYPE expectedResultType, UINT32 maxResultLength);
RAY_RMI SYSTEM RMISetup(RMISERIAL mySerial, UINT32 count);
RAY_RMI SYSTEM RMIRegister(RMIFUNCTION funcUID, void *entryPoint, PRIVLEVEL minPriv, BOOL partnership, MSG_TYPE expectedArgumentType, UINT32 maxArgumentLength);
volatile RAY_RMI SYSTEM AllocateMessageBuffer(UINT32 reqSize, MSG_TYPE messageType);
RAY_RMI SYSTEM FreeMessageBuffer(RMIMESSAGE messageBuffer);
void NORETURN SYSTEM CReturn(RMIMESSAGE message, UINT32 msgUID, BOOL isMemory);

#define RMIRegisterValue(funcUID, entryPoint, minPriv, partnership) RMIRegister(funcUID, entryPoint, minPriv, partnership, RTYPE_VALUE, 4)

#define RMInvoke(appSerial, exportID, messageSend) _RMInvoke(appSerial, exportID, messageSend, FALSE, NULL, RTYPE_NONE, 0)
#define RMCall(appSerial, exportID, messageSend, messageRecv, expectedResultType, maxResultLength) _RMInvoke(appSerial, exportID, messageSend, TRUE, messageRecv, expectedResultType, maxResultLength)

#define RMPassMessage(appSerial, exportID, messageSend) _RMPassMessage(appSerial, exportID, messageSend, FALSE, NULL, RTYPE_NONE, 0)
#define RMCallMessage(appSerial, exportID, messageSend, messageRecv, expectedResultType, maxResultLength) _RMPassMessage(appSerial, exportID, messageSend, TRUE, messageRecv, expectedResultType, maxResultLength)
#define RMCallMessageValue(appSerial, exportID, messageSend, messageRecv) _RMPassMessage(appSerial, exportID, messageSend, TRUE, messageRecv, RTYPE_VALUE, sizeof(UINT32))

#include <threads/threads.h>

#endif
