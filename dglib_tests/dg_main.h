#pragma once
#include "dg_libcommon.h"

/* sysui */
int  initialize_sysui();
void deinitialize_sysui();

/* filesystem */
int  initialize_filesystem();
void deinitialize_filesystem();

/* threads */
int  initialize_threads();
void deinitialize_threads();

/* memory */
int  initialize_memory();
void deinitialize_memory();

DG_API int dg_initialize();
DG_API void dg_deinitialize();