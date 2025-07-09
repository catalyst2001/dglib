#pragma once
#include "dg_libcommon.h"

/**
* mutex API
*/
typedef dg_handle_t dg_mutex_t;
dg_mutex_t mtx_alloc(const char *pname);
void       mtx_lock(dg_mutex_t hmutex);
bool       mtx_try_lock(dg_mutex_t hmutex);
void       mtx_unlock(dg_mutex_t hmutex);
void       mtx_free(dg_mutex_t hmutex);

/**
* cond API
*/
typedef dg_handle_t dg_cond_t;
dg_cond_t  cond_alloc(const char *pname);
void       cond_wait(dg_cond_t hcond, dg_mutex_t hmutex);
void       cond_signal(dg_cond_t hcond);
void       cond_broadcast(dg_cond_t hcond);
void       cond_free(dg_cond_t hcond);