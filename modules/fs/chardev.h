#ifndef _CHAR_DEVICE_H
#define _CHAR_DEVICE_H

typedef struct {
	MOUNT mount;
	UINT64 sector;
	UINT32 count;
} __attribute__((packed)) CHAR_DISK, *PCHAR_DISK;

#define _CharBarrierName(num) "CharDev:" # num
#define CreateCharBarrier(num) BarrierCreate(_CharBarrierName(num), FALSE);
#define FireCharBarrier(num) BarrierGo(_CharBarrierName(num), TRUE);

#endif
