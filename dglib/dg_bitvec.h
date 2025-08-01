#pragma once
#include "dg_libcommon.h"
#include "dg_darray.h"

enum BITVEC_CONTANTS {
	BITVEC_BITS_PER_CELL = sizeof(uint32_t)*8
};

typedef struct dg_dbitvec_s {
	size_t    ncells;
	uint32_t* pbits;
} dg_dbitvec_t;

bool dbitvec_init(dg_dbitvec_t *dbv, size_t size);
void dbitvec_deinit(dg_dbitvec_t* dbv);

static inline void dbitvec_set(dg_dbitvec_t* dbv, size_t bit, int value) {
	assert(dbv && "dbv is NULL");
	assert(dbv->pbits && "dbv->pbits is NULL");
	size_t   cell = bit / BITVEC_BITS_PER_CELL;
	uint32_t cbit = (uint32_t)(bit % BITVEC_BITS_PER_CELL);
	assert(cell < dbv->ncells && "cell out of bounds");
	if (value)
		dbv->pbits[cell] |= (1U << cbit);
	else dbv->pbits[cell] &= ~(1U << cbit);
}

static inline bool dbitvec_get(dg_dbitvec_t* dbv, size_t bit) {
	assert(dbv && "dbv is NULL");
	assert(dbv->pbits && "dbv->pbits is NULL");
	uint32_t mask;
	size_t   cell = bit / BITVEC_BITS_PER_CELL;
	uint32_t cbit = (uint32_t)(bit % BITVEC_BITS_PER_CELL);
	assert(cell < dbv->ncells && "cell out of bounds");
	mask = (1U << cbit);
	return (dbv->pbits[cell] & mask) == mask;
}