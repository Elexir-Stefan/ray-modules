#ifndef _THREAD_TYPES_H
#define _THREAD_TYPES_H

typedef UINT8 PRIVLEVEL;
typedef UINT8 PRIORITY;
typedef UINT32 PID;
typedef UINT32 TID;

typedef struct {
	PID pid;
	TID tid;
	BOOL isProcessParent;
	UINT32 handle;
} PID_HANDLE;

#define PID_DOES_NOT_EXIST	0

#endif
