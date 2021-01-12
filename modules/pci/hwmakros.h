/* makros */
/* concerning hardware */
#define GET_VENDOR(x) (x & 0xFFFF)
#define GET_DEVICE(x) (x >> 16)

#define INVALID(VenDev) ((GET_VENDOR(VenDev) == 0xFFFF) || (GET_VENDOR(VenDev) == 0x0000))


/* bit manipulation */
#define BITSET(integer, bitNum) (integer & (1 << bitNum))

/* constants */
#define REG_VENDOR	0x00
#define REG_DEVICE	0x04
#define REG_HEADER	0x0E
#define REG_SUBCLASS	0x0A
#define REG_CLASS	0x0B
#define REG_IRQ	0x3C
#define REG_BASEADDR	0x30

#define BIT_MULTI	7
