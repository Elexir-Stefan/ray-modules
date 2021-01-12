#ifndef _RAY_DEBUG_H
#define _RAY_DEBUG_H

void _BREAKPOINT();

void ProfilingEnable(UINT32 entries);
void ProfilingStart();
void ProfilingStop();
void ProfilingReset();
void ProfilingFlush();

UINT32 ProfilingGetSize();
UINT32 ProfilingGetUsed();

#endif
