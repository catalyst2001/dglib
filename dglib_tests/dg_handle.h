#pragma once
#include "dg_bitvec.h"

typedef union handle_u {
	uint64_t hvalue;
	struct {
		uint32_t index;
		uint32_t gen;
	};
} handle_t;

#define DG_HANDLE_MAGIC   (0x4C444E48)
#define DG_HANDLE_INVALID (0xFFFFFFFFFFFFFFFF)

typedef struct handle_block_s {
	uint32_t magic;
	uint32_t flags;
	size_t   size;
	uint8_t  data[];
} handle_block_t;

typedef struct dg_handle_alloc_s {
	size_t    blocksize;
	size_t    reserve;
	size_t    nhandles;
	uint32_t* pgenerations;
	uint8_t*  pblocks;
} dg_handle_alloc_t;

bool ha_init(dg_handle_alloc_t *pha, 
	size_t blocksize,
	size_t nhandles,
	size_t ngrow_reserve);
void ha_deinit(dg_handle_alloc_t* pha);

size_t ha_get_opened_handles();
