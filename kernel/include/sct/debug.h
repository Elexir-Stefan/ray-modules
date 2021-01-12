#ifndef _SCT_DEBUG_H
#define _SCT_DEBUG_H

#define SYS_PROFILE_FLUSH	504
#define SYS_PROFILE_SETUP	505
#define SYS_PROFILE_START	506
#define SYS_PROFILE_STOP	507
#define SYS_PROFILE_RESET	508
#define SYS_PROFILE_SIZE	509
#define SYS_PROFILE_RCOUNT	510
#define SYS_BREAKPOINT		511


#else
    #error "sct/debug.h included more than once!"
#endif
