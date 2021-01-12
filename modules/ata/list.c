#include <raykernel.h>
#include <memory/memory.h>
#include <rmi/rmi.h>

#include "../fs/fsregister.h"
#include "../fs/chardev.h"

#include "drives.h"
#include "list.h"

BOOL Enqueue(LIST *list, UINT16 *buffer, PCHAR_DISK disk, UINT32 msgID) {
	ELEMENT *newEntry;
	
	newEntry = (ELEMENT*)malloc(sizeof(ELEMENT));
	
	if(!newEntry) {
		return FALSE;
	}
	
	newEntry->request = msgID;
	newEntry->disk = disk;
	newEntry->buffer = buffer;
	newEntry->next = NULL;
	
	if (list->tail) {
		list->tail->next = newEntry;
	} else {
		/* first entry */
		list->root = newEntry;
	}
	
	list->tail = newEntry;
	
	return TRUE;
};

void Done(LIST *list) {
	ELEMENT *item;
	
	item = list->root;
	list->root = list->root->next;
	
	if (!list->root) {
		list->tail = NULL;
	}
	
	free(item);
}
