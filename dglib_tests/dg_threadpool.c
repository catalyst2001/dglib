#include "dg_threadpool.h"
#include "dg_cpuinfo.h"

#include <stdio.h>

int thread_pool_workers_entry(struct dg_thrd_data_s* ptinfo)
{
  printf("thread_pool_workers_entry(): thread %d from pool started\n", dg_get_curr_thread_id());
  dg_worker_t* pworker = (dg_worker_t*)ptinfo->puserdata;
  dg_threadpool_t* pthreadpool = pworker->ptpool;
  setjmp(pworker->start_context);//TODO: K.D. use this later
  while (dg_atomic_load(&pthreadpool->status) == DGTPSTATUS_RUNNING) {
    printf("thread_pool_workers_entry() thread %u start execution\n", dg_get_curr_thread_id());
    dg_task_t *ptask = (dg_task_t*)mtqueue_get_front(&pthreadpool->tasks);
    ptask->pworker = pworker;
    ptask->tstart = 0.; //TODO: K.D. use this later

    /* check special termination marker in task */
    if (!ptask->ptaskproc)
      break; //break cycle
    
    ptask->ptaskproc(ptask);
  }
  semaphore_post(pthreadpool->pfinish_sem);
  return 0;
}

int tp_init(dg_threadpool_t* ptp, size_t num_threads)
{
  dg_cpu_info_t cpuinfo;
  cpu_get_info(&cpuinfo);
  /* limit number of logical processors */
  if (cpuinfo.num_logical_processors > num_threads)
    cpuinfo.num_logical_processors = num_threads;

  /* init containers */
  ptp->pfinish_sem = semaphore_alloc(0, (int)cpuinfo.num_logical_processors, "dg_threadpool_t:pfinish_sem");
  ptp->workers = darray_init(dg_worker_t, cpuinfo.num_logical_processors, 1, 0);
  if (!mtqueue_alloc(&ptp->tasks, sizeof(dg_task_t), DG_TASKS_QUEUE_DEFAULT_LIMIT)) {
    DG_ERROR("threadpool_init(): mtqueue_alloc() failed");
    return DGERR_OUT_OF_MEMORY;
  }
  assert(ptp->tasks.elemsize == sizeof(dg_task_t) && "task structure size is invalid!");

  dg_thread_init_info_t thread_init_info = {
    .affinity=DGT_AUTO_AFFINITY,
    .flags=DGTF_NONE,
    .linalloc_size=0,
    .priority=DGPRIOR_DEFAULT,
    .pthread_end_routine=NULL,
    .pthread_pre_routine=NULL,
    .pthread_start_routine = thread_pool_workers_entry,
    .puserptr = ptp,
    .stack_size=0
  };

  /* set running state */
  dg_atomic_store(&ptp->status, DGTPSTATUS_RUNNING);

  /* create workers */
  for (size_t i = 0; i < cpuinfo.num_logical_processors; i++) {
    thread_init_info.affinity = (uint32_t)i;
    dg_worker_t* pworker = (dg_worker_t *)darray_add_back(&ptp->workers);
    if (!pworker) {
      DG_ERROR("threadpool_init(): darray_add_back() failed");
      return DGERR_OUT_OF_MEMORY;
    }
    pworker->ptpool = ptp;
    thread_init_info.puserptr = pworker;
    pworker->hthread = dg_thread_create_ex(&thread_init_info);
    if (!pworker->hthread) {
      DG_ERROR("threadpool_init(): thread_create() failed");
      return DGERR_OUT_OF_MEMORY;
    }
  }
  return DGERR_SUCCESS;
}

void tp_stop(dg_threadpool_t* ptp)
{
  dg_task_t termination_task = {
    .ptaskproc = NULL, //special flag for termination
    .puserdata = NULL,
    .pworker = NULL,
    .tlimit = 0., //TODO: K.D. use this later
    .tstart = 0.  //TODO: K.D. use this later
  };

  dg_atomic_store(&ptp->status, DGTPSTATUS_TERMINATE);
  for (size_t i = 0; i < darray_get_size(&ptp->workers); i++)
    mtqueue_add_back(&ptp->tasks, &termination_task);
}

int tp_task_add(dg_threadpool_t* ptp, 
  dg_task_start_proc ptaskexec,
  dg_task_skip_proc ptaskskip,
  uint32_t task_priority,
  void* puserdata,
  double timeout)
{
  dg_task_t task = {
    .ptaskproc= ptaskexec,
    .puserdata= puserdata,
    .pworker=NULL,
    .tlimit=0., //TODO: K.D. use this later
    .tstart=0.  //TODO: K.D. use this later
  };
  
  if (!mtqueue_try_add_back(&ptp->tasks, &task)) {
    DG_ERROR("threadpool_task_add(): threadpool queue overflowed!");
    return DGERR_OVERFLOWED;
  }
  return DGERR_SUCCESS;
}

void tp_join(dg_threadpool_t* ptp)
{
  assert(ptp->pfinish_sem && "ptp->pfinish_sem is NULL");
  for (size_t i = 0; i < darray_get_size(&ptp->workers); i++)
    semaphore_wait(ptp->pfinish_sem);
}

int tp_deinit(dg_threadpool_t* ptp)
{
  dg_worker_t* pworker;
  tp_stop(ptp);
  tp_join(ptp);
  for (size_t i = 0; i < darray_get_size(&ptp->workers); i++) {
    pworker = darray_getptr(&ptp->workers, i, dg_worker_t);
    dg_thread_close(pworker->hthread);
  }
  darray_free(&ptp->workers);
  mtqueue_free(&ptp->tasks);
  semaphore_free(ptp->pfinish_sem);
  return DGERR_SUCCESS;
}