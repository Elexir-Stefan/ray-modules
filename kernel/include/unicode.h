#ifndef _UNICODE_H
#define _UNICODE_H

typedef UINT16 UChar;

typedef struct {
	UINT32 length;
	UChar *string;
} TUString, *PTUString;

PTUString ConvertASCIItoTUString(char *input);
char *ConvertTUStringToASCII(PTUString unicode);

#endif
