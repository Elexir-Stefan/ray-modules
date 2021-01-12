#include <raykernel.h>
#include <string.h>
#include <unicode.h>
#include <memory/memory.h>
#include <rmi/rmi.h>

#include "vfstree.h"

static BOOL DiffPrefix(UINT16 *string1, UINT16 *string2, UINT32 len);

VFSnode *root;

void CreateRootNode() {
	root = malloc(sizeof(VFSnode));
	memset(root, 0, sizeof(VFSnode));

	root->parent = root;
	root->name = *ConvertASCIItoTUString("");
}

VFSnode* GetRootNode() {
	return root;
}

VFSnode* GetNodeByPath(VFSnode* dir, TUString path) {
	UINT32 currNameLength;

	while(dir) {
		/* compare names */
		currNameLength = dir->name.length;
		if (path.length >= currNameLength) {
			if (DiffPrefix(dir->name.string, path.string, currNameLength)) {
				dir = dir->next;
				continue;
			} else {
				/* same prefix */
				if (path.length == currNameLength) {
					/* same name */
					return dir;
				} else {
					/* path is longer */
					if (path.string[currNameLength] == DIR) {
						/* directory match */
						currNameLength++;	/* minus "/" */
						path.length -= currNameLength;
						path.string += currNameLength;
						if (path.length) {
							return GetNodeByPath(dir->root, path);
						} else {
							return dir;
						}
					} else {
						dir = dir->next;
						continue;
					}
				}
			}
		} else {
			/* cannot fit */
			dir = dir->next;
			continue;
		}
	}
	return NULL;

}

static BOOL DiffPrefix(UINT16 *string1, UINT16 *string2, UINT32 len) {
	UINT32 i;
	
	for (i = 0; i < len; i++) {
		if (string1[i] != string2[i]) {
			return TRUE;
		}
	}
	return FALSE;
}

VFSnode* GetFSNode(VFSnode* dir, PTUString path) {
	UINT32 currNameLength;

	while(dir) {
		/* compare names */
		currNameLength = dir->name.length;
		if (path->length >= currNameLength) {

			if (DiffPrefix(dir->name.string, path->string, currNameLength)) {
				dir = dir->next;
				continue;
			} else {
				/* same prefix */
				if (path->length == currNameLength) {
					if (dir->registeredDriver) {
						path->length = 0;
						return dir;
					} else {
						return NULL;
					}
				} else {
					/* path is longer */
					if (path->string[currNameLength] == DIR) {
						/* directory match */
						currNameLength++;	/* minus "/" */
						path->length -= currNameLength;
						path->string += currNameLength;
						if (dir->registeredDriver) {
							return dir;
						} else {
							return GetFSNode(dir->root, path);
						}
					} else {
						dir = dir->next;
						continue;
					}
				}
			}
		} else {
			/* cannot fit */
			dir = dir->next;
			continue;
		}
	}
	return NULL;

}

void AddToDir(VFSnode *toAdd, VFSnode *directory) {
	VFSnode *endOfDirList = directory->tail;

	/* add to empty or filled directory? */
	if (endOfDirList) {
		/* not empty - get last entry */

		/* update tail pointer of parent */
		directory->tail = toAdd;

		/* chain each other */
		endOfDirList->next = toAdd;
		toAdd->prev = endOfDirList;
	} else {
		/* empty one */
		directory->root = toAdd;
		directory->tail = toAdd;

		toAdd->prev = NULL;
	}
	toAdd->next = NULL;
	toAdd->parent = directory;
}

void DelFromDir(VFSnode *toDel) {
	VFSnode *parent = toDel->parent;

	if (parent) {

		/* first element? */
		if (toDel == parent->root) {
			parent->root = toDel->next;
		}  else {
			/* if not first - predecessor exists */
			toDel->prev->next = toDel->next;
		}
		/* last element? */
		if (toDel == parent->tail) {
			parent->tail = toDel->prev;
		} else {
			/* if not last - successor exists */
			toDel->next->prev = toDel->prev;
		}

	}
}

void MoveNode(VFSnode *node, VFSnode *to) {
	DelFromDir(node);
	AddToDir(node, to);
}
