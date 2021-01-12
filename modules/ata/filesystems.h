#include "../fs/fsregister.h"
#include "../fs/exports.h"

CALLBACK RegisterFileSystem(RMISERIAL sender, FS_REGISTER *fsData, UINT32 msgID);
CALLBACK GetClaimedDrivers(RMISERIAL sender, UINT32 dummy, UINT32 msgID);
CALLBACK MountPartition(RMISERIAL sender, MOUNT toMount, UINT32 msgID);
