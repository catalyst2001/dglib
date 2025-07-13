#include "dg_alloc.h"

bool linalloc_init(dg_hunkalloc_t* pdst, size_t cap)
{
  size_t allocsize=0;
  if (!cap) {
    assert(cap && "invalid capacity");
    return false;
  }

  if (!(pdst->flags & DGLA_LESSRESIZE)) {
    if (cap > pdst->capacity) {
      pdst->capacity = cap;
      allocsize = cap;
    }
  } else if (pdst->capacity > 0 && pdst->capacity < cap) {
    pdst->capacity = cap;
    allocsize = cap;
  }
  else {
    pdst->capacity = cap;
    //TODO: K.D. brain is dead
  }

  if (allocsize) {
    pdst->pdata = realloc(pdst->pdata, allocsize);
    return pdst->pdata != NULL;
  }
  return true;
}

void linalloc_deinit(dg_hunkalloc_t* pdst)
{
  linalloc_reset(pdst);
  if (pdst->pdata) {
    free(pdst->pdata);
    pdst->pdata = NULL;
  }
}

void* linalloc_hunk_alloc(dg_hunkalloc_t* pdst, size_t size)
{
  void* pmemchunk;
  size_t next_pos = pdst->position + size;
  if (next_pos >= pdst->capacity) {
    /* is not resizable? */
    if (!(pdst->flags & DGLA_AUTORESIZE)) {
      /* not anough space to alloc */
      return NULL;
    }

    /* try to resize */
    if (!linalloc_init(pdst, pdst->capacity + size + (size >> 1))) {
      /* not anough space for resize */
      return NULL;
    }
  }
  pmemchunk = &pdst->pdata[pdst->position];
  pdst->position += next_pos;
  return pmemchunk;
}

void linalloc_reset(dg_hunkalloc_t* pdst)
{
  pdst->position = 0;
}