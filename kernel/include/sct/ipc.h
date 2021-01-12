#ifndef _SCT_IPC_H
#define _SCT_IPC_H

#define SYS_RM_INVOKE		109
#define SYS_RMI_SETUP		110
#define SYS_RMI_REGISTER	111
#define SYS_ALLOC_MB		112
#define SYS_FREE_MB		113
#define SYS_RETURN		114
#define SYS_RM_PASS_MSG		115

#else
    #error "sct/ipc.h included more than once!"
#endif
