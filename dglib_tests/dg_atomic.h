#pragma once

#if defined(_MSC_VER)
#include <intrin.h>
#if defined(_M_X64) || defined(_M_ARM64)
typedef volatile long long atomic_size_t;
#pragma intrinsic(_InterlockedCompareExchange64)
#pragma intrinsic(_InterlockedExchange64)
#pragma intrinsic(_InterlockedExchangeAdd64)
#define ATOMIC_LOAD(ptr) ((size_t)InterlockedCompareExchange64((volatile atomic_size_t*)(ptr), 0, 0))
#define ATOMIC_STORE(ptr, val) InterlockedExchange64((volatile atomic_size_t*)(ptr), (atomic_size_t)(val))
#define ATOMIC_EXCHANGE(ptr, val) ((size_t)InterlockedExchange64((volatile atomic_size_t*)(ptr), (atomic_size_t)(val)))
#define ATOMIC_FETCH_ADD(ptr, val) ((size_t)InterlockedExchangeAdd64((volatile atomic_size_t*)(ptr), (atomic_size_t)(val)))
#else
typedef volatile long atomic_size_t;
#pragma intrinsic(_InterlockedCompareExchange)
#pragma intrinsic(_InterlockedExchange)
#pragma intrinsic(_InterlockedExchangeAdd)
#define ATOMIC_LOAD(ptr) ((size_t)InterlockedCompareExchange((volatile LONG*)(ptr), 0, 0))
#define ATOMIC_STORE(ptr, val) InterlockedExchange((volatile LONG*)(ptr), (LONG)(val))
#define ATOMIC_EXCHANGE(ptr, val) ((size_t)InterlockedExchange((volatile LONG*)(ptr), (LONG)(val)))
#define ATOMIC_FETCH_ADD(ptr, val) ((size_t)InterlockedExchangeAdd((volatile LONG*)(ptr), (LONG)(val)))
#endif

#elif defined(__GNUC__) || defined(__clang__)
typedef size_t atomic_size_t;
#define ATOMIC_LOAD(ptr) __atomic_load_n(ptr, __ATOMIC_SEQ_CST)
#define ATOMIC_STORE(ptr, val) __atomic_store_n(ptr, val, __ATOMIC_SEQ_CST)
#define ATOMIC_EXCHANGE(ptr, val) __atomic_exchange_n(ptr, val, __ATOMIC_SEQ_CST)
#define ATOMIC_FETCH_ADD(ptr, val) __atomic_fetch_add(ptr, val, __ATOMIC_SEQ_CST)
#else
#error "atomic operations not supported by this compiler"
#endif
