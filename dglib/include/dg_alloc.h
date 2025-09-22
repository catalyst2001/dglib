#pragma once
#include "dg_libcommon.h"

/**
* memory allocation flags
*/
#define DGMM_NONE  (0) 			/*< no special flags */
#define DGMM_COPY  (1<<0) 	/*< copy old memory block to new one */
#define DGMM_CLEAR (1<<1) /*< clear allocated memory block */

/**
* dg_memmgr_dt_t
* memory allocation dispatch table
*/
typedef struct dg_memmgr_dt_s {
	/**
	* @brief memory allocation function
	* @param poldmem - pointer to old memory block, can be NULL
	* @param size - size of memory block to allocate
	* @param flags - allocation flags (DGMM_*)
	* @return pointer to allocated memory block or NULL on failure
	*/
	void* (*mem_alloc)(void *poldmem, size_t size, uint32_t flags);

	/**
	* @brief memory free function
	* @param pmem - pointer to memory block to free
	* @return 0 on success, -1 on failure
	*/
	int (*mem_free)(void *pmem);

	/**
	* @brief debug memory allocation function
	* @param poldmem - pointer to old memory block, can be NULL
	* @param size - size of memory block to allocate
	* @param flags - allocation flags (DGMM_*)
	* @param pfile - file name where allocation is called
	* @param line - line number where allocation is called
	* @return pointer to allocated memory block or NULL on failure
	*/
	void* (*mem_alloc_debug)(void* poldmem, size_t size, uint32_t flags, const char *pfile, int line);
	
	/**
	* @brief debug memory free function
	* @param pmem - pointer to memory block to free
	* @param pfile - file name where free is called
	*	@param line - line number where free is called
	* @return 0 on success, -1 on failure
	*/
	int (*mem_free_debug)(void* pmem, const char* pfile, int line);
} dg_memmgr_dt_t;

/**
* @brief get memory allocation dispatch table
* @return pointer to memory allocation dispatch table (for custom allocators)
*/
DG_API dg_memmgr_dt_t* mm_get_dt();

typedef dg_voidptr_t dg_ma_heap_t; /*< memory allocation heap handle */

/**
* @brief memory heap information structure
*/
typedef struct ma_heap_info_s {
	size_t heap_size; /*< total heap size */
	size_t heap_reserve; /*< reserved heap size */
	size_t min_address; /*< minimum address for the heap */
	size_t max_address; /*< maximum address for the heap */
} ma_heap_info_t;

/**
* @brief create memory allocation heap
* @param pdst - pointer to heap handle to be created
* @param pheapinfo - pointer to heap information structure
* @param pname - name of the heap
* @return 0 on success, -1 on failure
*/
DG_API int ma_create_heap(dg_ma_heap_t *pdst, const ma_heap_info_t *pheapinfo, const char *pname);

/**
* @brief find memory allocation heap by name
* @param pname - name of the heap to find
* @return handle to the found heap or NULL if not found
* 
* @note if pname is NULL, returns the process default heap
* @note the returned heap handle should not be destroyed with ma_destroy_heap()
*/
DG_API dg_ma_heap_t ma_find_heap(const char* pname);

/**
* @brief destroy memory allocation heap
* @param hheap - handle to the heap to destroy
* @return 0 on success, -1 on failure
*/
DG_API int ma_destroy_heap(dg_ma_heap_t hheap);

/**
* @brief set the memory allocation heap for the current thread
* @param hheap - handle to the heap to set
* @return 0 on success, -1 on failure
*/
DG_API int ma_thread_set_heap(dg_ma_heap_t hheap);

/**
* @brief get the process default memory allocation heap
* @return handle to the process default heap
* 
*	@note the returned heap handle should not be destroyed with ma_destroy_heap()
*/
DG_API dg_ma_heap_t ma_get_process_heap();

/**
* @brief get the memory allocation heap for the current thread
* @return handle to the current thread's heap
* 
* @note if the thread has no specific heap set, returns NULL
*/
DG_API dg_ma_heap_t ma_thread_get_heap();

/**
* @brief allocate memory block
* @param pblock - pointer to old memory block, can be NULL
* @param size - size of memory block to allocate
* @param flags - allocation flags (DGMM_*)
* @return pointer to allocated memory block or NULL on failure
*/
DG_API void *ma_alloc(void *pblock, size_t size, uint32_t flags);

/**
* @brief allocate memory block with debug information
* @param pblock - pointer to old memory block, can be NULL
* @param size - size of memory block to allocate
* @param flags - allocation flags (DGMM_*)
* @param pfile - file name where allocation is called
* @param line - line number where allocation is called
* @return pointer to allocated memory block or NULL on failure
*/
DG_API void *ma_allocdbg(void *pblock, size_t size, uint32_t flags, const char* pfile, int line);

/**
*	@brief free memory block
* @param pblock - pointer to memory block to free
* @return 0 on success, -1 on failure
*/
DG_API int ma_free(void* pblock);

/**
* @brief free memory block with debug information
* @param pblock - pointer to memory block to free
* @param pfile - file name where free is called
* @param line - line number where free is called
* @return 0 on success, -1 on failure
*/
DG_API int ma_freedbg(void* pblock, const char* pfile, int line);

/**
* @brief memory allocation statistics structure
*/
typedef struct ma_stats_s {
	size_t heaps; /*< number of heaps */
	size_t total_bytes; /*< total allocated bytes */
	size_t total_allocs; /*< total number of allocations */
} ma_stats_t;

/**
* @brief get memory allocation statistics
* @param pdst - pointer to statistics structure to fill
* @return 0 on success, -1 on failure
*/
DG_API int  ma_stats(ma_stats_t *pdst);

/* mem override */
#ifndef DG_MEMNOVERRIDE
#ifdef _DEBUG
#define malloc(size) (ma_allocdbg(NULL, size, DGMM_NONE, __FILE__, __LINE__))
#define calloc(count, size) (ma_allocdbg(NULL, size * count, DGMM_CLEAR, __FILE__, __LINE__))
#define realloc(ptr, size) (ma_allocdbg(ptr, size, DGMM_COPY, __FILE__, __LINE__))
#define free(ptr) (ma_freedbg(ptr, __FILE__, __LINE__))
#else
#define malloc(size) (ma_alloc(NULL, size, DGMM_NONE))
#define calloc(count, size) (ma_alloc(NULL, size*count, DGMM_CLEAR))
#define realloc(ptr, size) (ma_alloc(ptr, size, DGMM_COPY))
#define free(ptr) (ma_free(ptr))
#endif
#endif

#define DG_NEW(type) ((type *)calloc(1, sizeof(type)))
#define DG_ALLOC(type, count) ((type *)calloc(count, sizeof(type)))
#define DG_FREE(addr) (free(addr))