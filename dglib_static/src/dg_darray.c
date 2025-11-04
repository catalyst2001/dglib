#include "dg_darray.h"
#include "dg_alloc.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define darray_get_internal(pd, idx) ((pd)->pdata + (idx) * (pd)->elemsize)

static bool try_ensure_capacity(dg_darray_t* pd, size_t newcap)
{
  if (!pd->pdata || newcap > pd->capacity) {
    if(newcap > pd->capacity)
      pd->capacity = newcap + pd->reserve;

    uint8_t* tmp = realloc(pd->pdata, pd->capacity * pd->elemsize); //FIXME: K.D. realloc freeing old memory!
    if (!tmp)
      return false;
    
    pd->pdata = tmp;
  }
  return true;
}

void* darray_get_ex(dg_darray_t* pd, size_t idx)
{
  assert(pd && "pd is NULL");
  assert(pd->elemsize > 0 && "element size is zero");
  assert(idx < pd->size && "idx out of bounds");
  return darray_get_internal(pd, idx);
}

int darray_resize(dg_darray_t* pd, size_t newsize)
{
  assert(pd && "pd is NULL");
  assert(pd->elemsize > 0 && "element size is zero");
  if (newsize > pd->capacity) {
    int st = darray_realloc(pd, newsize);
    if (st != DGERR_SUCCESS)
      return st;
  }
  pd->size = newsize;
  return DGERR_SUCCESS;
}

int darray_realloc(dg_darray_t* pd, size_t newcap) {
  assert(pd && "pd is NULL");
  assert(pd->elemsize > 0 && "element size is zero");
  if (!try_ensure_capacity(pd, newcap))
    return DGERR_OUT_OF_MEMORY;

  return DGERR_SUCCESS;
}

int darray_shrink_to_fit(dg_darray_t* pd)
{
  assert(pd && "pd is NULL");
  assert(pd->elemsize > 0 && "element size is zero");
  if (pd->capacity > pd->size)
    return darray_realloc(pd, pd->size);

  return DGERR_SUCCESS;
}

void darray_move(dg_darray_t* dst, dg_darray_t* src)
{
  assert(src && "pdarray is NULL");
  assert(src->elemsize > 0 && "element size is zero");
  dst->capacity = src->capacity;
  dst->elemsize = src->elemsize;
  dst->pdata = src->pdata;
  dst->reserve = src->reserve;
  dst->size = src->size;
  darray_reset(src);
}

int darray_copy(dg_darray_t* dst, const dg_darray_t* src)
{
  assert(src && "pdarray is NULL");
  assert(src->elemsize > 0 && "element size is zero");
  if (src->capacity == 0) 
   return DGERR_SUCCESS;

  dst->elemsize = src->elemsize;
  dst->reserve = src->reserve;
  dst->capacity = src->capacity;
  dst->size = src->size;
  dst->pdata = malloc(src->capacity * src->elemsize);
  if (!dst->pdata) 
     return DGERR_OUT_OF_MEMORY;

  memcpy(dst->pdata, src->pdata, src->capacity * src->elemsize);
  return DGERR_SUCCESS;
}

void darray_remove_from(dg_darray_t* pdarray, size_t idxfrom)
{
  assert(pdarray && "pdarray is NULL");
  assert(pdarray->elemsize > 0 && "element size is zero");
  if (!pdarray->pdata || !pdarray->size || pdarray->elemsize==0)
    return;

  /* element in begin or mid in array? */
  if ((idxfrom + 1) < pdarray->size) {
    memmove(
      darray_get_internal(pdarray, idxfrom),
      darray_get_internal(pdarray, idxfrom + 1),
      pdarray->elemsize
    );
    pdarray->size--;
  }
  else {
    /*
    element in the back
    decrement size
    */
    pdarray->size--;
  }
}

void darray_remove_from_fast(dg_darray_t* pdarray, size_t idxfrom)
{
  assert(pdarray && "pdarray is NULL");
  assert(pdarray->elemsize > 0 && "element size is zero");
  if (!pdarray->pdata || pdarray->size || pdarray->elemsize == 0)
    return;

  /* element in begin or mid in array? */
  if (idxfrom < pdarray->size) {
    memcpy(
      darray_get_internal(pdarray, idxfrom),
      darray_get_internal(pdarray, pdarray->size - 1),
      pdarray->elemsize
    );
    pdarray->size--;
  }
  else {
    /*
    element in the back
    decrement size
    */
    pdarray->size--;
  }
}

void darray_insert_from(dg_darray_t* pd, const dg_darray_t* ps, size_t from, size_t count)
{
  assert(ps && pd);
  assert(pd->elemsize > 0 && "element size is zero");
  assert(from + count <= ps->size && "source range out of bounds");
  if (!try_ensure_capacity(pd, pd->size + count)) {
    return;
  }
  memcpy(darray_get_internal(pd, pd->size),
    ps->pdata + from * ps->elemsize,
    count * pd->elemsize);
  pd->size += count;
}

void darray_clear(dg_darray_t* pd)
{
  assert(pd && "pd is NULL");
  assert(pd->elemsize > 0 && "element size is zero");
  pd->size = 0;
}

void darray_reset(dg_darray_t* pd)
{
  assert(pd && "pd is NULL");
  assert(pd->elemsize > 0 && "element size is zero");
  pd->capacity = 0;
  pd->size = 0;
  pd->pdata = NULL;
}

bool darray_free(dg_darray_t* pd)
{
  assert(pd && "pd is NULL");
  assert(pd->elemsize > 0 && "element size is zero");
  if (pd->pdata) {
    free(pd->pdata);
    pd->pdata = NULL;
    return true;
  }
  return false;
}

void* darray_add_back(dg_darray_t* pd)
{
	//FIXME: K.D. remove this old code after testing
#if 0
  void* ptr;
	assert(pd && "pd is NULL");
	assert(pd->elemsize > 0 && "element size is zero");
  if (!try_ensure_capacity(pd, pd->size + 1))
    return NULL;

  ptr = darray_get_internal(pd, pd->size);
  pd->size++;
  return ptr;
#endif
	return darray_add_back_multiple(pd, 1);
}

void* darray_add_back_multiple(dg_darray_t* pd, size_t count)
{
  void* ptr;
  assert(pd && "pd is NULL");
  assert(pd->elemsize > 0 && "element size is zero");
  if (!try_ensure_capacity(pd, pd->size + count))
    return NULL;

  ptr = darray_get_internal(pd, pd->size);
  pd->size += count;
  return ptr;
}

bool darray_push_back(dg_darray_t* pd, const void* src)
{
  void* pelem = darray_add_back(pd);
  if (pelem) {
    memcpy(pelem, src, pd->elemsize);
    return true;
  }
  return false;
}

void* darray_add_front(dg_darray_t* pd)
{
  if (!try_ensure_capacity(pd, pd->size + 1))
    return NULL;

  memmove(pd->pdata + pd->elemsize,
    pd->pdata,
    pd->size * pd->elemsize);

  pd->size++;
  return pd->pdata;
}

bool darray_push_front(dg_darray_t* pd, const void* src)
{
  void* pelem = darray_add_front(pd);
  if (pelem) {
    memcpy(pd->pdata, src, pd->elemsize);
    return true;
  }
  return false;
}

bool darray_pop_back(void* dst, dg_darray_t* pd)
{
  if (pd->size == 0) return false;
  pd->size--;
  memcpy(dst,
    darray_get_internal(pd, pd->size),
    pd->elemsize);
  return true;
}

bool darray_pop_front(void* dst, dg_darray_t* pd)
{
  if (pd->size == 0) return false;
  memcpy(dst, pd->pdata, pd->elemsize);
  memmove(pd->pdata,
    pd->pdata + pd->elemsize,
    (pd->size - 1) * pd->elemsize);
  pd->size--;
  return true;
}

int darray_shift(dg_darray_t* pd, size_t from, size_t to, size_t count)
{
  assert(from + count <= pd->size && "shift range out of bounds");
  assert((to + count) <= pd->capacity && "shift target out of bounds");
  memmove(
    pd->pdata + to * pd->elemsize,
    pd->pdata + from * pd->elemsize,
    count * pd->elemsize
  );
  return DGERR_SUCCESS;
}

