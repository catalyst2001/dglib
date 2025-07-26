#include "dg_handle.h"
#include "dg_alloc.h"

bool ha_init(dg_handle_alloc_t* pha, 
  size_t blocksize, 
  size_t nhandles, 
  size_t ngrow_reserve)
{
  assert(pha && "pha is NULL");
  pha->blocksize = blocksize;
  pha->nhandles = nhandles;
  pha->pblocks = (uint8_t*)calloc(pha->nhandles, pha->blocksize);
  if (!pha->pblocks)
    return false;

  pha->pgenerations = (uint32_t*)calloc(pha->nhandles, sizeof(uint32_t));
  if (!pha->pblocks)
    return false;

  return true;
}

void ha_init_static(dg_handle_alloc_t* pha, size_t blocksize, size_t nhandles, uint8_t* pblocks, uint32_t* pgens)
{
  assert(pha && "pha is NULL");
  pha->blocksize = blocksize;
  pha->nhandles = nhandles;
  pha->pblocks = pblocks;
  pha->pgenerations = pgens;
  pha->reserve = 0;
}

void ha_deinit(dg_handle_alloc_t* pha)
{
  assert(pha && "pha is NULL");
  if (pha->pgenerations) {
    free(pha->pgenerations);
    pha->pgenerations = NULL;
  }
  if (pha->pblocks) {
    free(pha->pblocks);
    pha->pblocks = NULL;
  }
  pha->nhandles = 0;
}

bool ha_get_info(dg_hinfo_t* pdst, dg_handle_alloc_t* pha)
{
  assert(pha && "pha is NULL");
  //TODO: K.D. fix it 
  pdst->allocated = pdst->max_handles = pha->nhandles;
  return true;
}

/**
* ha_alloc_handle
* 
* new handles allocation
* 
* example:
* generation 0 (even) - handle is free 
* generation 1 (odd) - handle is busy 
* generation 2 (even) - handle is free
* ...
* 
* Thus the total number of generations for each handle is 2^(size*8-1)
* 
*/
bool ha_alloc_handle(dg_halloc_result_t* pdst, dg_handle_alloc_t* pha)
{
  assert(pha && "pha is NULL");
  /* find free handles */
  for (size_t i = 0; i < pha->nhandles; i++) {
    if ((pha->pgenerations[i] & 1u) == 0) {
      pha->pgenerations[i]++;
      pdst->handle_body_size = pha->blocksize;
      pdst->phandle_body = &pha->pblocks[i * pha->blocksize];
      pdst->new_handle = ((handle_t){ 
        .index = i,
        .gen = pha->pgenerations[i]
      });
      return true;
    }
  }
  /* all handles are busy */
  return false;
}

bool ha_free_handle(dg_handle_alloc_t* pha, handle_t handle)
{
  size_t idx;
  assert(pha && "pha is NULL");
  if (ha_is_valid_handle(pha, handle)) {
    idx = (size_t)handle.index;
    pha->pgenerations[idx]++;
    return true;
  }
  return false;
}

bool ha_is_valid_handle(dg_handle_alloc_t* pha, handle_t handle)
{
  size_t handle_index;
  assert(pha && "pha is NULL");
  handle_index = (size_t)handle.index;
  /* valid index? */
  if (handle_index >= pha->nhandles)
    return false;

  /* old handle? */
  if (pha->pgenerations[handle_index] != handle.gen)
    return false;

  return true;
}

bool ha_get_handle_by_index(handle_t* pdst, dg_handle_alloc_t* pha, size_t index)
{
  handle_t handle = DG_INVALID_HANDLE;
  assert(pha && "pha is NULL");
  if (index < pha->nhandles) {
    handle.index = (uint32_t)index;
    handle.gen = pha->pgenerations[handle.index];
    *pdst = handle;
    return true;
  }
  return false;
}

void* ha_get_handle_data(dg_handle_alloc_t* pha, handle_t handle)
{
  if (!ha_is_valid_handle(pha, handle))
    return NULL;

  return &pha->pblocks[pha->blocksize * handle.index];
}

handle_t ha_get_first_handle(dg_handle_alloc_t* pha, size_t start, bool is_free_only)
{
  handle_t handle;
  if (start >= pha->nhandles)
    return DG_INVALID_HANDLE;

  handle.index = start;
  while (handle.index < pha->nhandles) {
    handle.gen = pha->pgenerations[handle.index];
    /* find busy handle */
    //TODO: K.D. is_free_only ignored here!
    if ((handle.gen & 1u) != 0) {
      return handle;
    }
    handle.index++;
  }
  return DG_INVALID_HANDLE;
}

bool ha_get_next_handle(handle_t* pdst, dg_handle_alloc_t* pha)
{
  assert(pha && "pha is NULL");
  handle_t handle = *pdst;
  uint32_t num_handles = (uint32_t)pha->nhandles;
  while (handle.index < num_handles) {
    handle.gen = pha->pgenerations[handle.index];
    if ((handle.gen & 1u) != 0) {
      /* valid handle found */
      return true;
    }
    handle.index++;
  }
  return false;
}
