#pragma once
#include "dg_alloc.h"

/**
 * @brief Dynamic array structure (1D)
 * @details Stores metadata and pointer to data for a dynamic array.
 */
typedef struct dg_darray_s {
  size_t   elemsize;   /**< Size of one element in bytes */
  size_t   capacity;   /**< Allocated capacity (number of elements) */
  size_t   reserve;    /**< Reserved minimum capacity */
  size_t   size;       /**< Current number of elements */
  uint8_t* pdata;      /**< Pointer to data buffer */
} dg_darray_t;

/**
 * @brief Default initializer for dynamic array
 * @param type Element type
 */
#define darray_init_default(type) {\
    .elemsize = sizeof(type), \
    .capacity = 1, \
    .reserve  = 1, \
    .size     = 0, \
    .pdata    = NULL \
}

/**
 * @brief Custom initializer for dynamic array
 * @param type Element type
 * @param cap Initial capacity
 * @param res Reserved minimum capacity
 * @param siz Initial size
 */
#define darray_init(type, cap, res, siz) {\
    .elemsize = sizeof(type), \
    .capacity = cap, \
    .reserve  = res, \
    .size     = siz, \
    .pdata    = NULL \
}

/** @name darray accessors */
///@{
#define darray_get_size(p)            ((p)->size)           /**< Get current size */
#define darray_get_capacity(p)       ((p)->capacity)        /**< Get current capacity */
#define darray_get_reserve(p)        ((p)->reserve)         /**< Get reserved capacity */
#define darray_get_elemsize(p)       ((p)->elemsize)        /**< Get element size */
#define darray_set_reserve(p, newr)  assert((newr)!=0), (p)->reserve = (newr) /**< Set reserved capacity */
///@}

/**
 * @brief Get pointer to element at index (internal)
 * @param pdarray Pointer to array
 * @param idx Index
 * @return Pointer to element
 */
void* darray_get_ex(dg_darray_t* pdarray, size_t idx);

/**
 * @brief Get element at index (typed)
 * @param p Pointer to array
 * @param idx Index
 * @param type Element type
 */
#define darray_get(p, idx, type)       *((type*)darray_get_ex(p, idx))
/**
 * @brief Get pointer to element at index (typed)
 * @param p Pointer to array
 * @param idx Index
 * @param type Element type
 */
#define darray_getptr(p, idx, type)    ((type*)darray_get_ex(p, idx))
/**
 * @brief Get pointer to data as type
 * @param p Pointer to array
 * @param type Element type
 */
#define darray_get_data_ptr_as(p, type)((type*)(p)->pdata)

/**
 * @brief Resize array to new size
 * @param pdarray Pointer to array
 * @param newsize New size
 * @return 0 on success, -1 on failure
 */
int     darray_resize(dg_darray_t* pdarray, size_t newsize);
/**
 * @brief Reallocate array to new capacity
 * @param pdarray Pointer to array
 * @param newcap New capacity
 * @return 0 on success, -1 on failure
 */
int     darray_realloc(dg_darray_t* pdarray, size_t newcap);
/**
 * @brief Shrink array capacity to fit size
 * @param pdarray Pointer to array
 * @return 0 on success, -1 on failure
 */
int     darray_shrink_to_fit(dg_darray_t* pdarray);
/**
 * @brief Move array contents to another array
 * @param pdst Destination array
 * @param psrc Source array
 */
void    darray_move(dg_darray_t* pdst, dg_darray_t* psrc);
/**
 * @brief Copy array contents to another array
 * @param pdst Destination array
 * @param psrc Source array
 * @return 0 on success, -1 on failure
 */
int     darray_copy(dg_darray_t* pdst, const dg_darray_t* psrc);

/**
 * @brief Remove elements from index to end
 * @param pdarray Pointer to array
 * @param idxfrom Index to start removing
 */
void    darray_remove_from(dg_darray_t* pdarray, size_t idxfrom);
/**
 * @brief Remove elements from index to end (fast, does not preserve order)
 * @param pdarray Pointer to array
 * @param idxfrom Index to start removing
 */
void    darray_remove_from_fast(dg_darray_t* pdarray, size_t idxfrom);

/**
 * @brief Insert elements from another array
 * @param pdarray Destination array
 * @param pdarraysrc Source array
 * @param from Source start index
 * @param count Number of elements
 */
void    darray_insert_from(dg_darray_t* pdarray, const dg_darray_t* pdarraysrc, size_t from, size_t count);

/**
 * @brief Clear array (set size to 0, keep capacity)
 * @param pdarray Pointer to array
 */
void    darray_clear(dg_darray_t* pdarray);
/**
 * @brief Reset array (set size and capacity to 0, free data)
 * @param pdarray Pointer to array
 */
void    darray_reset(dg_darray_t* pdarray);
/**
 * @brief Free array data
 * @param pdarray Pointer to array
 * @return true if freed, false if already NULL
 */
bool    darray_free(dg_darray_t* pdarray);

/**
 * @brief Add element to back, return pointer to new element
 * @param pdarray Pointer to array
 * @return Pointer to new element or NULL on failure
 */
void*   darray_add_back(dg_darray_t* pdarray);

/**
 * @brief Add multiple elements to back, return pointer to new elements
 * @param pdarray Pointer to array
 * @param count Number of elements to add
 * @return Pointer to new element or NULL on failure
 */
void* darray_add_back_multiple(dg_darray_t* pdarray, size_t count);

/**
 * @brief Push element to back (copy from src)
 * @param pdarray Pointer to array
 * @param psrc Pointer to source element
 * @return true on success, false on failure
 */
bool    darray_push_back(dg_darray_t* pdarray, const void* psrc);
/**
 * @brief Add element to front, return pointer to new element
 * @param pdarray Pointer to array
 * @return Pointer to new element or NULL on failure
 */
void*   darray_add_front(dg_darray_t* pdarray);
/**
 * @brief Push element to front (copy from src)
 * @param pdarray Pointer to array
 * @param psrc Pointer to source element
 * @return true on success, false on failure
 */
bool    darray_push_front(dg_darray_t* pdarray, const void* psrc);
/**
 * @brief Pop element from back
 * @param pdst Pointer to destination
 * @param pdarray Pointer to array
 * @return true on success, false if empty
 */
bool    darray_pop_back(void* pdst, dg_darray_t* pdarray);
/**
 * @brief Pop element from front
 * @param pdst Pointer to destination
 * @param pdarray Pointer to array
 * @return true on success, false if empty
 */
bool    darray_pop_front(void* pdst, dg_darray_t* pdarray);

/**
 * @brief Shift elements in array
 * @param pdarray Pointer to array
 * @param from Source index
 * @param to Destination index
 * @param count Number of elements
 * @return 0 on success, -1 on failure
 */
int     darray_shift(dg_darray_t* pdarray, size_t from, size_t to, size_t count);

/**
 * @brief Get remaining capacity (capacity - size)
 * @param pd Pointer to array
 * @return Remaining capacity
 */
static inline size_t darray_get_remaining_size(const dg_darray_t* pd) {
  assert(pd->size <= pd->capacity && "size greater than capacity");
  return pd->capacity - pd->size;
}

/**
 * @brief Check if array is empty
 * @param p Pointer to array
 * @return true if empty, false otherwise
 */
#define darray_is_empty(p) (!((p)->size))

/** @defgroup darray2d 2D dynamic array
 *  @{
 */
/**
 * @brief 2D dynamic array structure
 */
typedef struct dg_darray2d_s {
  size_t rows, cols;      /**< Number of rows and columns */
  size_t elemsize;        /**< Size of one element in bytes */
  uint8_t* pdata;         /**< Pointer to data buffer */
} dg_darray2d_t;

/**
 * @brief 2D array initializer
 * @param _rows Number of rows
 * @param _cols Number of columns
 * @param type Element type
 */
#define darray2d_init(_rows,_cols,type) { \
    .rows = (_rows), .cols = (_cols),      \
    .elemsize = sizeof(type),      \
    .pdata = NULL                  \
}

/**
 * @brief Allocate memory for 2D array
 * @param pd Pointer to 2D array
 * @param clear If true, zero memory
 * @return true on success, false on failure
 */
static inline bool darray2d_alloc(dg_darray2d_t* pd, bool clear) {
  assert(pd);
  size_t total = pd->rows * pd->cols;
  pd->pdata = malloc(total * pd->elemsize);
  if (!pd->pdata)
    return false;

  if (clear)
    memset(pd->pdata, 0, total * pd->elemsize);

  return true;
}

/**
 * @brief Free memory for 2D array
 * @param pd Pointer to 2D array
 * @return true if freed, false if already NULL
 */
static inline bool darray2d_free(dg_darray2d_t* pd) {
  if (!pd->pdata)
    return false;

  free(pd->pdata);
  pd->pdata = NULL;
  return true;
}

/**
 * @brief Get pointer to element at (i, j)
 * @param pd Pointer to 2D array
 * @param i Row index
 * @param j Column index
 * @return Pointer to element
 */
static inline void* darray2d_get_ex(
  const dg_darray2d_t* pd,
  size_t i,
  size_t j)
{
  assert(pd && pd->pdata);
  assert(i < pd->rows && j < pd->cols);
  return pd->pdata + ((i * pd->cols) + j) * pd->elemsize;
}
/**
 * @brief Get element at (i, j) (typed)
 * @param pd Pointer to 2D array
 * @param i Row index
 * @param j Column index
 * @param type Element type
 */
#define darray2d_get(pd,i,j,type) (*((type*)darray2d_get_ex((pd),(i),(j))))
/** @} */

/** @defgroup darray3d 3D dynamic array
 *  @{
 */
/**
 * @brief 3D dynamic array structure
 */
typedef struct dg_darray3d_s {
  size_t dim0, dim1, dim2; /**< Dimensions */
  size_t elemsize;         /**< Size of one element in bytes */
  uint8_t* pdata;          /**< Pointer to data buffer */
} dg_darray3d_t;

/**
 * @brief 3D array initializer
 * @param d0 First dimension
 * @param d1 Second dimension
 * @param d2 Third dimension
 * @param type Element type
 */
#define darray3d_init(d0,d1,d2,type) { \
    .dim0 = (d0), .dim1 = (d1), .dim2 = (d2), \
    .elemsize = sizeof(type),                   \
    .pdata = NULL                               \
}

/**
 * @brief Allocate memory for 3D array
 * @param pd Pointer to 3D array
 * @param clear If true, zero memory
 * @return true on success, false on failure
 */
static inline bool darray3d_alloc(dg_darray3d_t* pd, bool clear) {
  assert(pd);
  size_t total = pd->dim0 * pd->dim1 * pd->dim2;
  pd->pdata = malloc(total * pd->elemsize);
  if (!pd->pdata)
    return false;

  if (clear)
    memset(pd->pdata, 0, total * pd->elemsize);

  return true;
}

/**
 * @brief Free memory for 3D array
 * @param pd Pointer to 3D array
 * @return true if freed, false if already NULL
 */
static inline bool darray3d_free(dg_darray3d_t* pd) {
  if (!pd->pdata)
    return false;

  free(pd->pdata);
  pd->pdata = NULL;
  return true;
}

/**
 * @brief Get pointer to element at (i, j, k)
 * @param pd Pointer to 3D array
 * @param i First index
 * @param j Second index
 * @param k Third index
 * @return Pointer to element
 */
static inline void* darray3d_get_ex(
  const dg_darray3d_t* pd,
  size_t i, size_t j, size_t k) {
  assert(pd && pd->pdata);
  assert(i < pd->dim0 && j < pd->dim1 && k < pd->dim2);
  size_t idx = ((i * pd->dim1 + j) * pd->dim2 + k);
  return pd->pdata + idx * pd->elemsize;
}
/**
 * @brief Get element at (i, j, k) (typed)
 * @param pd Pointer to 3D array
 * @param i First index
 * @param j Second index
 * @param k Third index
 * @param type Element type
 */
#define darray3d_get(pd,i,j,k,type) (*((type*)darray3d_get_ex((pd),(i),(j),(k))))
/** @} */

/**
 * @brief Declare a dynamic array type for a specific element type
 * @param name Name of the new type
 * @param type Element type
 */
#define DARRAY_DECL(name, type)\
typedef struct name##_s {\
  size_t size;\
  size_t cap;\
  type *pdata;\
} name##_t;

//TODO: K.D. continue