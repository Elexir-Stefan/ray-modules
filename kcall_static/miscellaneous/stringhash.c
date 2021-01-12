#include <raykernel.h>
#include <unicode.h>
#include <miscellaneous/stringhash.h>

UINT32 SimpleHashTUString(TUString *input) {
	UINT32 lastHash = 0x9D8A7C34;
	UINT32 hash = 0x4A2E7BD9;
	UINT16 value;
	UINT32 i;
	
	for (i = 0; i < input->length; i++) {
		value = input->string[i];
		hash = (hash << 1) | ((hash>>31) & 1);
		hash += (hash & 0xFFFF) * value + (value & (hash & 0xFFFF));
		hash ^= lastHash;
		lastHash = hash;
	}
	
	return hash;
	
}

UINT32 SimpleHash(char *input) {
	UINT32 lastHash = 0x9D8A7C34;
	UINT32 hash = 0x4A2E7BD9;
	UINT16 value;
	
	while(*input) {
		value = (UINT16)*input++;
		hash = (hash << 1) | ((hash>>31) & 1);
		hash += (hash & 0xFFFF) * value + (value & (hash & 0xFFFF));
		hash ^= lastHash;
		lastHash = hash;
	}
	
	return hash;
	
}
