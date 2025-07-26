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

#define HA_DECL_DATA_ARRAYS(name, count, elemsize) struct {\
	uint8_t blocks[count*elemsize]; \
	uint32_t generations[count]; \
} name

/**
* @brief creates new handle allocator
*/
DG_API bool ha_init(dg_handle_alloc_t *pha,
	size_t blocksize,
	size_t nhandles,
	size_t ngrow_reserve);

DG_API void ha_init_static(dg_handle_alloc_t* pha,
	size_t blocksize,
	size_t nhandles,
	uint8_t *pblocks,
	uint32_t *pgens);

DG_API void ha_deinit(dg_handle_alloc_t* pha);

typedef struct dg_hinfo_s {
	size_t allocated;
	size_t max_handles;
} dg_hinfo_t;

DG_API bool ha_get_info(dg_hinfo_t *pdst, dg_handle_alloc_t* pha);

typedef struct dg_halloc_result_s {
	handle_t new_handle;
	uint8_t* phandle_body;
	size_t   handle_body_size;
} dg_halloc_result_t;

DG_API bool ha_alloc_handle(dg_halloc_result_t *pdst, dg_handle_alloc_t* pha);

DG_API bool ha_free_handle(dg_handle_alloc_t* pha, handle_t handle);

DG_API bool ha_is_valid_handle(dg_handle_alloc_t* pha, handle_t handle);

DG_API bool ha_get_handle_by_index(handle_t *pdst, dg_handle_alloc_t* pha, size_t index);

DG_API void* ha_get_handle_data(dg_handle_alloc_t* pha, handle_t handle);

DG_API handle_t ha_get_first_handle(dg_handle_alloc_t* pha, size_t start, bool is_free_only);

DG_API bool ha_get_next_handle(handle_t *pdst, dg_handle_alloc_t* pha);
