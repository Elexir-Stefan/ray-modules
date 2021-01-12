#include <raykernel.h>

#define MAX_TRIES 10
#define FAST_START 550

UINT32 MathSquareRoot(UINT32 value) {
	UINT8 tries;
	UINT32 sqr_est = FAST_START;
	
	for (tries = 0; tries < MAX_TRIES; tries++) {
		sqr_est = (sqr_est + value/sqr_est)>>1;
	}
	return sqr_est;
}
