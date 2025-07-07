#include "dg_vector.h"
#include "dg_alloc.h"

#include <string.h>

void* dg_da_get_ex(dg_da_t* pdarray, size_t idx)
{
  assert((idx * pdarray->elemsize < pdarray->size) && "idx out of bounds");
  return pdarray->pdata + idx * pdarray->elemsize;
}

int dg_da_resize(dg_da_t* pdst, size_t newsize)
{
  int status;
  if (newsize > pdst->capacity) {
    status = dg_da_realloc(pdst, newsize);
    if (status != DGERR_NONE) {
      /* reallocation failed */
      return status;
    }
    pdst->size = newsize;
  }
  return DGERR_NONE;
}

int dg_da_realloc(dg_da_t* pdst, size_t newcap)
{
  /* realloc memory with reservation */
  if (newcap > pdst->capacity) {
    pdst->capacity = newcap + pdst->reserve;
    pdst->pdata = realloc(pdst->pdata, pdst->capacity * pdst->elemsize);
    if (!pdst->pdata) {
      /* allocation failed */
      return DGERR_OUT_OF_MEMORY;
    }
  }
  return DGERR_NONE;
}

int dg_da_shrink_to_fit(dg_da_t* pdarray)
{
  if (pdarray->capacity > pdarray->size)
    return dg_da_realloc(pdarray, pdarray->size);

  return DGERR_NONE;
}

void dg_da_move(dg_da_t* pdst, dg_da_t* psrc)
{
  pdst->capacity = psrc->capacity;
  pdst->elemsize = psrc->elemsize;
  pdst->pdata = psrc->pdata;
  pdst->reserve = psrc->reserve;
  pdst->size = psrc->size;
  dg_da_reset(psrc);
}

int dg_da_copy(dg_da_t* pdst, const dg_da_t* psrc)
{
  int status;
  if (!psrc->capacity)
    return DGERR_NONE;

  pdst->pdata = malloc(psrc->capacity);
  if (!pdst->pdata)
    return DGERR_OUT_OF_MEMORY;

  memcpy(pdst->pdata, psrc->pdata, psrc->capacity * psrc->elemsize);
}

void dg_da_insert_from(dg_da_t* pdarray, const dg_da_t* pdarraysrc, size_t from, size_t count)
{
}

void dg_da_clear(dg_da_t* pdst)
{
}

void dg_da_reset(dg_da_t* pdarray)
{
  pdarray->capacity = 0;
  pdarray->size = 0;
  pdarray->pdata = NULL;
}

bool dg_da_free(dg_da_t* pdarray)
{
  if (pdarray->pdata) {
    free(pdarray->pdata);
    return true;
  }
  return false;
}

bool dg_da_push_back(dg_da_t* pdarray, const void* psrc)
{
  return false;
}

bool dg_da_push_front(dg_da_t* pdarray, const void* psrc)
{
  return false;
}

bool dg_da_pop_back(void* pdst, dg_da_t* pdarray)
{
  return false;
}

bool dg_da_pop_front(void* pdst, dg_da_t* pdarray)
{
  return false;
}
