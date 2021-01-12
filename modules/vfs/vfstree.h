#include "vfsstructure.h"

#define DIR '/'

void CreateRootNode();
VFSnode* GetRootNode();
VFSnode* GetNodeByPath(VFSnode* dir, TUString path);
VFSnode* GetFSNode(VFSnode* dir, PTUString path);

void AddToDir(VFSnode *toAdd, VFSnode *directory);
void DelFromDir(VFSnode *toDel);
void GetNodePath(VFSnode *node);
void PrintFileSystem();
void MoveNode(VFSnode *node, VFSnode *to);
