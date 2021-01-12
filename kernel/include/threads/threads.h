#ifndef _RAY_THREADS_H
#define _RAY_THREADS_H

#include <ray/arguments.h>
#include <rmi/rmi.h>
#include "thread_types.h"

/**
 * @brief Mutex type tu use when code is critical and not re-entrant-save
 * @note Naming convention for mutex-variables: M_UnderscoreFreeUppercaseLetterStartingVariable
 */
typedef unsigned long MUTEX;
typedef UINT32 IPLOCK;

#define THREAD_IDENT_LENGTH	64

typedef enum {
	SEMAPHORE_IN_USE = 0,
	SEMAPHORE_ILLEGAL = 1,
	SEMAPHORE_MISUSE = 2,
	SEMAPHORE_DENIED = 3,
	SEMAPHORE_READY = 4
} SEMAPHORE_STATUS;

typedef enum {
	T_RUNNING = 0,
	T_IDLE = 1,
	T_WAITING = 2,
	T_MSG_RECV = 3,
	T_EXITING = 5
} __attribute__((packed)) THREAD_STATE;

typedef struct {
	UINT32 pid;
	RMISERIAL serial;
	UINT32 numExports;

	UINT32 usedMemory;
	UINT32 usedPages;
	UINT32 memAllocs;
	UINT32 lastFitPointerCode;
	UINT32 lastFitPointerData;

	UINT64 contextSwitches;
	UINT64 cpuCycles;
	UINT64 privCycles;
	THREAD_STATE state;

	UINT32 threadNum;
	BOOL isParent;
	char ident[THREAD_IDENT_LENGTH];
} THREAD_INFO;

typedef struct {
	UINT32 numThreads;
	UINT32 kernelMemory;
	UINT32 memUsageTotal;
	UINT32 memFree;
	THREAD_INFO *thread;
} THREAD_INFO_LIST;


#define RAY_PRIO_COMA		-128
#define RAY_PRIO_VERYLOW	-64
#define RAY_PRIO_LOW		-32
#define RAY_PRIO_LOWER		-10
#define RAY_PRIO_NORMAL		 0
#define RAY_PRIO_HIGHER		 10
#define RAY_PRIO_HIGH		 32
#define RAY_PRIO_VERYHIGH	 64
#define RAY_PRIO_REALTIME	 127

#define EXIT_CODE_ABORT		-32760
#define EXIT_CODE_CHILD		-32761
#define EXIT_CODE_RMIRESULT	-32762
#define EXIT_CODE_NO_THREAD	-32763

#define THREAD_INFO_ALL		0

BOOL ThreadCreateReturn(POINTER entryPoint, POINTER returnAddress, PRIORITY prio, ARGUMENTS *args, String threadName, PID_HANDLE* handle);
BOOL ThreadCreate(POINTER entryPoint, PRIORITY prio, ARGUMENTS *args, String threadName, PID_HANDLE* handle);

void ThreadExit(UINT32 exitCode);
BOOL ThreadAbort(PID_HANDLE* handle);
BOOL ThreadNotify(PID_HANDLE* handle);
void ThreadGet(PID_HANDLE* handle);
SINT32 ThreadJoin(PID_HANDLE* handle);
BOOL IsThreadAlive(PID_HANDLE* handle);


IPLOCK SemaphoreCreate(CString lockName, SINT32 initialValue, BOOL othersAllowed);
IPLOCK SemaphoreGet(CString lockName);
SEMAPHORE_STATUS SemaphoreDestroy(IPLOCK lockUID);
SEMAPHORE_STATUS SemaphoreEnter(IPLOCK lockUID);
SEMAPHORE_STATUS SemaphoreLeave(UINT32 lockUID);
SEMAPHORE_STATUS SemaphoreStatus(IPLOCK lockUID);

BOOL BarrierCreate(CString barrierName, BOOL othersAllowed);
BOOL BarrierArrive(CString barrierName);
BOOL BarrierGo(CString barrierName, BOOL wakeForeignProcesses);
BOOL BarrierClose(CString barrierName);

#define MutexCreate(lockName, othersAllowed) SemaphoreCreate(lockName, 1, othersAllowed)

UINT32 GetPID();

/**
 * Get process/thread information
 * @param pid of the process you want to get information of, 0 for all processes
 * @return list of process information
 */
THREAD_INFO_LIST* GetThreadInfo(UINT32 pid);

#endif
