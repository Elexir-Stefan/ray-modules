#ifndef _STRING_H
#define _STRING_H


/**
 * copies count bytes from src to dest
 * @param dest Pointer to where data will be copied to
 * @param src Pointer from where data will be copied
 * @param count Amount of data to copy in bytes
 * @return dest
 */
POINTER memcpy(POINTER dest, const POINTER src, UINT32 count);

/**
 * overwrites count bytes with c at position dest
 * @param dest a pointer to the beginning of the erasure
 * @param c the byte that will be written at every position
 * @param count length of the overwriting
 * @return dest
 */
POINTER memset(POINTER dest, UINT8 c, UINT32 count);

/**
 * overwrites count words with c at position dest
 * @param dest a pointer to the beginning of the erasure
 * @param c the word that will be written at every position
 * @param count length of the overwriting
 * @return dest
 */
POINTER memsetw(POINTER dest, UINT16 c, UINT32 count);

/**
 * Moves count bytes from src to dst
 * @param dst Pointer to where data will be moved to
 * @param src Pointer from where data will be moved
 * @param count amount of bytes to move
 * @return dest
 */
POINTER memmove(POINTER dst, const POINTER src, UINT32 count);

/**
* copies a string from dest to src
* @param dest address to write to
* @param src address to read from
* @return destination address
*/
char *strcpy(volatile char *dest,volatile const char *src);

/**
 * Appends string2 to the end of string1
 * @param string1 String wich will be expanded by string2
 * @param string2 String to concatenate
 */
char *strcat(volatile char *string1, volatile char *string2);

/**
 * copies n bytes from src to dest
 * @param dest address to write to
 * @param src address to read from
 * @param num number of bytes to copy
 * @return destination address
 */
char *strncpy(volatile char *dest, volatile const char *src, UINT32 num);

/**
 * compares two string and returns matching
 * @param string1 a string
 * @param string2 another string
 * @return 0 if equal, non-0 else
 */
int strcmp(volatile const char *string1, volatile const char *string2);

/**
 * compares two string of maximum length n and returns matching
 * @param string1 a string
 * @param string2 another string
 * @param n maximum length of strings to compare
 * @return 0 if equal, non-0 else
 */
int strncmp(volatile const char *string1,volatile  const char *string2, UINT32 n);

/**
 * returns the length of a string (first occurence of '\0')
 * @param string string to check
 * @return length of the string
 */
int strlen(volatile const char *string);

/**
 * returns the length of a string (first occurence of '\0')
 * But terminates if not found after maxLength
 * @param string string to check
 * @param maxLength maximum length of string. When not found till this position
 * search will abort
 * @return length of the string
 */
UINT32 strnlen(volatile char *string, UINT32 maxLength);

/**
 * searches for search in string and returns a pointer pointing to
 * the start of search
 * @param string String to search in
 * @param search What to search
 * @return Pointer to start of 1st occurence of search in string
 */
char *strchr(volatile char *string, char search);

/**
 * searches for search in string and returns a pointer pointing to
 * the start of search
 * @param string String to search in
 * @param length maximum length of string
 * @param search What to find
 * @return Pointer to start of 1st occurence of search in string
 */
char *strnchr(volatile char *string, UINT32 length, char search);

/**
 * searches for search in string and returns postion of 1st
 * occurence if found, -1 otherwise
 * @param string Strint to search in
 * @param search What to find
 */
SINT32 strpos(volatile char *string, char search);

/**
 * Searches for search in string and returns position of 1st
 * occurence (if it's not > length) -1 otherwise
 * @param string Strint to search in
 * @param length maximum length of string
 * @param search What to find
 */
SINT32 strnpos(volatile char *string, UINT32 length, char search);

#endif
