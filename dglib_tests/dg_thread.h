#pragma once
#include "dg_libcommon.h"
#include "dg_darray.h"
#include "dg_linalloc.h"

typedef dg_handle_t dg_thrd_t;
struct dg_thrd_data_s;

/**
* @brief Thread entry point callback
*/
typedef int  (*dg_thrd_proc)(struct dg_thrd_data_s* ptinfo);

/**
* @brief Preparation procedure. Called before the thread entry point is called.
*/
typedef void (*dg_thrd_pre_proc)(struct dg_thrd_data_s *ptinfo);

/**
* @brief Called when a thread terminates.
*/
typedef void (*dg_thread_end_proc)(struct dg_thrd_data_s* ptinfo);

/**
* @brief internal dglib thread data
*/
typedef struct dg_thrd_data_s {
	uint32_t           flags; /*< current flags */
	dg_hunkalloc_t     hunk_allocator; /*< linear allocator */
	size_t             stack_size; /*< size of thread stack */
	dg_darray_t        thrd_data; /*< thread user data */
	dg_thrd_t          thrd_handle; /*< this thread handle */
	dg_thrd_proc       pthread_start_routine; /*< address of thread start routine */
	dg_thrd_pre_proc   pthread_pre_routine; /*< address of thread pre start routine */
	dg_thread_end_proc pthread_end_routine; /*< address of thread end routine */
	void*              puserdata; /*< address of userdata from thread creation */
} dg_thrd_data_t;

enum DGPRIOR {
	DGPRIOR_LOW = 0,
	DGPRIOR_MIDDLE,
	DGPRIOR_HIGH,
	DGPRIOR_DEFAULT
};

#define DGTF_NONE (0)
#define DGTF_USE_LINALLOC (1 << 0)
#define DGTF_SUSPENDED (1 << 0)

#define DGT_WAIT_INFINITE ((uint32_t)-1)
#define DGT_AUTO_AFFINITY ((uint32_t)-1)

/**
* @brief thread init struct
*/
typedef struct dg_thread_init_info_s {
	uint32_t           flags; /*< creation flags */
	uint32_t           priority; /*< priority */
	uint32_t           affinity; /*< affinity */
	size_t             linalloc_size; /*< linear allocator reserved block size */
	size_t             stack_size; /*< size of stack in bytes */
	dg_thrd_proc       pthread_start_routine; /*< thread start routine address */
	dg_thrd_pre_proc   pthread_pre_routine; /*< thread pre start routine address */
	dg_thread_end_proc pthread_end_routine; /*< thread finish execution routine address */
	void* puserptr; /*< user pointer */
} dg_thread_init_info_t;

/**
* @brief extended thread creation
* @note This function supports affinity up to 32 cores.
* @param pthreadinfo - struct with all information for thread creation
* @return handle of created thread, otherwise returns NULL
*/
dg_thrd_t dg_thread_create_ex(const dg_thread_init_info_t *pthreadinfo);

/**
* @brief simple thread creation
* @param stacksize - size of thread stack
* @param pthreadroutine - address of thread entry point
* @param puserdata - address of user data
* @return handle of created thread, otherwise returns NULL
*/
dg_thrd_t dg_thread_create(uint32_t stacksize, dg_thrd_proc pthreadroutine, void *puserdata);

/**
* @brief get current thread dglib data
* @return If the thread was created by the dglib subsystem, the address of the thread description structure is returned.
* @return If the thread was not created by dglib, NULL is returned.
*/
dg_thrd_data_t* dg_get_curr_thread_data();

/**
* @brief gets the handle to the current thread
* @return Returns a handle to the current thread.
* @return This function never returns NULL
*/
dg_thrd_t dg_get_curr_thread();

/**
* @brief gets the id to the current thread
* @return Returns a id to the current thread.
*/
uint32_t dg_get_curr_thread_id();

/**
* @brief Waits indefinitely for a thread to join.
* @return If the thread has joined, DGERR_SUCCESS is returned
* @return If an internal error occurs, DGERR_UNKNOWN_ERROR is returned
*/
int dg_thread_join(dg_thrd_t hthread);

/**
* @brief close thread handle
* @param hthread - handle of thread
* @param timeout - wait timeout (DGT_WAIT_INFINITE for umlimited wait)
* @return if an invalid thread handle is passed, returns DGERR_INVALID_PARAM.
* @return If the thread completed within the specified time interval, DGERR_SUCCESS is returned.
* @return If the thread fails to complete before the specified time, DGERR_TIMEOUT is returned.
*/
bool dg_thread_close(dg_thrd_t hthread);

/**
* @brief waiting for thread to join with timeout
* @param hthread - handle of existing thread
* @param timeout - wait timeout (DGT_WAIT_INFINITE for umlimited wait)
* @return if an invalid thread handle is passed, returns DGERR_INVALID_PARAM.
* @return If the thread completed within the specified time interval, DGERR_SUCCESS is returned.
* @return If the thread fails to complete before the specified time, DGERR_TIMEOUT is returned.
*/
int dg_thread_join_timed(dg_thrd_t hthread, uint32_t timeout);

/**
* @brief gets the return value of the terminated thread
* @param hthread - handle of existing thread
* @return if an invalid thread handle is passed, returns DGERR_INVALID_PARAM.
* @return Otherwise, DGERR_SUCCESS is returned.
*/
int dg_thread_get_exit_code(int* pdst, dg_thrd_t hthread);