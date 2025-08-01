#include "dg_bitvec.h"
#include "dg_alloc.h"

bool dbitvec_init(dg_dbitvec_t* dbv, size_t size)
{
  assert(dbv && "dbv is NULL");
  size = DG_ALIGN_UP(size, BITVEC_BITS_PER_CELL);
  dbv->ncells = size / BITVEC_BITS_PER_CELL;
  if (!dbv->ncells) {
    DG_ERROR("dbitvec_init(): trying empty allocation!");
    return false;
  }

  dbv->pbits = (uint32_t*)calloc(dbv->ncells, sizeof(uint32_t));
  if (!dbv->pbits) {
    DG_ERROR("dbitvec_init(): bits allocation failed! requested %zd cells", 
      dbv->ncells);
    return false;
  }
  return true;
}

void dbitvec_deinit(dg_dbitvec_t* dbv)
{
  dbv->ncells = 0;
  if (dbv->pbits) {
    free(dbv->pbits);
    dbv->pbits = NULL;
  }
}
