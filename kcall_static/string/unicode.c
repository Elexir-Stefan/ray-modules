#include <raykernel.h>
#include <unicode.h>
#include <memory/memory.h>
#include <string.h>

PTUString ConvertASCIItoTUString(char *input) {
	UINT32 length, i;
	PTUString limitedUnicodeString;
	UChar *string;
	
	length = strlen(input);
	
	limitedUnicodeString = (PTUString)malloc(sizeof(TUString) + length * sizeof(UChar));
	string = (UChar*)(limitedUnicodeString + 1);
	
	limitedUnicodeString->string = string;
	limitedUnicodeString->length = length;
	
	for (i = 0; i < length; i++) {
		string[i] = input[i];
	}
	
	return limitedUnicodeString;
}

char *ConvertTUStringToASCII(PTUString unicode) {
	char *ascii;
	UINT32 i;
	
	ascii = (char*)malloc(unicode->length + 1);
	
	for (i = 0; i < unicode->length; i++) {
		ascii[i] = (unicode->string[i] & 0xFF);
	}
	ascii[unicode->length] = '\0';
	
	return ascii;
}
