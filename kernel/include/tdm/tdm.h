#ifndef _RAY_TDM_H
#define _RAY_TDM_H


typedef enum {
	TDM_SUCCESS = 0,
	TDM_INSUFFICIENT_RIGHTS = 1,
	TDM_TRUSTED_ONLY = 2,
	TDM_OUT_OF_MEMORY = 3,
	TDM_PARTNERSHIP_ONLY = 4,
	TDM_ILLEGAL_ARGUMENT = 5,
	TDM_IMPOSSIBLE = 6
} RAY_TDM;

UINT32 SYSTEM GetPrivLevel(void);

#endif
