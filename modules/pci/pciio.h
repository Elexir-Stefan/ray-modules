#define	CONFIG1_ADR	0xCF8
#define	CONFIG1_DATA	0xCFC

#define PCI_DIRECT_ACC	0x80000000L

UINT32 PCIDirectDetect(void);
PCIDeviceArray *PCIScanBuses();
