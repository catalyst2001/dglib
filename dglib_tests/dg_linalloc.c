#include "dg_linalloc.h"
#include "dg_thread.h"

#define LA_CURR_THREAD_DATA(varname, ...)\
  dg_thrd_data_t *varname = dg_get_curr_thread_data();\
  if (!varname)\
    return __VA_ARGS__;

bool la_present()
{
  LA_CURR_THREAD_DATA(pthrddata, false)
  return linalloc_is_present(&pthrddata->hunk_allocator);
}

bool la_resize(size_t newsize)
{
  LA_CURR_THREAD_DATA(pthrddata, false)
  return linalloc_init(&pthrddata->hunk_allocator, newsize);
}

size_t la_get_capacity()
{
  LA_CURR_THREAD_DATA(pthrddata, 0)
  return pthrddata->hunk_allocator.capacity;
}

size_t la_get_position()
{
  LA_CURR_THREAD_DATA(pthrddata, 0)
  return pthrddata->hunk_allocator.position;
}

void* la_hunk_alloc(size_t size, bool bclear)
{
  void* ptr;
  LA_CURR_THREAD_DATA(pthrddata, NULL)
  ptr = linalloc_hunk_alloc(&pthrddata->hunk_allocator, size);
  if (bclear && ptr)
    memset(ptr, 0, size);
  
  return ptr;
}

void la_reset()
{
  LA_CURR_THREAD_DATA(pthrddata)
  linalloc_reset(&pthrddata->hunk_allocator);
}
