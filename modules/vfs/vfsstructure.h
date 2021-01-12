#ifndef _VFS_STRUCTURE_H
#define _VFS_STRUCTURE_H
/**
 * @file vfsstructure.h
 * @brief Structures where vfs information is stored
 */

#include "unicode.h"

/**
 * a node structure
 */
typedef struct _VFSnode {
	TUString name;
	TUString addInfoName;
	RMISERIAL registeredDriver;
	UINT32 mountID;
	UINT32 pathHash;
	struct _VFSnode *parent;

	struct _VFSnode *root;
	struct _VFSnode *tail;

	struct _VFSnode *prev;
	struct _VFSnode *next;
} VFSnode;


#endif
