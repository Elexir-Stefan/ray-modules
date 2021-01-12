#include <raykernel.h>
#include <threads/sleep.h>
#include <memory/memory.h>
#include <rmi/rmi.h>
#include <pid/pids.h>
#include <string.h>

#include "exports.h"
#include "dmesgerr.h"

#define MAX_MESSAGE_LENGTH	512

/* currently 4k buffer */
#define DMESG_BUFFER_LENGTH	4096

unsigned char *diagnosticMessages;
UINT32 diagnosticLength = 0;

CALLBACK AddMessageToBuffer(RMISERIAL sender, RMIMESSAGE log, UINT32 msgID) {
	SINT32 length;
	UINT32 overSize;
	unsigned char *message = (unsigned char*)log;
	
	length = strnlen((char*)message, MAX_MESSAGE_LENGTH);
	if (length == -1) {
		length = MAX_MESSAGE_LENGTH - 1;
	}
	message[length] = '\n';	/* messages are devided by a new line */
	length++;	/* new-line-character */
	
	if (diagnosticLength + length < DMESG_BUFFER_LENGTH) {
		/* current message fits in buffer */
		memcpy(diagnosticMessages + diagnosticLength, message, length);
		diagnosticLength += length;
		diagnosticMessages[diagnosticLength] = '\0';	/* terminate buffer */
	} else {
		/* message won't fit, delete old messages */
		overSize = diagnosticLength + length - DMESG_BUFFER_LENGTH + 1;	/* plus null-byte */
		
		/* move old messages out */
		memcpy(diagnosticMessages, diagnosticMessages + overSize, diagnosticLength - overSize);
		memcpy(diagnosticMessages + diagnosticLength - overSize, message, length);
		diagnosticLength = DMESG_BUFFER_LENGTH  -1;
		diagnosticMessages[diagnosticLength] = '\0';
	}
	
	FreeMessageBuffer(log);
	ThreadExit(0);
}

CALLBACK GetMessageBuffer(RMISERIAL sender, UINT32 reserved, UINT32 msgID) {
	RMIMESSAGE packet;
	
	packet = (RMIMESSAGE)AllocateMessageBuffer(diagnosticLength + 1, RTYPE_STRING);	/* plus null-char */
	if (!packet) {
		CReturn((RMIMESSAGE)DMESG_ERR_OUT_OF_MEMORY, msgID, FALSE);
	}
	
	memcpy(packet, diagnosticMessages, diagnosticLength + 1);
	
	CReturn(packet, msgID, TRUE);
	ThreadExit(0);
}

RAYENTRY KernelModuleEntry(char *arguments) {
	
	diagnosticMessages = (unsigned char*)malloc(DMESG_BUFFER_LENGTH);
	if (!diagnosticMessages) {
		exit(DMESG_ERR_OUT_OF_MEMORY);
	}
	
	RMISetup(DRV_DMESG, 2);
	
	RMIRegister(EXPORT_ADDMESSAGE, AddMessageToBuffer, 255, FALSE, RTYPE_STRING, 4096);
	RMIRegisterValue(EXPORT_GETBUFFER, GetMessageBuffer, 255, FALSE);
	
	for(;;) {
		Sleep();
	}
}
