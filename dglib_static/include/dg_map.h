// dg_map.h — header-only C99 hash map with linear probing + top7 hash metadata
// Public Domain / Unlicense. Single-file: include this header; in one C file define
//   #define DG_MAP_IMPLEMENTATION
// before including it to emit function bodies.
//
// Design goals: high-throughput, low-overhead, cache-friendly.
// - Open addressing, linear probing
// - 8-bit control metadata per bucket (like SwissTable top-7)
// - 64-bit hash stored per bucket to avoid re-hashing during probe
// - Tombstones with periodic rehash; load factor target ~85%
// - Power-of-two capacity for mask arithmetic
// - Macro-based type generator: declare + define with custom HASH/EQ
// - Iteration helpers and reserve/clear APIs
//
// Usage example (string->int):
//   #define DG_MAP_IMPLEMENTATION
//   #include "dg_map.h"
//   static uint64_t my_str_hash(const char *s) { return dg_hash_str(s); }
//   static int my_str_eq(const char *a, const char *b) { return strcmp(a,b)==0; }
//   DG_MAP_DECL(str_i32_map, const char*, int);
//   DG_MAP_IMPL(str_i32_map, const char*, int, my_str_hash, my_str_eq);
//
//   str_i32_map m; str_i32_map_init(&m, 0);
//   int v = 42; str_i32_map_set(&m, &"key", &v, NULL);
//   int out; if (str_i32_map_get(&m, &"key", &out)) { /* use out */ }
//   str_i32_map_destroy(&m);
//
// Optional per-map hooks:
//   Define NAME_KEY_FREE(k)  and/or  NAME_VAL_FREE(v) before DG_MAP_IMPL to free payloads on erase/destroy.
//   By default they are no-ops.
//
// Notes:
// - Keys/values are stored by value (copied with assignment). For pointers, manage pointees yourself
//   or use *_FREE hooks.
// - Thread safety is up to the caller.

#ifndef DG_MAP_H
#define DG_MAP_H

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#ifndef DG_MAP_MALLOC
#  define DG_MAP_MALLOC(sz) malloc(sz)
#endif
#ifndef DG_MAP_FREE
#  define DG_MAP_FREE(p) free(p)
#endif
#ifndef DG_MAP_REALLOC
#  define DG_MAP_REALLOC(p,sz) realloc(p,sz)
#endif

// --- Portable 64-bit mixers -------------------------------------------------
static inline uint64_t dg_rotl64_(uint64_t x, int r) { return (x << r) | (x >> (64 - r)); }

// SplitMix64 for seeds
static inline uint64_t dg_splitmix64_(uint64_t x) {
	x += 0x9E3779B97F4A7C15ull;
	x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ull;
	x = (x ^ (x >> 27)) * 0x94D049BB133111EBull;
	return x ^ (x >> 31);
}

// Hash arbitrary bytes. Good general-purpose mixer; not crypto.
static inline uint64_t dg_hash_bytes(const void* data, size_t len) {
	const uint8_t* p = (const uint8_t*)data;
	uint64_t h = 0xA0761D6478BD642Full ^ (uint64_t)len;
	while (len >= 8) {
		uint64_t k;
		memcpy(&k, p, 8);
		h = dg_splitmix64_(h ^ k);
		p += 8; len -= 8;
	}
	uint64_t tail = 0;
	for (size_t i = 0; i < len; ++i) tail |= (uint64_t)p[i] << (i * 8);
	h = dg_splitmix64_(h ^ tail);
	return h;
}

static inline uint64_t dg_hash_str(const char* s) {
	return dg_hash_bytes(s, s ? strlen(s) : 0);
}

// --- Internals --------------------------------------------------------------
#define DG__CTRL_EMPTY  ((uint8_t)0x80)
#define DG__CTRL_TOMB   ((uint8_t)0xFE)

static inline uint8_t dg__h2meta(uint64_t h) { return (uint8_t)((h >> 57) & 0x7F); }

static inline size_t dg__next_pow2_(size_t x) {
	if (x < 16) return 16;
	x--; x |= x >> 1; x |= x >> 2; x |= x >> 4; x |= x >> 8; x |= x >> 16;
#if SIZE_MAX > 0xFFFFFFFFu
	x |= x >> 32;
#endif
	return x + 1;
}

// --- Public API generator ---------------------------------------------------
#define DG_MAP_DECL(NAME, K, V) \
    typedef struct NAME { \
        size_t size; \
        size_t tombs; \
        size_t cap; \
        size_t mask; \
        uint8_t  *ctrl; \
        uint64_t *hash; \
        K *keys; \
        V *vals; \
    } NAME; \
    int NAME##_init(NAME *m, size_t initial_capacity); \
    void NAME##_destroy(NAME *m); \
    void NAME##_clear(NAME *m); \
    size_t NAME##_size(const NAME *m); \
    int NAME##_reserve(NAME *m, size_t want); \
    int NAME##_set(NAME *m, const K *key, const V *val, int *replaced); \
    int NAME##_get(const NAME *m, const K *key, V *out_val); \
    V* NAME##_get_ref(NAME *m, const K *key); \
    int NAME##_erase(NAME *m, const K *key); \
    /* iteration helpers */ \
    size_t NAME##_iter_begin(const NAME *m); \
    size_t NAME##_iter_next(const NAME *m, size_t it); \
    K* NAME##_iter_key(NAME *m, size_t it); \
    V* NAME##_iter_val(NAME *m, size_t it);

/*
#define DG__MAP_DEFAULT_HOOKS(NAME) \
		#ifndef NAME##_KEY_FREE \
		#define NAME##_KEY_FREE(k) (void)0 \
		#endif \
		#ifndef NAME##_VAL_FREE \
		#define NAME##_VAL_FREE(v) (void)0 \
		#endif
*/

//TODO: K.D. removed DG__MAP_DEFAULT_HOOKS(NAME)
#define DG_MAP_IMPL(NAME, K, V, HASHFN, EQFN) \
    static int NAME##__grow_(NAME *m, size_t min_cap); \
    static int NAME##__rehash_into_(NAME *m, size_t new_cap); \
    int NAME##_init(NAME *m, size_t initial_capacity) { \
        if (!m) return 0; \
        m->size = m->tombs = 0; \
        m->cap = 0; m->mask = 0; \
        m->ctrl = NULL; m->hash = NULL; m->keys = NULL; m->vals = NULL; \
        if (initial_capacity && !NAME##_reserve(m, initial_capacity)) return 0;\
        return 1;\
    } \
    void NAME##_destroy(NAME *m) { \
        if (!m) return; \
        if (m->ctrl) { \
            for (size_t i = 0; i < m->cap; ++i) { \
                uint8_t c = m->ctrl[i]; \
                if (c != DG__CTRL_EMPTY && c != DG__CTRL_TOMB) { \
                    NAME##_KEY_FREE(&m->keys[i]); \
                    NAME##_VAL_FREE(&m->vals[i]); \
                } \
            } \
            DG_MAP_FREE(m->ctrl); \
            DG_MAP_FREE(m->hash); \
            DG_MAP_FREE(m->keys); \
            DG_MAP_FREE(m->vals); \
        } \
        m->ctrl = NULL; m->hash = NULL; m->keys = NULL; m->vals = NULL; \
        m->size = m->tombs = m->cap = m->mask = 0; \
    } \
    void NAME##_clear(NAME *m) { \
        if (!m || !m->ctrl) return; \
        for (size_t i = 0; i < m->cap; ++i) { \
            if (m->ctrl[i] != DG__CTRL_EMPTY && m->ctrl[i] != DG__CTRL_TOMB) { \
                NAME##_KEY_FREE(&m->keys[i]); \
                NAME##_VAL_FREE(&m->vals[i]); \
            } \
            m->ctrl[i] = DG__CTRL_EMPTY; \
        } \
        m->size = 0; m->tombs = 0; \
    } \
    size_t NAME##_size(const NAME *m) { return m ? m->size : 0; } \
    int NAME##_reserve(NAME *m, size_t want) { \
        if (!m) return 0; \
        size_t need = dg__next_pow2_(want * 100 / 85 + 1); /* target 85% */ \
        if (need <= m->cap) return 1; \
        return NAME##__grow_(m, need); \
    } \
    static int NAME##__grow_(NAME *m, size_t min_cap) { \
        size_t new_cap = dg__next_pow2_(min_cap); \
        return NAME##__rehash_into_(m, new_cap); \
    } \
    static int NAME##__rehash_into_(NAME *m, size_t new_cap) { \
        uint8_t  *new_ctrl = (uint8_t*)DG_MAP_MALLOC(new_cap * sizeof(uint8_t)); \
        uint64_t *new_hash = (uint64_t*)DG_MAP_MALLOC(new_cap * sizeof(uint64_t)); \
        K *new_keys = (K*)DG_MAP_MALLOC(new_cap * sizeof(K)); \
        V *new_vals = (V*)DG_MAP_MALLOC(new_cap * sizeof(V)); \
        if (!new_ctrl || !new_hash || !new_keys || !new_vals) { \
            DG_MAP_FREE(new_ctrl); DG_MAP_FREE(new_hash); DG_MAP_FREE(new_keys); DG_MAP_FREE(new_vals); \
            return 0; \
        } \
        for (size_t i = 0; i < new_cap; ++i) new_ctrl[i] = DG__CTRL_EMPTY; \
        size_t old_cap = m->cap; \
        uint8_t  *old_ctrl = m->ctrl; \
        uint64_t *old_hash = m->hash; \
        K *old_keys = m->keys; \
        V *old_vals = m->vals; \
        m->ctrl = new_ctrl; m->hash = new_hash; m->keys = new_keys; m->vals = new_vals; \
        m->cap = new_cap; m->mask = new_cap - 1; \
        size_t old_size = m->size; \
        m->size = 0; m->tombs = 0; \
        if (old_ctrl) { \
            for (size_t i = 0; i < old_cap; ++i) { \
                uint8_t c = old_ctrl[i]; \
                if (c != DG__CTRL_EMPTY && c != DG__CTRL_TOMB) { \
                    /* reinsert */ \
                    uint64_t h = old_hash[i]; \
                    uint8_t meta = dg__h2meta(h); \
                    size_t idx = (size_t)h & m->mask; \
                    for (;;) { \
                        uint8_t cc = m->ctrl[idx]; \
                        if (cc == DG__CTRL_EMPTY) { \
                            m->ctrl[idx] = meta; \
                            m->hash[idx] = h; \
                            m->keys[idx] = old_keys[i]; \
                            m->vals[idx] = old_vals[i]; \
                            m->size++; \
                            break; \
                        } \
                        idx = (idx + 1) & m->mask; \
                    } \
                } \
            } \
            DG_MAP_FREE(old_ctrl); DG_MAP_FREE(old_hash); DG_MAP_FREE(old_keys); DG_MAP_FREE(old_vals); \
        } \
        (void)old_size; \
        return 1; \
    } \
    static inline int NAME##__should_grow_(const NAME *m) { \
        return (m->size + m->tombs) * 100 >= (m->cap * 85); \
    } \
    int NAME##_set(NAME *m, const K *key, const V *val, int *replaced) { \
        if (!m) return 0; \
        if (m->cap == 0 || NAME##__should_grow_(m)) { \
            size_t newc = m->cap ? (m->cap << 1) : 16; \
            if (!NAME##__grow_(m, newc)) return 0; \
        } \
        uint64_t h = (uint64_t)HASHFN(*key); \
        uint8_t meta = dg__h2meta(h); \
        size_t idx = (size_t)h & m->mask; \
        size_t first_tomb = (size_t)-1; \
        for (;;) { \
            uint8_t c = m->ctrl[idx]; \
            if (c == DG__CTRL_EMPTY) { \
                size_t t = (first_tomb != (size_t)-1) ? first_tomb : idx; \
                m->ctrl[t] = meta; \
                m->hash[t] = h; \
                m->keys[t] = *key; \
                m->vals[t] = *val; \
                m->size++; \
                if (t == first_tomb) m->tombs--; \
                if (replaced) *replaced = 0; \
                return 1; \
            } else if (c == DG__CTRL_TOMB) { \
                if (first_tomb == (size_t)-1) first_tomb = idx; \
            } else if (c == meta) { \
                if (m->hash[idx] == h && EQFN(m->keys[idx], *key)) { \
                    NAME##_VAL_FREE(&m->vals[idx]); \
                    m->vals[idx] = *val; \
                    if (replaced) *replaced = 1; \
                    return 1; \
                } \
            } \
            idx = (idx + 1) & m->mask; \
        } \
    } \
    int NAME##_get(const NAME *m, const K *key, V *out_val) { \
        if (!m || m->cap == 0) return 0; \
        uint64_t h = (uint64_t)HASHFN(*key); \
        uint8_t meta = dg__h2meta(h); \
        size_t idx = (size_t)h & m->mask; \
        for (;;) { \
            uint8_t c = m->ctrl[idx]; \
            if (c == DG__CTRL_EMPTY) return 0; \
            if (c == meta && m->hash[idx] == h && EQFN(m->keys[idx], *key)) { \
                if (out_val) *out_val = m->vals[idx]; \
                return 1; \
            } \
            idx = (idx + 1) & m->mask; \
        } \
    } \
    V* NAME##_get_ref(NAME *m, const K *key) { \
        if (!m || m->cap == 0) return NULL; \
        uint64_t h = (uint64_t)HASHFN(*key); \
        uint8_t meta = dg__h2meta(h); \
        size_t idx = (size_t)h & m->mask; \
        for (;;) { \
            uint8_t c = m->ctrl[idx]; \
            if (c == DG__CTRL_EMPTY) return NULL; \
            if (c == meta && m->hash[idx] == h && EQFN(m->keys[idx], *key)) { \
                return &m->vals[idx]; \
            } \
            idx = (idx + 1) & m->mask; \
        } \
    } \
    int NAME##_erase(NAME *m, const K *key) { \
        if (!m || m->cap == 0) return 0; \
        uint64_t h = (uint64_t)HASHFN(*key); \
        uint8_t meta = dg__h2meta(h); \
        size_t idx = (size_t)h & m->mask; \
        for (;;) { \
            uint8_t c = m->ctrl[idx]; \
            if (c == DG__CTRL_EMPTY) return 0; \
            if (c == meta && m->hash[idx] == h && EQFN(m->keys[idx], *key)) { \
                NAME##_KEY_FREE(&m->keys[idx]); \
                NAME##_VAL_FREE(&m->vals[idx]); \
                m->ctrl[idx] = DG__CTRL_TOMB; \
                m->size--; m->tombs++; \
                /* Opportunistic rehash if too many tombstones */ \
                if (m->tombs * 2 > m->cap) NAME##__rehash_into_(m, m->cap); \
                return 1; \
            } \
            idx = (idx + 1) & m->mask; \
        } \
    } \
    size_t NAME##_iter_begin(const NAME *m) { \
        if (!m || m->cap == 0) return (size_t)-1; \
        for (size_t i = 0; i < m->cap; ++i) { \
            uint8_t c = m->ctrl[i]; \
            if (c != DG__CTRL_EMPTY && c != DG__CTRL_TOMB) return i; \
        } \
        return (size_t)-1; \
    } \
    size_t NAME##_iter_next(const NAME *m, size_t it) { \
        if (!m || m->cap == 0 || it == (size_t)-1) return (size_t)-1; \
        for (size_t i = it + 1; i < m->cap; ++i) { \
            uint8_t c = m->ctrl[i]; \
            if (c != DG__CTRL_EMPTY && c != DG__CTRL_TOMB) return i; \
        } \
        return (size_t)-1; \
    } \
    K* NAME##_iter_key(NAME *m, size_t it) { \
        if (!m || it == (size_t)-1) return NULL; \
        return &m->keys[it]; \
    } \
    V* NAME##_iter_val(NAME *m, size_t it) { \
        if (!m || it == (size_t)-1) return NULL; \
        return &m->vals[it]; \
    }\
    size_t NAME##_erase_at(NAME *m, size_t it) { \
      if (!m || it == (size_t)-1 || it >= m->cap) return (size_t)-1; \
      uint8_t c = m->ctrl[it]; \
      if (c == DG__CTRL_EMPTY || c == DG__CTRL_TOMB) { \
      return NAME##_iter_next(m, it); \
      } \
      KEYFREE(&m->keys[it]); \
      VALFREE(&m->vals[it]); \
      m->ctrl[it] = DG__CTRL_TOMB; \
      m->size--; \
      m->tombs++; \
      /* Intentionally DO NOT rehash here to keep iteration stable. */ \
      return NAME##_iter_next(m, it); \
    } \
    int NAME##_compact(NAME *m) { \
    if (!m) return 0; \
    if (m->tombs == 0) return 1; \
    return NAME##__rehash_into_(m, m->cap); \
    }

#endif /* DG_MAP_H */
