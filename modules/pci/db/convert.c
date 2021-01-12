#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DB_FILE		"pci.ids"
#define HEADER_FILE	"vendors.h"
#define BUFFER_SIZE	1024

#define HEADER_PROLOGUE "#define\tPCI_DEVTABLE_LEN\t(sizeof(PciDevTable)/sizeof(PCI_DEVTABLE))\n\
typedef struct _PCI_DEVTABLE\n\
{\n\
\tunsigned short	VenID;\n\
\tunsigned short	DevID;\n\
\tchar *	CardName;\n\
} PCI_DEVTABLE, *PPCI_DEVTABLE;\n\
\n\
PCI_DEVTABLE	PciDevTable [] =\n\
{\n\
"
#define HEADER_EPILOGUE "} ;\n"

void ConvertDB(FILE* db, FILE* header) {
	char *buffer;
	char *outBuffer;
	char *vendorString;
	
	char vendorID[5];
	char deviceID[5];
	
	buffer = malloc(BUFFER_SIZE);
	outBuffer = malloc(BUFFER_SIZE);
	vendorString = malloc(64);
	
	if(!buffer || !outBuffer || !vendorString) {
		printf ("Out of memory!. Aborting!\n");
		return;
	}
	
	/* terminate vendor and device id */
	vendorID[4] = 0;
	deviceID[4] = 0;
	
	while(fgets(buffer, BUFFER_SIZE, db)) {
		switch(buffer[0]) {
		case '#': /* comment */
			break;
		case '\t': /* entry */
			
			/* device or subsystem */
			if (buffer[1] != '\t') {
				
				if (strncmp(deviceID, buffer + 1, 4) == 0) {
					/* the same as the last one */
					break;
				}
				
				memcpy(deviceID, buffer+1, 4);
				
				/* eleminate newline char */
				buffer[strlen(buffer) - 1] = 0;
				/* write outfile */
				sprintf(outBuffer, "{0x%s, 0x%s, \"%s - %s\"},\n", vendorID, deviceID, vendorString, buffer + 7);
				fputs(outBuffer, header);
			} /* else subsystem */
			break;
		case 'C':
			goto skipClasses;
			/* class not supported yet */
			break;
		default:
			/* vendor id */
			memcpy(vendorID, buffer, 4);
			strcpy(vendorString, buffer + 6);
			vendorString[strlen(vendorString) -1] = 0;
		}
	}
	
skipClasses:
	free(buffer);
	free(outBuffer);
}

int main(int argc, char **argv) {

	FILE* pciDB;
	FILE* header;
	
	pciDB = fopen(DB_FILE, "r");
	header = fopen(HEADER_FILE, "w+");
	
	if (pciDB && header) {
		
		fputs(HEADER_PROLOGUE, header);

		ConvertDB(pciDB, header);
		
		fputs(HEADER_EPILOGUE, header);
		
		fclose(pciDB);
		fclose(header);
	} else {
		if (!pciDB) {
			printf ("Error reading pci-database file '%s'.\n", DB_FILE);
		}
		if (!header) {
			printf ("Error opening file '%s' for writing.\n", HEADER_FILE);
		}
		exit(1);
	}
	
	
}
