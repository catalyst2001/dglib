#pragma once
#include "dg_libcommon.h"

/**
* mutex API
*/
typedef dg_voidptr_t dg_mutex_t; /*< mutex handle definition */

/**
* @brief Allocate a mutex
* @param pname - Name of the mutex (for debugging purposes)
* @return Handle to the allocated mutex, or NULL on failure
*/
DG_API dg_mutex_t mutex_alloc(const char *pname);

/**
* @brief Lock the mutex
* @param hmutex - Handle to the mutex
* @note This function will block until the mutex is acquired
*/
DG_API void mutex_lock(dg_mutex_t hmutex);

/**
* @brief Try to lock the mutex
* @param hmutex - Handle to the mutex
* @return true if the mutex was successfully locked, false otherwise
*/
DG_API bool mutex_try_lock(dg_mutex_t hmutex);

/**
* @brief Unlock the mutex
* @param hmutex - Handle to the mutex
* @note The mutex must be locked by the calling thread
* @note Unlocking an unlocked mutex or a mutex locked by another thread results in undefined behavior
*/
DG_API void mutex_unlock(dg_mutex_t hmutex);

/**
* @brief Free the mutex
* @param hmutex - Handle to the mutex
* @note The mutex must be unlocked and not used by any thread when this function is called
*/
DG_API void mutex_free(dg_mutex_t hmutex);

/**
* cond API
*/
typedef dg_voidptr_t dg_cond_t; /*< condition variable handle definition */

/**
* @brief Allocate a condition variable
* @param pname - Name of the condition variable (for debugging purposes)
* @return Handle to the allocated condition variable, or NULL on failure
*/
DG_API dg_cond_t  cond_alloc(const char *pname);

/**
* @brief Wait on the condition variable
* @param hcond - Handle to the condition variable
* @param hmutex - Handle to the mutex associated with the condition variable
* @note The mutex must be locked by the calling thread before calling this function
*/
DG_API void cond_wait(dg_cond_t hcond, dg_mutex_t hmutex);

/**
* @brief Signal one thread waiting on the condition variable
* @param hcond - Handle to the condition variable
* @note If no threads are waiting, this function has no effect
*/
DG_API void cond_signal(dg_cond_t hcond);

/**
* @brief Broadcast to all threads waiting on the condition variable
* @param hcond - Handle to the condition variable
* @note If no threads are waiting, this function has no effect
*/
DG_API void cond_broadcast(dg_cond_t hcond);

/**
* @brief Free the condition variable
* @param hcond - Handle to the condition variable
*/
DG_API void cond_free(dg_cond_t hcond);

/*
* semaphore
*/
typedef dg_voidptr_t dg_semaphore_t; /*< semaphore handle */

/**
* @brief Allocate a semaphore
* @param initial_value - Initial value of the semaphore
* @param max_count - Maximum count of the semaphore
* @param pname - Name of the semaphore (for debugging purposes)
* @return Handle to the allocated semaphore, or NULL on failure
*/
DG_API dg_semaphore_t semaphore_alloc(int initial_value, int max_count, const char* pname);

/**
* @brief Wait (decrement) the semaphore
*	@param hsem - Handle to the semaphore
*/
DG_API void semaphore_wait(dg_semaphore_t hsem);

/**
* @brief Try to wait (decrement) the semaphore
* @param hsem - Handle to the semaphore
*/
DG_API int semaphore_trywait(dg_semaphore_t hsem);

/**
* @brief Wait (decrement) the semaphore with a timeout
* @param hsem - Handle to the semaphore
* @param waitms - Timeout in milliseconds
* @return true if the semaphore was successfully decremented, false if the timeout occurred
*/
DG_API bool semaphore_timed_wait(dg_semaphore_t hsem, int waitms);

/**
* @brief Post (increment) the semaphore
* @param hsem - Handle to the semaphore
*/
DG_API void semaphore_post(dg_semaphore_t hsem);

/**
* @brief Free the semaphore
* @param hsem - Handle to the semaphore
*/
DG_API void semaphore_free(dg_semaphore_t hsem);


/**
* rwlock
*/
typedef dg_voidptr_t dg_rwlock_t; /*< read-write lock handle */

/**
* @brief Allocate a read-write lock
* @param pname - Name of the read-write lock (for debugging purposes)
* @return Handle to the allocated read-write lock, or NULL on failure
*/
DG_API dg_rwlock_t rwlock_alloc(const char* pname);

/**
* @brief Free the read-write lock
* @param hrwlock - Handle to the read-write lock
*/
DG_API void rwlock_free(dg_rwlock_t hrwlock);

/**
* @brief Acquire a read lock
* @param hrwlock - Handle to the read-write lock
* @return 0 on success, non-zero on failure
*/
DG_API int  rwlock_rdlock(dg_rwlock_t hrwlock);

/**
* @brief Release a read lock
* @param hrwlock - Handle to the read-write lock
* @return 0 on success, non-zero on failure
*/
DG_API int  rwlock_rdunlock(dg_rwlock_t hrwlock);

/**
* @brief Acquire a write lock
* @param hrwlock - Handle to the read-write lock
* @return 0 on success, non-zero on failure
*/
DG_API int  rwlock_wrlock(dg_rwlock_t hrwlock);

/**
* @brief Release a write lock
* @param hrwlock - Handle to the read-write lock
* @return 0 on success, non-zero on failure
*/
DG_API int  rwlock_wrunlock(dg_rwlock_t hrwlock);

/**
* @brief Try to acquire a read lock
* @param hrwlock - Handle to the read-write lock
* @return 0 on success, non-zero on failure
*/
DG_API int  rwlock_try_rdlock(dg_rwlock_t hrwlock);

/**
* @brief Try to acquire a write lock
* @param hrwlock - Handle to the read-write lock
* @return 0 on success, non-zero on failure
*/
DG_API int  rwlock_try_wrlock(dg_rwlock_t hrwlock);