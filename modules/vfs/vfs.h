#ifndef _VFS_H
#define _VFS_H

/**
 * @defgroup FS Filesystem
 * Functions usefull for filesystems to register a node in the VFS tree
 */

/**
 * @defgroup USER Userspace
 * Functions and structures usefull for common usermode programs to
 * access the VFS and get information
 */

/**
 * @defgroup COMMON Filesystem & Userspace
 * Usefull for both - FS drivers and usermode programs
 */

/**
 * Attributes for specific groups
 */

#include "vfstypes.h"
#include "vfsstructure.h"

#define HANDLE_PFS	1
#define HANDLE_FILE	2

typedef struct _HANDLEINFO{
	UINT16 attributes;
	VFSnode* dir;
	VFS_ACCESS mode;
} *HANDLEINFO;

#endif
