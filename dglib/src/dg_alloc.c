#include "dg_alloc.h"
#include "dg_string.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

/* default impl */
void* win32_mem_alloc(void* poldmem, size_t size, uint32_t flags) {
	void* ptr;
	DWORD dwerror;
	if (!size)
		return NULL;

	HANDLE hheap = GetProcessHeap();
	if (!hheap) {
		dwerror = GetLastError();
		DG_ERROR("win32_mem_alloc(): GetProcessHeap() failed! GetLastError()=%d (0x%x)", dwerror, dwerror);
		return NULL;
	}

	ptr = HeapAlloc(hheap, 0, size);
	if (!ptr)
		return NULL;

	if (flags & DGMM_CLEAR)
		mem_set(ptr, 0, size);

	if (poldmem && (flags & DGMM_COPY))
		mem_copy(ptr, poldmem, size);

	return ptr;
}

int win32_mem_free(void* pmem) {
	DWORD dwerror;
	if (pmem) {
		HANDLE hheap = GetProcessHeap();
		if (!hheap) {
			dwerror = GetLastError();
			DG_ERROR("win32_mem_alloc(): GetProcessHeap() failed! GetLastError()=%d (0x%x)", dwerror, dwerror);
			return DGERR_UNKNOWN_ERROR;
		}

		if (!HeapFree(hheap, 0, pmem)) {
			dwerror = GetLastError();
			DG_ERROR("win32_mem_alloc(): HeapFree() failed! GetLastError()=%d (0x%x)", dwerror, dwerror);
			return DGERR_UNKNOWN_ERROR;
		}
		return DGERR_SUCCESS;
	}
	return DGERR_INVALID_PARAM;
}

void* win32_mem_alloc_debug(void* poldmem, size_t size, uint32_t flags, const char* pfile, int line) {
	void* ptr = win32_mem_alloc(poldmem, size, flags);
	if (!ptr) {
		DG_ERROR("mem_alloc_debug(): allocation failed! File: %s Line: %d\n", pfile, line);
		return NULL;
	}
	return ptr;
}

int win32_mem_free_debug(void* pmem, const char* pfile, int line) {
	int status = win32_mem_free(pmem);
	if (status != DGERR_SUCCESS)
		DG_ERROR("mem_free_debug(): free failed! status: %d  File: %s  Line: %d\n",
			status, pfile, line);

	return status;
}

#endif

dg_memmgr_dt_t glob_allocdt = {
#ifdef _WIN32
	.mem_alloc = win32_mem_alloc,
	.mem_alloc_debug = win32_mem_alloc_debug,
	.mem_free = win32_mem_free,
	.mem_free_debug = win32_mem_free_debug
#else
	/* TODO: K.D. add linux impls */
	.mem_alloc = 0,
	.mem_alloc_debug = 0,
	.mem_free = 0,
	.mem_free_debug = 0
#endif
};

int initialize_memory()
{
	DG_LOG("initialize_memory(): initializing mem allocator...");

	return 1;
}

void deinitialize_memory()
{
	DG_LOG("deinitialize_memory(): deinitializing mem allocator");
}

dg_memmgr_dt_t* mm_get_dt()
{
	return &glob_allocdt;
}

int ma_create_heap(dg_ma_heap_t* pdst, const ma_heap_info_t* pheapinfo, const char* pname)
{
	return 0;
}

DG_API dg_ma_heap_t ma_find_heap(const char* pname)
{
	return NULL;
}

int ma_destroy_heap(dg_ma_heap_t hheap)
{
	return 0;
}

int ma_thread_set_heap(dg_ma_heap_t hheap)
{
	return 0;
}

dg_ma_heap_t ma_get_process_heap()
{
	return NULL;
}

dg_ma_heap_t ma_thread_get_heap()
{
	return NULL;
}

void* ma_alloc(void* pblock, size_t size, uint32_t flags)
{
	return glob_allocdt.mem_alloc(pblock, size, flags);
}

void* ma_allocdbg(void* pblock, size_t size, uint32_t flags, const char* pfile, int line)
{
	return glob_allocdt.mem_alloc_debug(pblock, size, flags, pfile, line);
}

int ma_free(void* pblock)
{
	return glob_allocdt.mem_free(pblock);
}

int ma_freedbg(void* pblock, const char* pfile, int line)
{
	return glob_allocdt.mem_free_debug(pblock, pfile, line);
}

int ma_stats(ma_stats_t* pdst)
{
	return 0;
}
