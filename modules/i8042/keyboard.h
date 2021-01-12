#ifndef _KEYBOARD_H
#define _KEYBOARD_H

typedef struct _threadList{
	RMISERIAL process;
	RMIFUNCTION funcExport;
	struct _threadList *next;
} THREADLIST;

#endif
