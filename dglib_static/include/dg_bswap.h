#ifndef DG_ENDIAN_H
#define DG_ENDIAN_H
#include <stdint.h>

#if defined(__clang__) || defined(__GNUC__)
#define MYLIB_BSWAP16(x) __builtin_bswap16(x)
#define MYLIB_BSWAP32(x) __builtin_bswap32(x)
#define MYLIB_BSWAP64(x) __builtin_bswap64(x)
#else
static inline uint16_t _dg_byteswap16(uint16_t x)
{
  return (uint16_t)((x >> 8) | (x << 8));
}

static inline uint32_t _dg_byteswap32(uint32_t x)
{
  return ((x >> 24)) |
    ((x >> 8) & 0x0000FF00) |
    ((x << 8) & 0x00FF0000) |
    ((x << 24));
}

static inline uint64_t _dg_byteswap64(uint64_t x)
{
  return ((x & 0x00000000000000FFULL) << 56) |
    ((x & 0x000000000000FF00ULL) << 40) |
    ((x & 0x0000000000FF0000ULL) << 24) |
    ((x & 0x00000000FF000000ULL) << 8) |
    ((x & 0x000000FF00000000ULL) >> 8) |
    ((x & 0x0000FF0000000000ULL) >> 24) |
    ((x & 0x00FF000000000000ULL) >> 40) |
    ((x & 0xFF00000000000000ULL) >> 56);
}
#endif

#if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && defined(__ORDER_BIG_ENDIAN__)
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define DG_LITTLE_ENDIAN 1
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define DG_BIG_ENDIAN    1
#else
#error "dg_bswap.h: unknown byte order (__BYTE_ORDER__)"
#endif

/* fallback: glibc-style */
#elif defined(__linux__) && defined(__GLIBC__)
#include <endian.h>
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define DG_LITTLE_ENDIAN 1
#elif __BYTE_ORDER == __BIG_ENDIAN
#define DG_BIG_ENDIAN    1
#else
#error "dg_bswap.h: unknown byte order (glibc <endian.h>)"
#endif

/* fallback by arch */
#elif defined(__i386__) || defined(__x86_64__) || defined(_M_IX86) || defined(_M_X64) || defined(__arm__)   || defined(__aarch64__)
#define DG_LITTLE_ENDIAN 1
#else
#error "dg_bswap.h: unknown platform!"
#endif

#if defined(DG_LITTLE_ENDIAN)
#define htole16(x)  (uint16_t)(x)
#define letoh16(x)  (uint16_t)(x)
#define htobe16(x)  _dg_byteswap16(x)
#define betoh16(x)  _dg_byteswap16(x)

#define htole32(x)  (uint32_t)(x)
#define letoh32(x)  (uint32_t)(x)
#define htobe32(x)  _dg_byteswap32(x)
#define betoh32(x)  _dg_byteswap32(x)

#define htole64(x)  (uint64_t)(x)
#define letoh64(x)  (uint64_t)(x)
#define htobe64(x)  _dg_byteswap64(x)
#define betoh64(x)  _dg_byteswap64(x)
#elif defined(DG_BIG_ENDIAN)
#define htobe16(x)  (uint16_t)(x)
#define betoh16(x)  (uint16_t)(x)
#define htole16(x)  _dg_byteswap16(x)
#define letoh16(x)  _dg_byteswap16(x)

#define htobe32(x)  (uint32_t)(x)
#define betoh32(x)  (uint32_t)(x)
#define htole32(x)  _dg_byteswap32(x)
#define letoh32(x)  _dg_byteswap32(x)

#define htobe64(x)  (uint64_t)(x)
#define betoh64(x)  (uint64_t)(x)
#define htole64(x)  _dg_byteswap64(x)
#define letoh64(x)  _dg_byteswap64(x)
#endif
#endif /* dg_bswap.h */