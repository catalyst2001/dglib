#pragma once
#include "dg_libcommon.h"

/**
* memory allocation flags
*/
#define DGMM_NONE  (0)
#define DGMM_COPY  (1<<0)
#define DGMM_CLEAR (1<<1)

/**
* dg_memmgr_dt_t
* memory allocation dispatch table
*/
typedef struct dg_memmgr_dt_s {
	void* (*mem_alloc)(void *poldmem, size_t size, uint32_t flags);
	int   (*mem_free)(void *pmem);
	void* (*mem_alloc_debug)(void* poldmem, size_t size, uint32_t flags, const char *pfile, int line);
	int   (*mem_free_debug)(void* pmem, const char* pfile, int line);
} dg_memmgr_dt_t;

DG_API dg_memmgr_dt_t* mm_get_dt();

typedef dg_voidptr_t dg_ma_heap_t;

typedef struct ma_heap_info_s {
	size_t heap_size;
	size_t heap_reserve;
	size_t min_address;
	size_t max_address;
} ma_heap_info_t;

DG_API int          ma_create_heap(dg_ma_heap_t *pdst, const ma_heap_info_t *pheapinfo, const char *pname);
DG_API dg_ma_heap_t ma_find_heap(const char* pname);
DG_API int          ma_destroy_heap(dg_ma_heap_t hheap);
DG_API int          ma_thread_set_heap(dg_ma_heap_t hheap);
DG_API dg_ma_heap_t ma_get_process_heap();
DG_API dg_ma_heap_t ma_thread_get_heap();
DG_API void        *ma_alloc(void *pblock, size_t size, uint32_t flags);
DG_API void        *ma_allocdbg(void *pblock, size_t size, uint32_t flags, const char* pfile, int line);
DG_API int          ma_free(void* pblock);
DG_API int          ma_freedbg(void* pblock, const char* pfile, int line);

typedef struct ma_stats_s {
	size_t heaps;
	size_t total_bytes;
	size_t total_allocs;
} ma_stats_t;

DG_API int  ma_stats(ma_stats_t *pdst);

/* mem override */
#ifndef DG_MEMNOVERRIDE
#ifdef _DEBUG
#define malloc(size) (gpmmdt->mem_alloc_debug(NULL, size, DGMM_NONE, __FILE__, __LINE__))
#define calloc(count, size) (gpmmdt->mem_alloc_debug(NULL, size * count, DGMM_CLEAR, __FILE__, __LINE__))
#define realloc(ptr, size) (gpmmdt->mem_alloc_debug(ptr, size, DGMM_COPY, __FILE__, __LINE__))
#define free(ptr) (gpmmdt->mem_free_debug(ptr, __FILE__, __LINE__))
#else
#define malloc(size) (gpmmdt->mem_alloc(NULL, size, DGMM_NONE))
#define calloc(count, size) (gpmmdt->mem_alloc(NULL, size*count, DGMM_CLEAR))
#define realloc(ptr, size) (gpmmdt->mem_alloc(ptr, size, DGMM_COPY))
#define free(ptr) (gpmmdt->mem_free(ptr))
#endif
#endif

#define DG_NEW(type) ((type *)calloc(1, sizeof(type)))
#define DG_ALLOC(type, count) ((type *)calloc(count, sizeof(type)))
#define DG_FREE(addr) (free(addr))