typedef struct _OPENFILE{
	MOUNT_STORE* store;
	FAT16ENTRY file;
	UINT32 sector;
	UINT64 currCluster;
	UINT32 sectorClusterIndex;
	UINT32 currIndex;
	UINT64 bufferPositionInFile;
	UINT64 bufferLength;
	unsigned char *clusterBuffer;
} *OPENFILE;

UINT32 ParseHexNumber (char *commandline);
unsigned char LFNchecksum(const unsigned char *fileName);
void TerminatedDOSName(unsigned char *fileName, unsigned char *extension, unsigned char *completeName);
PCHAR_DISK CreateReadCommand(UINT64 sector, UINT32 count, MOUNT* mount);
void TerminatedUnicodeDOSName(unsigned char *fileName, unsigned char *extension, PTUString completeName);
BOOL DifferentPrefix(PTUString first, PTUString second);
FAT_FILE FindObject(PTUString fileName, UINT64 rootEntry, MOUNT_STORE* store);
BOOL ReadFile(OPENFILE file, FILESTREAM stream, unsigned char *content);
