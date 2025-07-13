#include "dg_dt.h"
#include "dg_alloc.h"

size_t dt_get_num_functions(const void* psrcdt, size_t maxfunctions)
{
  size_t count = 0;
  void** pfuncs = (void**)((uint8_t *)psrcdt + sizeof(dt_id_t));
  while (*pfuncs && count < maxfunctions)
    count++;

  return count;
}

size_t dt_copy(void** pdst, const void* psrc, size_t maxfunctions)
{
  void* pnewdt;
  size_t byte_count;
  size_t count = dt_get_num_functions(psrc, maxfunctions);
  if (count == maxfunctions)
    return 0;

  byte_count = sizeof(dt_id_t) + count * sizeof(void*);
  pnewdt = calloc(byte_count, 1);
  if (!pnewdt)
    return 0;

  memcpy(pnewdt, psrc, byte_count);
  *pdst = pnewdt;
  return byte_count;
}
