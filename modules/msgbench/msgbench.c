#include <raykernel.h>
#include <kdisplay/kprintf.h>
#include <threads/sleep.h>
#include <rmi/rmi.h>
#include <string.h>
#include <pid/pids.h>

#define READTSC(var) __asm__ __volatile__("rdtsc":"=a"(var)::"edx")

volatile UINT64 sum = 0L;
volatile UINT32 count = 0;

void itoa(UINT32 number, char *string, UINT32 radix) {
	UINT32 pos = 0, check = number;
	
	while (check) {
		check /= radix;
		pos++;
	}
	
	string[pos] = 0;
	while (number) {
		string[--pos] = (number % radix) + '0';
		number /= radix;
	}
}


CALLBACK GetResult(RMISERIAL sender, RMIMESSAGE message, UINT32 msgID) {
	
	UINT32 result = sum / count;
	itoa(result, (char*)message, 10);
	
	CReturn(message, msgID, TRUE);
}

CALLBACK BenchMessage(RMISERIAL sender, RMIMESSAGE message, UINT32 msgID) {
	UINT32 stop;
	READTSC(stop);
	
	UINT32 diff = stop - (UINT32)message;
	
	//KPrintf("%u\t", diff);
	
	sum += diff;
	count++;
	ThreadExit(0);
}

RAYENTRY KernelModuleEntry(void) {
	
	RMISetup(0xDEADCAFE, 2);
	
	RMIRegisterValue(0, BenchMessage, 255, FALSE);
	RMIRegister(1, GetResult, 255, FALSE, RTYPE_STRING, 128);
	
	for(;;) {
		Sleep();
	}
}
