#ifndef _RAY_TYPEDEFS_H
#define _RAY_TYPEDEFS_H

/* types named by occupied length in bits */
typedef unsigned int UINT32;
typedef unsigned short UINT16;
typedef unsigned char UINT8;
typedef signed int SINT32;
typedef signed short SINT16;
typedef signed char SINT8;
typedef unsigned long long UINT64;
typedef signed long long SINT64;

typedef enum {
	FALSE = 0,
	TRUE = 1
} __attribute__((packed)) BOOL;

/* generic pointer type */
typedef volatile void* POINTER;

typedef char* String;
typedef const char* CString;

/* the other zero */
#define NULL (void*)0

#endif
