#pragma once
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

typedef void* dg_handle_t;

enum DGERR {
	DGERR_SUCCESS = 0,
	DGERR_OUT_OF_MEMORY,
	DGERR_INVALID_PARAM,
	DGERR_UNKNOWN_ERROR,
	DGERR_INCONSISTENT,
	DGERR_TIMEOUT,
	DGERR_OVERFLOWED
};

#define DG_STR(x) #x
#define DG_TO_STRING(x) DG_STR(x)

//TODO: K.D. nothing temporarily
#define DG_ERROR(x, ...) ((void)0)
#define DG_WARNING(x, ...) ((void)0)

#define DG_ALIGN_DOWN(x, a) (((x)/(a))*(a))
#define DG_ALIGN_UP(x, a)   ((((x)+(a)-1)/(a))*(a))

/**
* common declarations
*/
#if defined(_MSC_VER)
#define DG_API __cdecl
#else
/* GCC/CLANG */
#define DG_API __attribute__((visibility("default")))
#endif