#include "dg_threadpool.h"
#include "dg_cpuinfo.h"

int thread_pool_workers_entry(struct dg_thrd_data_s* ptinfo)
{
  dg_worker_t* pworker = (dg_worker_t*)ptinfo->puserdata;
  dg_threadpool_t* pthreadpool = pworker->ptpool;
  setjmp(pworker->start_context);//TODO: K.D. user this later
  while (dg_atomic_load(&pthreadpool->status) == DGTPSTATUS_RUNNING) {
    dg_task_t *ptask = (dg_task_t*)mtqueue_get_front(&pthreadpool->tasks); //TODO: K.D. mtqueue_get_front() call blocking this thread???
    if (ptask) {
      ptask->pworker = pworker;
      ptask->tstart = 0.; //TODO: K.D. user this later
      ptask->ptaskproc(ptask);
    }
  }
  return 0;
}

int threadpool_init(dg_threadpool_t* ptp, size_t num_threads)
{
  dg_cpu_info_t cpuinfo;
  cpu_get_info(&cpuinfo);
  /* limit number of logical processors */
  if (cpuinfo.num_logical_processors > num_threads)
    cpuinfo.num_logical_processors = num_threads;

  /* init containers */
  ptp->workers = darray_init(dg_worker_t, cpuinfo.num_logical_processors, 1, 0);
  if (!mtqueue_alloc(&ptp->tasks, DG_TASKS_QUEUE_DEFAULT_LIMIT)) {
    DG_ERROR("threadpool_init(): mtqueue_alloc() failed");
    return DGERR_OUT_OF_MEMORY;
  }

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

int threadpool_task_add(dg_threadpool_t* ptp, 
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
    .tlimit=0., //TODO: K.D. user this later
    .tstart=0.  //TODO: K.D. user this later
  };
  
  if (!mtqueue_try_add_back(&ptp->tasks, &task)) {
    DG_ERROR("threadpool_task_add(): threadpool queue overflowed!");
    return DGERR_OVERFLOWED;
  }
  return DGERR_SUCCESS;
}
