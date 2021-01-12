typedef struct {
	UINT8	bootable;
	
	UINT8	cylinder;
	UINT8	head;
	UINT8	sector;
	
	UINT8	type;
	
	UINT8	cylinder2;
	UINT8	head2;
	UINT8	sector2;
	
	UINT32 sectorStart;
	UINT32 partitionSize;
} PARTITION;

typedef struct {
	PARTITION partition[4];
} MBR;

typedef struct {
	BOOL bootable;
	UINT8 systemID;
	UINT64 partitionStart;
	UINT64 partitionSize;
	char identifyer[32];
} PARTITION_INFO;
