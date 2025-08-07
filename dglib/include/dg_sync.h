#pragma once
#include "dg_libcommon.h"

/**
* mutex API
*/
typedef dg_voidptr_t dg_mutex_t;
DG_API dg_mutex_t mutex_alloc(const char *pname);
DG_API void       mutex_lock(dg_mutex_t hmutex);
DG_API bool       mutex_try_lock(dg_mutex_t hmutex);
DG_API void       mutex_unlock(dg_mutex_t hmutex);
DG_API void       mutex_free(dg_mutex_t hmutex);

/**
* cond API
*/
typedef dg_voidptr_t dg_cond_t;
DG_API dg_cond_t  cond_alloc(const char *pname);
DG_API void       cond_wait(dg_cond_t hcond, dg_mutex_t hmutex);
DG_API void       cond_signal(dg_cond_t hcond);
DG_API void       cond_broadcast(dg_cond_t hcond);
DG_API void       cond_free(dg_cond_t hcond);

/*
* semaphore
*/
typedef dg_voidptr_t dg_semaphore_t;
DG_API dg_semaphore_t  semaphore_alloc(int initial_value, int max_count, const char* pname);
DG_API void      semaphore_wait(dg_semaphore_t hsem);
DG_API int       semaphore_trywait(dg_semaphore_t hsem);
DG_API bool      semaphore_timed_wait(dg_semaphore_t hsem, int waitms);
//DG_API bool      semaphore_try_wait(dg_semaphore_t hsem);
DG_API void      semaphore_post(dg_semaphore_t hsem);
DG_API void      semaphore_free(dg_semaphore_t hsem);


/**
* rwlock
*/
typedef dg_voidptr_t dg_rwlock_t;
DG_API dg_rwlock_t rwlock_alloc(const char* pname);
DG_API void rwlock_free(dg_rwlock_t hrwlock);
DG_API int  rwlock_rdlock(dg_rwlock_t hrwlock);
DG_API int  rwlock_rdunlock(dg_rwlock_t hrwlock);
DG_API int  rwlock_wrlock(dg_rwlock_t hrwlock);
DG_API int  rwlock_wrunlock(dg_rwlock_t hrwlock);
DG_API int  rwlock_try_rdlock(dg_rwlock_t hrwlock);
DG_API int  rwlock_try_wrlock(dg_rwlock_t hrwlock);