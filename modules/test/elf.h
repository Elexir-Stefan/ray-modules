BOOL ELF32LoadFile(UINT8 *image, UINT32 length, char *procName, char *procArguments);

/**
 * Same as ELF32LoadFile, but returns exact TSC of process creation time, 0 otherwise
*/
UINT32 ELF32LoadFile2(UINT8 *image, UINT32 length, char *procName, char *procArguments);