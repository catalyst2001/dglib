#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "dg_vector.h"

#define DG_MEMNOVERRIDE
#include "dg_alloc.h"

/* default impl */
void* mem_alloc_default(void* poldmem, size_t size, uint32_t flags) {
	void* ptr;
	if (!size)
		return NULL;

	ptr = malloc(size);
	if (!ptr)
		return NULL;

	if (flags & DGMM_CLEAR)
		memset(ptr, 0, size);

	if (poldmem && (flags & DGMM_COPY))
		memcpy(ptr, poldmem, size);

	return ptr;
}

int mem_free_default(void* pmem) {
	if (pmem) {
		free(pmem);
		return DGERR_NONE;
	}
	return DGERR_INVALID_PARAM;
}

void* mem_alloc_debug_default(void* poldmem, size_t size, uint32_t flags, const char* pfile, int line) {
	void* ptr = mem_alloc_default(poldmem, size, flags);
	if (!ptr) {
		fprintf(stderr, "mem_alloc_debug_default(): allocation failed! File: %s Line: %d\n", pfile, line);
		return NULL;
	}
	return ptr;
}

int mem_free_debug_default(void* pmem, const char* pfile, int line) {
	int status = mem_free_default(pmem);
	if (status != DGERR_NONE)
		fprintf(stderr, "mem_free_debug_default(): free failed! status: %d  File: %d  Line: %d\n", 
			status, pfile, line);

	return status;
}

dg_memmgr_dt_t dt = {
	.mem_alloc = mem_alloc_default,
	.mem_alloc_debug = mem_alloc_debug_default,
	.mem_free = mem_free_default,
	.mem_free_debug = mem_free_debug_default
};
dg_memmgr_dt_t* gpmmdt = &dt;

dg_da_t dynarray = dg_da_init(int, 100, 100, 0);

int main()
{
	int value=1;
	for(int i = 0; i < 10; i++)
		dg_da_push_back(&dynarray, &value);

	for (int i = 0; i < dg_da_get_size(&dynarray); i++) {
		printf("[%d] %d\n", i, dg_da_get(&dynarray, i, int));
	}
	dg_da_free(&dynarray);
	return 0;
}