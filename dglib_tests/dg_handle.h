#pragma once
#include "dg_bitvec.h"

typedef union handle_u {
	uint64_t hvalue;
	struct {
		uint32_t index;
		uint32_t gen;
	};
} handle_t;

#define DG_HANDLE_MAGIC         (0x4C444E48)
#define DG_HANDLE_INVALID_INDEX (0xFFFFFFFF)
#define DG_HANDLE_INVALID_GEN   (0xFFFFFFFF)
#define DG_HANDLE_INVALID_VALUE (0xFFFFFFFFFFFFFFFF) //low part: DG_HANDLE_INVALID_INDEX and high part: DG_HANDLE_INVALID_GEN

#define DG_INVALID_HANDLE ((handle_t){ .hvalue = DG_HANDLE_INVALID_VALUE })

//typedef struct handle_block_s {
//	uint32_t magic;
//	uint32_t flags;
//	size_t   size;
//	uint8_t  data[];
//} handle_block_t;

typedef struct dg_handle_alloc_s {
	size_t    blocksize;
	size_t    reserve;
	size_t    nhandles;
	uint32_t* pgenerations;
	uint8_t*  pblocks;
} dg_handle_alloc_t;

#define ha_init_data_arrays(num_handles, bsize)\
	uint8_t block

#define ha_init_static(num_handles, bsize, pgens, pblks)\
	((dg_handle_alloc_t){\
		.blocksize=bsize,\
		.reserve=1,\
		.nhandles=num_handles,\
		.pgenerations=pgens,\
		.pblocks=pblks\
	})

/**
* @brief creates new handle allocator
* 
* 
* 
* 
*/
bool ha_init(dg_handle_alloc_t *pha, 
	size_t blocksize,
	size_t nhandles,
	size_t ngrow_reserve);

void ha_deinit(dg_handle_alloc_t* pha);

typedef struct dg_hinfo_s {
	size_t allocated;
	size_t max_handles;
} dg_hinfo_t;

bool ha_get_info(dg_hinfo_t *pdst, dg_handle_alloc_t* pha);

typedef struct dg_halloc_result_s {
	handle_t new_handle;
	uint8_t* phandle_body;
	size_t   handle_body_size;
} dg_halloc_result_t;

bool ha_alloc_handle(dg_halloc_result_t *pdst, dg_handle_alloc_t* pha);

bool ha_free_handle(dg_handle_alloc_t* pha, handle_t handle);

bool ha_is_valid_handle(dg_handle_alloc_t* pha, handle_t handle);

bool ha_get_handle_by_index(handle_t *pdst, dg_handle_alloc_t* pha, size_t index);

bool ha_enumerate_handles(handle_t *pdst, dg_handle_alloc_t* pha);
