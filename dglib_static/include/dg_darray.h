#pragma once
#include "dg_libcommon.h"

typedef struct dg_darray_s {
  size_t   elemsize;
  size_t   capacity;
  size_t   reserve;
  size_t   size;
  uint8_t* pdata;
} dg_darray_t;

#define darray_init_default(type) {\
    .elemsize = sizeof(type), \
    .capacity = 1, \
    .reserve  = 1, \
    .size     = 0, \
    .pdata    = NULL \
}

#define darray_init(type, cap, res, siz) {\
    .elemsize = sizeof(type), \
    .capacity = cap, \
    .reserve  = res, \
    .size     = siz, \
    .pdata    = NULL \
}

#define darray_get_size(p)            ((p)->size)
#define darray_get_capacity(p)       ((p)->capacity)
#define darray_get_reserve(p)        ((p)->reserve)
#define darray_get_elemsize(p)       ((p)->elemsize)
#define darray_set_reserve(p, newr)  assert((newr)!=0), (p)->reserve = (newr)

void* darray_get_ex(dg_darray_t* pdarray, size_t idx);
#define darray_get(p, idx, type)       *((type*)darray_get_ex(p, idx))
#define darray_getptr(p, idx, type)    ((type*)darray_get_ex(p, idx))
#define darray_get_data_ptr_as(p, type)((type*)(p)->pdata)

int     darray_resize(dg_darray_t* pdarray, size_t newsize);
int     darray_realloc(dg_darray_t* pdarray, size_t newcap);
int     darray_shrink_to_fit(dg_darray_t* pdarray);
void    darray_move(dg_darray_t* pdst, dg_darray_t* psrc);
int     darray_copy(dg_darray_t* pdst, const dg_darray_t* psrc);

void    darray_insert_from(dg_darray_t* pdarray, const dg_darray_t* pdarraysrc, size_t from, size_t count);

void    darray_clear(dg_darray_t* pdarray);
void    darray_reset(dg_darray_t* pdarray);
bool    darray_free(dg_darray_t* pdarray);

void*   darray_add_back(dg_darray_t* pdarray);
bool    darray_push_back(dg_darray_t* pdarray, const void* psrc);
void*   darray_add_front(dg_darray_t* pdarray);
bool    darray_push_front(dg_darray_t* pdarray, const void* psrc);
bool    darray_pop_back(void* pdst, dg_darray_t* pdarray);
bool    darray_pop_front(void* pdst, dg_darray_t* pdarray);

int     darray_shift(dg_darray_t* pdarray, size_t from, size_t to, size_t count);
static inline size_t darray_get_remaining_size(const dg_darray_t* pd) {
  assert(pd->size <= pd->capacity && "size greater than capacity");
  return pd->capacity - pd->size;
}
#define darray_is_empty(p) (!((p)->size))

/*-------------------------------------------
 2D array
-------------------------------------------*/
typedef struct dg_darray2d_s {
  size_t rows, cols;
  size_t elemsize;
  uint8_t* pdata;
} dg_darray2d_t;

#define darray2d_init(_rows,_cols,type) { \
    .rows = (_rows), .cols = (_cols),      \
    .elemsize = sizeof(type),      \
    .pdata = NULL                  \
}

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

static inline bool darray2d_free(dg_darray2d_t* pd) {
  if (!pd->pdata)
    return false;

  free(pd->pdata);
  pd->pdata = NULL;
  return true;
}

static inline void* darray2d_get_ex(
  const dg_darray2d_t* pd,
  size_t i,
  size_t j)
{
  assert(pd && pd->pdata);
  assert(i < pd->rows && j < pd->cols);
  return pd->pdata + ((i * pd->cols) + j) * pd->elemsize;
}
#define darray2d_get(pd,i,j,type) (*((type*)darray2d_get_ex((pd),(i),(j))))

/*-------------------------------------------
 3D array
-------------------------------------------*/
typedef struct dg_darray3d_s {
  size_t dim0, dim1, dim2;
  size_t elemsize;
  uint8_t* pdata;
} dg_darray3d_t;

#define darray3d_init(d0,d1,d2,type) { \
    .dim0 = (d0), .dim1 = (d1), .dim2 = (d2), \
    .elemsize = sizeof(type),                   \
    .pdata = NULL                               \
}

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

static inline bool darray3d_free(dg_darray3d_t* pd) {
  if (!pd->pdata)
    return false;

  free(pd->pdata);
  pd->pdata = NULL;
  return true;
}

static inline void* darray3d_get_ex(
  const dg_darray3d_t* pd,
  size_t i, size_t j, size_t k) {
  assert(pd && pd->pdata);
  assert(i < pd->dim0 && j < pd->dim1 && k < pd->dim2);
  size_t idx = ((i * pd->dim1 + j) * pd->dim2 + k);
  return pd->pdata + idx * pd->elemsize;
}

#define darray3d_get(pd,i,j,k,type) (*((type*)darray3d_get_ex((pd),(i),(j),(k))))