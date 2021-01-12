#include <raykernel.h>
#include <kdisplay/kprintf.h>
#include <threads/sleep.h>
#include <rmi/rmi.h>
#include <string.h>

#include <pid/pids.h>

#define READTSC(var) asm volatile ("rdtsc":"=a"(var)::"edx")


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

CALLBACK Benchmark(RMISERIAL sender, UINT32 value, UINT32 msgID) {
	volatile UINT32 start, stop;
	UINT32 counter;
	UINT32 result;
	
	// per message
	UINT32 msgStart;
	
	/* save start time */
	READTSC(start);
	for (counter = 0; counter < 200; counter++) {
		// Read Time Stamp Counter and pass it to helper process.
		// The other process will then calculate the sum of the time
		// differences and save it for later retrieval.
		READTSC(msgStart);
		RMInvoke(0xDEADCAFE, 0, msgStart);
		Pause(50);
	}
	READTSC(stop);
	
	
	if (stop > start) {
		result = stop - start;
	} else {
		result = 0xffffffff - start + stop;
	}
	
	char totalTime[20];
	char avgTime[20];
	volatile char* test = (char*)AllocateMessageBuffer(128, RTYPE_STRING);
	
	KPrintf ("Message handle: %x (%s)\n", test, test);
	// Now we retrieve the average time difference
	RMCallMessage(0xDEADCAFE, 1, (RMIMESSAGE)test, (RMIMESSAGE*)&test, RTYPE_STRING, 128);
	KPrintf ("Message handle: %x (%s)\n", test, test);
	strcpy(avgTime, test);
	KPrintf ("Message handle: %x (%s)\n", test, test);
	itoa(result, totalTime, 10);
	KPrintf ("Message handle: %x (%s)\n", test, test);
	/*strcpy(test, "Total (2000 msg): ");
	strcat(test, totalTime);
	strcat(test, ", average: ");
	strcat(test, avgTime);
*/
	
	
	CReturn((RMIMESSAGE)test, msgID, TRUE);
}

RAYENTRY KernelModuleEntry(void) {
	
	RMISetup(0xDEADBEEF, 1);
	
	RMIRegisterValue(0, Benchmark, 255, FALSE);
	
	for(;;) {
		Sleep();
	}
}
