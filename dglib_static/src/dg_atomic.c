#include "dg_atomic.h"
#include <Windows.h>
#include <intrin.h>

#if defined(_MSC_VER)
size_t dg_atomic_load(atomic_size_t* ptr)
{
#if defined(_M_X64)
  return ((size_t)InterlockedCompareExchange64((volatile atomic_size_t*)(ptr), 0, 0));
#else
  return ((size_t)InterlockedCompareExchange((volatile LONG*)(ptr), 0, 0));
#endif
}

size_t dg_atomic_store(atomic_size_t* ptr, atomic_size_t val)
{
#if defined(_M_X64)
  return InterlockedExchange64((volatile atomic_size_t*)(ptr), (atomic_size_t)(val));
#else
  return InterlockedExchange((volatile LONG*)(ptr), (LONG)(val));
#endif
}

size_t dg_atomic_exchange(atomic_size_t* ptr, atomic_size_t val)
{
#if defined(_M_X64)
  return ((size_t)InterlockedExchange64((volatile atomic_size_t*)(ptr), (atomic_size_t)(val)));
#else
  return ((size_t)InterlockedExchange((volatile LONG*)(ptr), (LONG)(val)));
#endif
}

size_t dg_atomic_fetch_add(atomic_size_t* ptr, atomic_size_t val)
{
#if defined(_M_X64)
  return ((size_t)InterlockedExchangeAdd64((volatile atomic_size_t*)(ptr), (atomic_size_t)(val)));
#else
  return ((size_t)InterlockedExchangeAdd((volatile LONG*)(ptr), (LONG)(val)));
#endif
}

size_t dg_atomic_fetch_sub(atomic_size_t* ptr, atomic_size_t val)
{
#if defined(_M_X64)
  return ((size_t)InterlockedExchangeAdd64((volatile atomic_size_t*)(ptr), (atomic_size_t)(-val)));
#else
  ((size_t)InterlockedExchangeAdd((volatile LONG*)(ptr), (LONG)(-val)))
#endif
}

#elif defined(__GNUC__) || defined(__clang__)
size_t dg_atomic_load(atomic_size_t* ptr)
{
  return __atomic_load_n(ptr, __ATOMIC_SEQ_CST);
}

size_t dg_atomic_store(atomic_size_t* ptr, atomic_size_t val)
{
  return __atomic_store_n(ptr, val, __ATOMIC_SEQ_CST);
}

size_t dg_atomic_exchange(atomic_size_t* ptr, atomic_size_t val)
{
  return __atomic_exchange_n(ptr, val, __ATOMIC_SEQ_CST);
}

size_t dg_atomic_fetch_add(atomic_size_t* ptr, atomic_size_t val)
{
  return __atomic_fetch_add(ptr, val, __ATOMIC_SEQ_CST);
}

size_t dg_atomic_fetch_sub(atomic_size_t* ptr, atomic_size_t val)
{
  return __atomic_fetch_add(ptr, -val, __ATOMIC_SEQ_CST);
}

#endif