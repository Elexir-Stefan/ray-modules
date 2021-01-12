#include <raykernel.h>
#include <rmi/rmi.h>
#include <pid/pids.h>
#include <string.h>
#include <drivers/dmesg/dmesg.h>

#include "../../modules/dmesg/exports.h"

void dmesg(char *message) {
	RMIMESSAGE log;
	UINT32 len;
	
	len = strlen(message);
	if (!len) return;	/* nothing to log */
	
	log = (RMIMESSAGE)AllocateMessageBuffer(len + 1, RTYPE_STRING);
	if (!log) return;	/* out of memory */
	memcpy(log, message, len + 1);
	RMPassMessage(DRV_DMESG, EXPORT_ADDMESSAGE, log);
}
