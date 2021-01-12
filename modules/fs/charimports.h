#include "fsregister.h"

CALLBACK GetClaimedDrivers(RMISERIAL sender, UINT32 dummy, UINT32 msgID);
CALLBACK MountPartition(RMISERIAL sender, INIT_MOUNT* toMount, UINT32 msgID);
CALLBACK RegisterFileSystem(RMISERIAL sender, FS_REGISTER *fsData, UINT32 msgID);
