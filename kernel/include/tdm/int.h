#ifndef _RAY_INT_H
#define _RAY_INT_H

#include <tdm/tdm.h>

RAY_TDM SYSTEM RegisterIRQ(UINT32 irqNum, void *functionAddress);
void SYSTEM InterruptDone(UINT32 irqNum);

#endif
