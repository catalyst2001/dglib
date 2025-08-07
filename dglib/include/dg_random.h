#pragma once
#include "dg_libcommon.h"

/**
* Mersenne Twister random engnie (MT19937)
*/

/* Period parameters */
#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define UPPER_MASK 0x80000000UL /* most significant w-r bits */
#define LOWER_MASK 0x7fffffffUL /* least significant r bits */

typedef struct dg_mt19937_s {
	unsigned long mt[N]; /*< the array for the state vector  */
	int mti; /*< mti==N+1 means mt[N] is not initialized */
} dg_mt19937_t;

DG_API void mt19937_init(dg_mt19937_t *pdstctx, unsigned long seed);
DG_API void mt19937_init_by_array(dg_mt19937_t * pdstctx, unsigned long init_key[], int key_length);
DG_API unsigned long mt19937_genrand_int32(dg_mt19937_t* pctx);

static inline long mt19937_genrand_int31(dg_mt19937_t* pctx) {
	return (long)(mt19937_genrand_int32(pctx) >> 1);
}

/* generates a random number on [0,1]-real-interval */
static inline float mt19937_genrand_float32_full(dg_mt19937_t* pctx) {
  return mt19937_genrand_int32(pctx) * (1.f / 4294967295.f); /* divided by 2^32-1 */
}

/* generates a random number on [0,1)-real-interval */
static inline float mt19937_genrand_float32_notone(dg_mt19937_t* pctx) {
  return mt19937_genrand_int32(pctx) * (1.f / 4294967296.f); /* divided by 2^32 */
}