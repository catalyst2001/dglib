#pragma once

#if defined(_M_X64) || defined(_M_ARM64)
typedef volatile long long atomic_size_t;
#elif defined(_M_IX86)
typedef volatile long atomic_size_t;
#elif defined(__GNUC__) || defined(__clang__)
typedef size_t atomic_size_t;
#else
#error "atomic operations not supported by this compiler"
#endif

size_t dg_atomic_load(atomic_size_t *ptr);
size_t dg_atomic_store(atomic_size_t* ptr, atomic_size_t val);
size_t dg_atomic_exchange(atomic_size_t* ptr, atomic_size_t val);
size_t dg_atomic_fetch_add(atomic_size_t* ptr, atomic_size_t val);
size_t dg_atomic_fetch_sub(atomic_size_t* ptr, atomic_size_t val);