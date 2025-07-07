#pragma once
#include <stddef.h>
#include <stdint.h>
#include <assert.h> //TODO: K.D. replace this later
#include <stdbool.h> //TODO: K.D. replace this later

enum DGERR {
	DGERR_NONE = 0,
	DGERR_OUT_OF_MEMORY,
	DGERR_INVALID_PARAM,
	DGERR_INCONSISTENT
};

#define DG_STR(x) #x
#define DG_TO_STRING(x) DG_STR(x)

/**
* common declarations
*/
#if defined(_MSC_VER)
#define DG_API __cdecl
#else
/* GCC/CLANG */
#define DG_API __attribute__((visibility("default")))
#endif