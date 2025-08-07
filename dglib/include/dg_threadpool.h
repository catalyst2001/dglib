#pragma once
#include "dg_darray.h"
#include "dg_thread.h"
#include "dg_queue.h"
#include <setjmp.h>

#define DG_TASKS_QUEUE_DEFAULT_LIMIT 2048

/**
* @brief Task termination reasons
*/
enum DGTASKTERM {
	DGTASKTERM_TIMEOUT = 0, /*< task skiped by timeout */
	DGTASKTERM_MANUAL /*< task skipped by user */
};

/**
* @brief Task priority
*/
enum DGTASKPRIOR {
	DGTASKPRIOR_LOW = 0, /*< Task low priority. Add task to back of the queue */
	DGTASKPRIOR_MIDDLE, /*< Task middle priority. Add task to queue center */
	DGTASKPRIOR_HIGH /*< Task high priority! Add this task to the front */
};

enum DGTPSTATUS {
	DGTPSTATUS_RUNNING = 0,
	DGTPSTATUS_TERMINATE
};

struct dg_task_s; //forward decl

/**
* @brief Task start proc pointer
*/
typedef void (*dg_task_start_proc)(struct dg_task_s *ptask);

/**
* @brief Task skip proc pointer
*/
typedef void (*dg_task_skip_proc)(struct dg_task_s *ptask, int taskterm_reason);

/**
* @brief Threadpool worker structure
*/
struct dg_threadpool_s;
typedef struct dg_worker_s {
	dg_thrd_t hthread; /*< worker thread handle */
	jmp_buf   start_context; /*< worker thread start context to reset (for reset loop tasks) */
	struct dg_threadpool_s* ptpool;
} dg_worker_t;

/**
* @brief One task structure
*/
typedef struct dg_task_s {
	double        tstart; /*< task creation time */
	double        tlimit; /*< task time limit */
	dg_task_start_proc ptaskproc; /*< task exec proc */
	dg_worker_t*  pworker; /*< worker thread pointer */
	void*         puserdata; /*< user data pointer */
} dg_task_t;

/**
* @brief Threadpool structure
*/
typedef struct dg_threadpool_s {
	atomic_size_t  status;
	dg_darray_t    workers; /*< workers dynamic array */
	dg_mtqueue_t   tasks; /*< thread safe tasks queue */
	dg_semaphore_t pfinish_sem;
} dg_threadpool_t;

/**
* @brief Initialize thread pool
* 
* @param ptp - address of thread pool structure
* @param num_threads - num of worker threads
* @return DGERR_SUCCESS if operation sucessfully completed
* @return DGERR_OUT_OF_MEMORY if no enough RAM space
* @return DGERR_UNKNOWN_ERROR if ocurred internal platform error
*/
DG_API int tp_init(dg_threadpool_t *ptp, size_t num_threads);

/**
* @brief Adds special tasks to the general queue to complete worker threads.
* @note  This function may block the calling thread if the queue is full.
* 
* @param ptp - address of thread pool structure
* @return nothing
*/
DG_API void tp_stop(dg_threadpool_t* ptp);

/**
* @brief Adds a task to a shared queue with a specified execution priority
*
* @param ptp - address of thread pool structure
* @param ptaskexec - the address of the procedure that the worker thread will begin executing. This value should never be NULL in normal mode, since its presence means that the worker thread will terminate!
* @param ptaskskip - address of the procedure that is called if the task was canceled. The reason for termination is indicated
* @param task_priority - Priority described by enumeration DGTASKPRIOR. Currently ignored!
* @param puserdata - address of user data to be transferred to the task execution procedure
* @param timeout - Task timeout. Ñurrently ignored!
* @return DGERR_SUCCESS if operation sucessfully completed
* @return DGERR_UNKNOWN_ERROR if ocurred internal platform error
*/
DG_API int tp_task_add(dg_threadpool_t* ptp,
	dg_task_start_proc ptaskexec,
	dg_task_skip_proc ptaskskip,
	uint32_t task_priority,
	void *puserdata,
	double timeout);

/**
* @brief Waits for all worker threads to complete
* 
* @param ptp - address of thread pool structure
* @return nothing
*/
DG_API void tp_join(dg_threadpool_t* ptp);

/**
* @brief Terminates the thread pool
* 
* @param ptp - address of thread pool structure
* @return DGERR_SUCCESS if operation sucessfully completed
*/
DG_API int tp_deinit(dg_threadpool_t* ptp);