#ifndef _RAY_KERNEL_H
#define _RAY_KERNEL_H

/**
 * include other files 
 */
#include <ray/typedefs.h>
#include <standard.h>

/**
 * optimization hints 
 */
#define RAYENTRY void __attribute__ ((noreturn))
#define SYSTEM __attribute__ ((noinline))
#define INLINE __attribute__ ((always_inline))

#endif
