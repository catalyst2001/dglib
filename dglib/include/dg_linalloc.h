/**
* Purpose: Linear allocator for each thread
* Author:  Deryabin K.
* Created: 12.07.2025
*/
#ifndef __dg_linalloc_h__
#define __dg_linalloc_h__
#include "dg_libcommon.h"

/**
* linear memory allocator
*/
#define DGLA_NONE (0) 			/*< no special flags */
#define DGLA_AUTORESIZE (1 << 0) /*< automatically resize if no space for allocation */
#define DGLA_LESSRESIZE (1 << 1) /*< resize to smaller size if possible */

/**
* linear memory allocator structure
*/
typedef struct dg_linalloc_s {
	uint32_t flags; 				/*< flags */
	size_t   capacity; 			/*< total capacity */
	size_t   position;	/*< current position */
	uint8_t* pdata;				/*< pointer to data */
} dg_hunkalloc_t;

/**
* macro to initialize linear allocator structure
* @param f - flags
*/
#define la_init_struct(f) { .flags = f, .capacity=0, .position=0, .pdata=NULL }

/**
* @brief Initialize linear allocator
* @param pdst - Pointer to the linear allocator structure
* @param cap - Initial capacity
* @return true on success, false on failure
*/
bool linalloc_init(dg_hunkalloc_t* pdst, size_t cap);

/**
* @brief Deinitialize linear allocator
* @param pdst - Pointer to the linear allocator structure
* @return void
*/
void linalloc_deinit(dg_hunkalloc_t* pdst);

/**
* @brief Allocate memory from linear allocator
* @param pdst - Pointer to the linear allocator structure
* @param size - Size of memory to allocate
* @return Pointer to allocated memory or NULL on failure
*/
void* linalloc_hunk_alloc(dg_hunkalloc_t* pdst, size_t size);

/**
* @brief Reset linear allocator position to zero
* @param pdst - Pointer to the linear allocator structure
* @return void
*/
void linalloc_reset(dg_hunkalloc_t* pdst);

/**
* @brief check if linear allocator is initialized
*/
#define linalloc_is_present(p) ((p)->pdata)

/**
* @brief checking linear allocator for existence for current thread
* @return true if initialized, false otherwise
*/
DG_API bool la_present();

/**
* @brief initialize linear allocator for current thread
* @param initialsize - initial size of the allocator
* @return true on success, false on failure
*/
DG_API bool la_resize(size_t newsize);

/**
* @brief get current capacity of the linear allocator
* @return capacity in bytes, 0 if not initialized
* 
* @note if not initialized, returns 0
*/
DG_API size_t la_get_capacity();

/**
* @brief get current position of the linear allocator
* @return position in bytes, 0 if not initialized
* 
* @note if not initialized, returns 0
*/
DG_API size_t la_get_position();

/**
* @brief allocate memory from linear allocator for current thread
* @param size - size of memory to allocate
* @param bclear - if true, allocated memory will be zeroed
* @return pointer to allocated memory or NULL on failure
* 
* @note if allocator is not initialized, returns NULL
*/
DG_API void* la_hunk_alloc(size_t size, bool bclear);

/**
* @brief reset linear allocator position for current thread
* @return void
*/
DG_API void la_reset();

#endif // __dg_linalloc_h__