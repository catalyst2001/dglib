#include "dg_darray.h"
#include "dg_alloc.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define darray_get_internal(pd, idx) ((pd)->pdata + (idx) * (pd)->elemsize)

static bool try_ensure_capacity(darray_t* pd, size_t newcap)
{
  if (!pd->pdata || newcap > pd->capacity) {
    pd->capacity = newcap + pd->reserve;
    uint8_t* tmp = realloc(pd->pdata, pd->capacity * pd->elemsize);
    if (!tmp) {
      return false;
    }
    pd->pdata = tmp;
  }
  return true;
}

void* darray_get_ex(darray_t* pd, size_t idx)
{
  assert(idx < pd->size && "idx out of bounds");
  return darray_get_internal(pd, idx);
}

int darray_resize(darray_t* pd, size_t newsize)
{
  if (newsize > pd->capacity) {
    int st = darray_realloc(pd, newsize);
    if (st != DGERR_NONE) return st;
  }
  pd->size = newsize;
  return DGERR_NONE;
}

int darray_realloc(darray_t* pd, size_t newcap) {
  if (!try_ensure_capacity(pd, newcap))
    return DGERR_OUT_OF_MEMORY;

  return DGERR_NONE;
}

int darray_shrink_to_fit(darray_t* pd)
{
  if (pd->capacity > pd->size)
    return darray_realloc(pd, pd->size);

  return DGERR_NONE;
}

void darray_move(darray_t* dst, darray_t* src)
{
  dst->capacity = src->capacity;
  dst->elemsize = src->elemsize;
  dst->pdata = src->pdata;
  dst->reserve = src->reserve;
  dst->size = src->size;
  darray_reset(src);
}

int darray_copy(darray_t* dst, const darray_t* src)
{
  if (src->capacity == 0) return DGERR_NONE;
  dst->elemsize = src->elemsize;
  dst->reserve = src->reserve;
  dst->capacity = src->capacity;
  dst->size = src->size;
  dst->pdata = malloc(src->capacity * src->elemsize);
  if (!dst->pdata) return DGERR_OUT_OF_MEMORY;
  memcpy(dst->pdata, src->pdata, src->capacity * src->elemsize);
  return DGERR_NONE;
}

void darray_insert_from(darray_t* pd, const darray_t* ps, size_t from, size_t count)
{
  assert(ps && pd);
  assert(from + count <= ps->size && "source range out of bounds");
  if (!try_ensure_capacity(pd, pd->size + count)) {
    return;
  }
  memcpy(darray_get_internal(pd, pd->size),
    ps->pdata + from * ps->elemsize,
    count * pd->elemsize);
  pd->size += count;
}

void darray_clear(darray_t* pd)
{
  pd->size = 0;
}

void darray_reset(darray_t* pd)
{
  pd->capacity = 0;
  pd->size = 0;
  pd->pdata = NULL;
}

bool darray_free(darray_t* pd)
{
  if (pd->pdata) {
    free(pd->pdata);
    pd->pdata = NULL;
    return true;
  }
  return false;
}

bool darray_push_back(darray_t* pd, const void* src)
{
  if (!try_ensure_capacity(pd, pd->size + 1)) return false;
  memcpy(darray_get_internal(pd, pd->size), src, pd->elemsize);
  pd->size++;
  return true;
}

bool darray_push_front(darray_t* pd, const void* src)
{
  if (!try_ensure_capacity(pd, pd->size + 1)) return false;
  memmove(pd->pdata + pd->elemsize,
    pd->pdata,
    pd->size * pd->elemsize);
  memcpy(pd->pdata, src, pd->elemsize);
  pd->size++;
  return true;
}

bool darray_pop_back(void* dst, darray_t* pd)
{
  if (pd->size == 0) return false;
  pd->size--;
  memcpy(dst,
    darray_get_internal(pd, pd->size),
    pd->elemsize);
  return true;
}

bool darray_pop_front(void* dst, darray_t* pd)
{
  if (pd->size == 0) return false;
  memcpy(dst, pd->pdata, pd->elemsize);
  memmove(pd->pdata,
    pd->pdata + pd->elemsize,
    (pd->size - 1) * pd->elemsize);
  pd->size--;
  return true;
}

int darray_shift(darray_t* pd, size_t from, size_t to, size_t count)
{
  assert(from + count <= pd->size && "shift range out of bounds");
  assert((to + count) <= pd->capacity && "shift target out of bounds");
  memmove(
    pd->pdata + to * pd->elemsize,
    pd->pdata + from * pd->elemsize,
    count * pd->elemsize
  );
  return DGERR_NONE;
}

