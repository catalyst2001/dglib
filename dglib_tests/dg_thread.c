#include "dg_thread.h"
#include "dg_alloc.h"

/**
* thread_internal_new_data
* @brief allocate new internal dglib thread data
* 
*/ 
dg_thrd_data_t* thread_internal_new_data(const dg_thread_init_info_t* pthreadinfo)
{
	dg_thrd_data_t* pthrd_data = DG_NEW(dg_thrd_data_t);
	if (!pthrd_data) {
		DG_ERROR("thread_internal_new_data(): failed to alloc internal thread data");
		return NULL;
	}
	pthrd_data->flags = pthreadinfo->flags;
	pthrd_data->pthread_pre_routine = pthreadinfo->pthread_pre_routine;
	pthrd_data->pthread_start_routine = pthreadinfo->pthread_start_routine;
	pthrd_data->pthread_end_routine = pthreadinfo->pthread_end_routine;
	pthrd_data->stack_size = pthreadinfo->stack_size;
	pthrd_data->puserdata = pthreadinfo->puserptr;

	/* init linear allocator if needed */
	if (pthreadinfo->linalloc_size) {
		if (linalloc_init(&pthrd_data->hunk_allocator, pthreadinfo->linalloc_size))
			pthrd_data->flags |= DGTF_USE_LINALLOC;
		else DG_WARNING("thread_internal_new_data(): linear allocator block allocation failed!");
	}
	return pthrd_data;
}

/* NOTE: K.D. windows implementation */
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

DWORD glob_tls_index = TLS_OUT_OF_INDEXES;

int initialize_threads()
{
	DG_LOG("initialize_threads(): initializing threads subsystem...");
	glob_tls_index = TlsAlloc();
	if (TLS_OUT_OF_INDEXES == glob_tls_index) {
		DG_ERROR("init_thread_subsystem(): TlsAlloc() returned TLS_OUT_OF_INDEXES! No free TLS slots");
		return 0;
	}
	DG_LOG("initialize_threads(): threads initialized! TLS is %d", glob_tls_index);
	return 1;
}

void deinitialize_threads()
{
	DG_LOG("deinitialize_threads(): deinitializing threads subsystem...");
	if (glob_tls_index != TLS_OUT_OF_INDEXES) {
		TlsFree(glob_tls_index);
		glob_tls_index = TLS_OUT_OF_INDEXES;
	}
}

DWORD WINAPI win32_thread_proc(dg_thrd_data_t* pthread_data)
{
	int return_value;
	assert(glob_tls_index != -1 && "dg_threadsys_tls_index is invalid!");
	assert(pthread_data->pthread_start_routine != NULL && "pthread_data->pthread_start_routine is NULL");
	TlsSetValue(glob_tls_index, pthread_data);
	if(pthread_data->pthread_pre_routine)
		pthread_data->pthread_pre_routine(pthread_data);

	return_value = pthread_data->pthread_start_routine(pthread_data);
	if (pthread_data->pthread_end_routine)
		pthread_data->pthread_end_routine(pthread_data);

	free(pthread_data);
	return (DWORD)return_value;
}

dg_thrd_t dg_thread_create_ex(const dg_thread_init_info_t* pthreadinfo)
{
	DWORD           threadid;
	HANDLE          hthread;
	dg_thrd_data_t* pthrd_data;
	if (!pthreadinfo) {
		assert(pthreadinfo && "pthreadinfo is NULL");
		return NULL;
	}

	pthrd_data = thread_internal_new_data(pthreadinfo);
	if (!pthrd_data)
		return NULL;

	hthread = CreateThread(
		NULL,
		(SIZE_T)pthreadinfo->stack_size,
		(LPTHREAD_START_ROUTINE)win32_thread_proc,
		pthrd_data,
		0,
		&threadid
	);
	/* thread created? */
	if (hthread == NULL) {
		free(pthrd_data);
		threadid = GetLastError();
		DG_ERROR("CreateThread(): failed. GetLastError(): %d (0x%x)", 
			threadid, threadid);
		return NULL;
	}

	if (pthreadinfo->affinity != DGT_AUTO_AFFINITY && pthreadinfo->affinity < 32)
		SetThreadAffinityMask(hthread, ((ULONG_PTR)1 << pthreadinfo->affinity));

	if (pthreadinfo->priority < DGPRIOR_DEFAULT) {
		static const int win32_priors[] = {
			THREAD_PRIORITY_LOWEST, //DGPRIOR_LOW
			THREAD_PRIORITY_NORMAL, //DGPRIOR_MIDDLE
			THREAD_PRIORITY_TIME_CRITICAL //DGPRIOR_HIGH
		};
		SetThreadPriority(hthread, win32_priors[pthreadinfo->priority]);
	}
	return (dg_thrd_t)hthread;
}

dg_thrd_t dg_thread_create(uint32_t stacksize, dg_thrd_proc pthreadroutine, void* puserdata)
{
	dg_thread_init_info_t tii = {
		.affinity = DGT_AUTO_AFFINITY,
		.flags = DGTF_NONE,
		.linalloc_size = 0,
		.priority = DGPRIOR_LOW,
		.pthread_end_routine = NULL,
		.pthread_pre_routine = NULL,
		.pthread_start_routine = pthreadroutine,
		.stack_size = stacksize,
		.puserptr = puserdata
	};
	return dg_thread_create_ex(&tii);
}

dg_thrd_data_t* dg_get_curr_thread_data()
{
	return TlsGetValue(glob_tls_index);
}

dg_thrd_t dg_get_curr_thread()
{
	return (dg_thrd_t)GetCurrentThread();
}

uint32_t dg_get_curr_thread_id()
{
	return (uint32_t)GetCurrentThreadId();
}

int dg_thread_join(dg_thrd_t hthread)
{
	return dg_thread_join_timed(hthread, DGT_WAIT_INFINITE);
}

bool dg_thread_close(dg_thrd_t hthread)
{
	if (!hthread) {
		assert(hthread && "hthread was NULL");
		DG_ERROR("hthread is invalid");
		return DGERR_INVALID_PARAM;
	}
	return !!CloseHandle((HANDLE)hthread);
}

int dg_thread_join_timed(dg_thrd_t hthread, uint32_t timeout)
{
	if (!hthread) {
		assert(hthread && "hthread was NULL");
		DG_ERROR("hthread is invalid");
		return DGERR_INVALID_PARAM;
	}

	if (timeout == DGT_WAIT_INFINITE)
		timeout = INFINITE;

	DWORD result = WaitForSingleObject((HANDLE)hthread, (DWORD)timeout);
	if (result == WAIT_OBJECT_0)
		return DGERR_SUCCESS;
	
	if (result == WAIT_TIMEOUT)
		return DGERR_TIMEOUT;

	if (result == WAIT_FAILED) {
		result = GetLastError();
		DG_ERROR("WaitForSingleObject() returned WAIT_FAILED GetLastError(): %d (0x%x)",
			result, result);
		return DGERR_UNKNOWN_ERROR;
	}
	DG_ERROR("WaitForSingleObject() returned unexpected value! (%d)", result);
	return DGERR_UNKNOWN_ERROR;
}

int dg_thread_get_exit_code(int* pdst, dg_thrd_t hthread)
{
	DWORD return_code;
	if (!hthread) {
		assert(hthread && "hthread was NULL");
		DG_ERROR("hthread is invalid");
		return DGERR_INVALID_PARAM;
	}
	if (!GetExitCodeThread((HANDLE)hthread, &return_code)) {
		return_code = GetLastError();
		DG_ERROR("GetExitCodeThread() returned error! GetLastError(): %d (0x%x)",
			return_code, return_code);
		return DGERR_UNKNOWN_ERROR;
	}
	*pdst = (int)return_code;
	return DGERR_SUCCESS;
}

void dg_delay_ms(uint32_t delay)
{
	Sleep((DWORD)delay);
}

dg_thrd_data_t* dg_thread_attach_info(uint32_t flags, size_t linalloc_size)
{
	DWORD dwerror;
	/* try to get current thread allocated info block */
	dg_thrd_data_t* pthread_data = dg_get_curr_thread_data();
	if (pthread_data)
		return pthread_data;

	/* try to alloc new block and fill by defaults */
	dg_thread_init_info_t thread_new_info = {
		.affinity = DGT_AUTO_AFFINITY,
		.flags = flags,
		.linalloc_size = linalloc_size,
		.priority = DGPRIOR_DEFAULT,
		.pthread_end_routine = NULL,
		.pthread_pre_routine = NULL,
		.pthread_start_routine = NULL,
		.stack_size = 0,
		.puserptr = NULL
	};
	/* try to alloc info */
	pthread_data = thread_internal_new_data(&thread_new_info);
	if (!pthread_data)
		return NULL;

	if (!TlsSetValue(glob_tls_index, pthread_data)) {
		dwerror = GetLastError();
		DG_ERROR("TlsSetValue() failed! GetLastError()=%d (0x%x)", dwerror, dwerror);
		free(pthread_data);
		return NULL;
	}
	return pthread_data;
}

#else

#endif