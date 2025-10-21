#pragma once
#include "dg_alloc.h"
#include "dg_queue.h"

typedef struct dg_mempool_s {
  uint32_t          poolsize; /*< elements in pool */
  uint32_t          blocksize; /*< block size (include dg_pool_block_t size) */
  uint8_t*          pdata; /*< preallocated memory */
  dg_mtqueue_mpmc_t fqueue; /*< free indices queue */
} dg_mempool_t;

typedef struct dg_pool_block_s {
  uint32_t magic;
  uint32_t flags;
  uint32_t index;
  uint32_t dbgtag;
} dg_pool_block_t;

int   mempool_init(dg_mempool_t *pdst, uint32_t poolsize, uint32_t blocksize);
int   mempool_deinit(dg_mempool_t* psrc);
void* mempool_alloc(dg_mempool_t* psrc, uint32_t tag, uint32_t flags, uint32_t *dstindex);
bool  mempool_is_valid_addr(dg_mempool_t* psrc, const void* block);
int   mempool_free(dg_mempool_t* psrc, void *block);

static inline const dg_pool_block_t* mempool_get_block_info(dg_mempool_t* psrc, const void *pblock) {
  if (mempool_is_valid_addr(psrc, pblock))
		return (const dg_pool_block_t*)((const uint8_t*)pblock - sizeof(dg_pool_block_t));
  
	return NULL;
}

static inline void* mempool_get(dg_mempool_t* psrc, size_t block_index) {
  if (block_index < psrc->poolsize)
    return (&psrc->pdata[block_index * psrc->blocksize] + sizeof(dg_pool_block_t));
  
  return NULL;
}

/* === NEW: helper query functions === */
/* NOTE: The free/allocated counts are derived from the internal MPMC queue.
 * The queue stores indices of FREE blocks. So: allocated = poolsize - free.
 * Under concurrent producers/consumers these values are momentary snapshots. */
uint32_t mempool_get_num_free(const dg_mempool_t* psrc);

uint32_t mempool_get_num_allocated(const dg_mempool_t* psrc);