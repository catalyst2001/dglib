#pragma once
#include "dg_libcommon.h"

/**
* mutex API
*/
typedef dg_handle_t dg_mutex_t;
dg_mutex_t mutex_alloc(const char *pname);
void       mutex_lock(dg_mutex_t hmutex);
bool       mutex_try_lock(dg_mutex_t hmutex);
void       mutex_unlock(dg_mutex_t hmutex);
void       mutex_free(dg_mutex_t hmutex);

/**
* cond API
*/
typedef dg_handle_t dg_cond_t;
dg_cond_t  cond_alloc(const char *pname);
void       cond_wait(dg_cond_t hcond, dg_mutex_t hmutex);
void       cond_signal(dg_cond_t hcond);
void       cond_broadcast(dg_cond_t hcond);
void       cond_free(dg_cond_t hcond);

/*
* semaphore
*/
typedef dg_handle_t dg_semaphore_t;
dg_semaphore_t  semaphore_alloc(int initial_value, int max_count, const char* pname);
void      semaphore_wait(dg_semaphore_t hsem);
int       semaphore_trywait(dg_semaphore_t hsem);
bool      semaphore_timed_wait(dg_semaphore_t hsem, int waitms);
bool      semaphore_try_wait(dg_semaphore_t hsem);
void      semaphore_post(dg_semaphore_t hsem);
void      semaphore_free(dg_semaphore_t hsem);