#pragma once
#include "dg_libcommon.h"

/**
* memory allocation flags
*/
#define DGMM_NONE  (0)
#define DGMM_COPY  (1<<0)
#define DGMM_CLEAR (1<<1)

/**
* dg_alloc_dt_t
* memory allocation dispatch table
*/
typedef struct dg_memmgr_dt_s {
	void* (*mem_alloc)(void *poldmem, size_t size, uint32_t flags);
	int   (*mem_free)(void *pmem);
	void* (*mem_alloc_debug)(void* poldmem, size_t size, uint32_t flags, const char *pfile, int line);
	int   (*mem_free_debug)(void* pmem, const char* pfile, int line);
} dg_memmgr_dt_t;

extern dg_memmgr_dt_t *gpmmdt;

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

/**
* linear memory allocator
*/
#define DGLA_NONE (0)
#define DGLA_AUTORESIZE (1 << 0)
#define DGLA_LESSRESIZE (1 << 1)

typedef struct dg_linalloc_s {
	uint32_t flags;
	size_t   capacity;
	size_t   position;
	uint8_t* pdata;
} dg_hunkalloc_t;

#define la_init_struct(f) { .flags = f, .capacity=0, .position=0, .pdata=NULL }

bool  linalloc_init(dg_hunkalloc_t *pdst, size_t cap);
void  linalloc_deinit(dg_hunkalloc_t* pdst);
void* linalloc_hunk_alloc(dg_hunkalloc_t* pdst, size_t size);
void  linalloc_reset(dg_hunkalloc_t* pdst);
#define linalloc_is_present(p) ((p)->pdata)