#ifndef __dg_handle_h__
#define __dg_handle_h__
#include "dg_bitvec.h"

/**
* @brief Handle type
*/
typedef union dg_handle_u {
	uint64_t hvalue; /*< combined values */
	struct {
		uint32_t index; /*< index in the handle array */
		uint32_t gen; /*< generation counter to avoid stale handles */
	};
} dg_handle_t;

#define DG_HANDLE_MAGIC         (0x4C444E48) /*< 'HDNL' in little-endian - magic number for handle blocks */
#define DG_HANDLE_INVALID_INDEX (0xFFFFFFFF) /*< invalid index value */
#define DG_HANDLE_INVALID_GEN   (0xFFFFFFFF) /*< invalid generation value */
#define DG_HANDLE_INVALID_VALUE (0xFFFFFFFFFFFFFFFF) //low part: DG_HANDLE_INVALID_INDEX and high part: DG_HANDLE_INVALID_GEN

#define DG_INVALID_HANDLE ((dg_handle_t){ .hvalue = DG_HANDLE_INVALID_VALUE }) /*< invalid handle constant */

//typedef struct handle_block_s {
//	uint32_t magic;
//	uint32_t flags;
//	size_t   size;
//	uint8_t  data[];
//} handle_block_t;

/**
* @brief Handle allocator structure
*/
typedef struct dg_handle_alloc_s {
	size_t    blocksize; /*< size of each handle block */
	size_t    reserve;		/*< number of handles to reserve when growing */
	size_t    nhandles;	/*< total number of handles allocated */
	uint32_t* pgenerations; /*< array of generation counters */
	uint8_t* pblocks;			/*< array of handle blocks */
} dg_handle_alloc_t;

/**
* @brief Declare static data arrays for handle allocator
*/
#define HA_DECL_DATA_ARRAYS(name, count, elemsize) struct {\
	uint8_t blocks[count*elemsize]; \
	uint32_t generations[count]; \
} name

/**
* @brief creates new handle allocator
* @param pha - pointer to the handle allocator structure
* @param blocksize - size of each handle block
* @param nhandles - initial number of handles to allocate
* @param ngrow_reserve - number of handles to reserve when growing
* @return true on success, false on failure
*/
DG_API bool ha_init(dg_handle_alloc_t *pha,
	size_t blocksize,
	size_t nhandles,
	size_t ngrow_reserve);

/**
* @brief initializes handle allocator with static data arrays
* @param pha - pointer to the handle allocator structure
* @param blocksize - size of each handle block
* @param nhandles - total number of handles
* @param pblocks - pointer to the preallocated blocks array
* @param pgens - pointer to the preallocated generations array
* @return none
*/
DG_API void ha_init_static(dg_handle_alloc_t* pha,
	size_t blocksize,
	size_t nhandles,
	uint8_t *pblocks,
	uint32_t *pgens);

/**
* @brief deinitializes handle allocator and frees all allocated memory
* @param pha - pointer to the handle allocator structure
* @return none
*/
DG_API void ha_deinit(dg_handle_alloc_t* pha);

/**
* @brief Handle allocator info structure
*/
typedef struct dg_hinfo_s {
	size_t allocated; /*< number of currently allocated handles */
	size_t max_handles; /*< maximum number of handles*/
} dg_hinfo_t;

/**
* @brief retrieves handle allocator info
* @param pdst - pointer to the destination info structure
* @param pha - pointer to the handle allocator structure
* @return true on success, false on failure
*/
DG_API bool ha_get_info(dg_hinfo_t *pdst, dg_handle_alloc_t* pha);

/**
* @brief Handle allocation result structure
*/
typedef struct dg_halloc_result_s {
	dg_handle_t new_handle; /*< newly allocated handle */
	uint8_t* phandle_body; /*< pointer to the handle's data block */
	size_t   handle_body_size; /*< size of the handle's data block */
} dg_halloc_result_t;

/**
* @brief allocates a new handle
* @param pdst - pointer to the handle allocation result structure
* @param pha - pointer to the handle allocator structure
* @return true on success, false on failure
*/
DG_API bool ha_alloc_handle(dg_halloc_result_t *pdst, dg_handle_alloc_t* pha);

/**
* @brief frees a previously allocated handle
* @param pha - pointer to the handle allocator structure
* @param handle - handle to free
* @return true on success, false on failure (e.g., invalid handle or already freed)
*/
DG_API bool ha_free_handle(dg_handle_alloc_t* pha, dg_handle_t handle);

/**
* @brief checks if a handle is valid and currently allocated
* @param pha - pointer to the handle allocator structure
* @param handle - handle to check
* @return true if the handle is valid and allocated, false otherwise
*/
DG_API bool ha_is_valid_handle(dg_handle_alloc_t* pha, dg_handle_t handle);

/**
* @brief retrieves a handle by its index
* @param pdst - pointer to the destination handle
* @param pha - pointer to the handle allocator structure
* @param index - index of the handle to retrieve
* @return true on success, false on failure (e.g., index out of bounds)
*/
DG_API bool ha_get_handle_by_index(dg_handle_t *pdst, dg_handle_alloc_t* pha, size_t index);

/**
* @brief retrieves the data block associated with a handle
* @param pha - pointer to the handle allocator structure
* @param handle - handle whose data block to retrieve
* @return pointer to the handle's data block, or NULL if the handle is invalid or not allocated
*/
DG_API void* ha_get_handle_data(dg_handle_alloc_t* pha, dg_handle_t handle);

/**
* @brief retrieves the first handle in the allocator
* @param pha - pointer to the handle allocator structure
* @param start - starting index to search from
* @param is_free_only - if true, only free handles are considered
* @return the first found handle, or DG_INVALID_HANDLE if none found
*/
DG_API dg_handle_t ha_get_first_handle(dg_handle_alloc_t* pha, size_t start, bool is_free_only);

/**
* @brief retrieves the next handle in the allocator
* @param pdst - pointer to the current handle; updated to the next handle if found
* @param pha - pointer to the handle allocator structure
* @return true if the next handle is found and pdst is updated, false if no more handles are available
*/
DG_API bool ha_get_next_handle(dg_handle_t *pdst, dg_handle_alloc_t* pha);

static inline bool ha_is_empty_handle(dg_handle_t handle) {
	return handle.hvalue != DG_HANDLE_INVALID_VALUE;
}

#endif // __dg_handle_h__