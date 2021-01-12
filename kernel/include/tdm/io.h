#ifndef _RAY_IO_H
#define _RAY_IO_H

#include <tdm/tdm.h>

RAY_TDM SYSTEM RequestIOPort(UINT16 port);

static __inline__ void OutPortB(UINT16 port, UINT8 val)
{
	asm volatile("outb %0,%1"::"a"(val), "Nd" (port));
}

static __inline__ void OutPortW(UINT16 port, UINT16 val)
{
	asm volatile("outw %0,%1"::"a"(val), "Nd" (port));
}

static __inline__ void OutPortL(UINT16 port, UINT32 val)
{
	asm volatile("outl %0,%1"::"a"(val), "Nd" (port));
}

static __inline__ UINT8 InPortB(UINT16 port)
{
	UINT8 ret;
	asm volatile ("inb %1,%0":"=a"(ret):"Nd"(port));
	return ret;
}

static __inline__ UINT16 InPortW(UINT16 port)
{
	UINT16 ret;
	asm volatile ("inw %1,%0":"=a"(ret):"Nd"(port));
	return ret;
}

static __inline__ UINT32 InPortL(UINT16 port)
{
	UINT32 ret;
	asm volatile ("inl %1,%0":"=a"(ret):"Nd"(port));
	return ret;
}


#endif
