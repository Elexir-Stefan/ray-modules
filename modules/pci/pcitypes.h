#define MAX_PCI_DEVS	64

#define PCIREG_BASE	0x10
#define PCIREG_SIZE	4

typedef struct {
	UINT16 vendorID;
	UINT16 deviceID;
	UINT16 command;
	UINT16 status;
	UINT32 revisionClassCode;
	
	UINT8 cacheLineSize;
	UINT8 latencyTimer;
	UINT8 headerType;
	UINT8 BIST;
	
	UINT32 baseAddressReg[6];
	
	UINT32 cardBusCISPointer;
	
	UINT16 subSystemVendorID;
	UINT16 subSystemID;
	
	UINT32 expansionROMBaseAddress;
	UINT8 capListPointer;
	
	char reseved[7];
	
	UINT8 interruptLine;
	UINT8 interruptPin;
	UINT8 minGrant;
	UINT8 maxLatency;
} __attribute__((packed)) PCI_COMMON_HEADER;

typedef struct {
	
} PCI_HEADER_TYPE1;

typedef struct {
	
} PCI_HEADER_TYPE2;


/* PCI IO specific */
typedef struct {
	UINT32 IOStart;
	UINT32 IOLength;
} PCI_IO_RANGE;

typedef struct {
	UINT32 MemoryStart;
	UINT32 MemoryLength;
	enum {
		PCIMemTypeStandard = 0,
		PCIMemTypePrefetchable = 1
	} MemoryType;
} PCI_MEM_RANGE;

typedef struct {
	enum {
		PCIResourceUnused = 0,
		PCIResourceIsIO = 1,
		PCIResourceIsMemory = 2
	} __attribute__((packed)) IOType;
	
	union {
		PCI_IO_RANGE rangeIO;
		PCI_MEM_RANGE rangeMem;
	} range;
} PCI_RESOURCES;


typedef struct _PCIDevice {
	UINT8 bus;
	UINT8 device;
	UINT8 function;
	
	PCI_RESOURCES resource[6];
	
	union {
		PCI_COMMON_HEADER PCIHeader;
		UINT8 configBytes[64];
		UINT16 configWords[32];
		UINT32 configDWords[16];
	} configSpace;
	
	union {
		PCI_HEADER_TYPE1 hdrType1;
		PCI_HEADER_TYPE2 hdrType2;
	} extendedHeader;
	
	struct _PCIDevice *next;
} PCIDevice;

typedef struct {
	PCIDevice *root;
	PCIDevice *tail;
	UINT32 numPCIDevices;
} PCIDeviceList;

typedef struct {
	PCIDevice *list;
	UINT32 numPCIDevices;
} PCIDeviceArray;
