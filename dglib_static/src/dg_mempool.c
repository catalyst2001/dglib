#include "dg_mempool.h"

#define POOL_MAGIC 0xBA218001

enum MEMPOOL_FLAGS {
  MPF_NONE = 0,
  MPF_BUSY=1<<0,
};

int mempool_init(dg_mempool_t* pdst, uint32_t poolsize, uint32_t blocksize)
{
  /* init free indices queue */
  if (!mpmc_queue_alloc(&pdst->fqueue, sizeof(uint32_t), (size_t)poolsize)) {
    DG_ERROR("mempool_init(): mpmc_queue_alloc() returned false! poolsize is power of two??!");
    return 0;
  }

  /* alloc pool memory */
  pdst->poolsize = poolsize;
  pdst->blocksize = blocksize + sizeof(dg_pool_block_t);
  pdst->pdata = calloc(pdst->poolsize, pdst->blocksize);
  if (!pdst->pdata) {
    DG_ERROR("mempool_init(): memory allocation failed!");
    mpmc_queue_free(&pdst->fqueue);
    return 0;
  }
  return 1;
}

int mempool_deinit(dg_mempool_t* psrc)
{
  if (psrc) {
    if (psrc->pdata) {
      free(psrc->pdata);
      psrc->pdata = NULL;
    }

    if (psrc->fqueue.pdata)
      mpmc_queue_free(&psrc->fqueue);

    psrc->poolsize = 0;
    psrc->blocksize = 0;
    return 1;
  }
  return 0;
}

void* mempool_alloc(dg_mempool_t* psrc, uint32_t tag, uint32_t flags)
{
  uint32_t free_idx;
  if (!mpmc_queue_get_front(&free_idx, &psrc->fqueue))
    return NULL;

  dg_pool_block_t* pblock = (dg_pool_block_t*)&psrc->pdata[free_idx * psrc->blocksize];
  pblock->magic = POOL_MAGIC;
  pblock->flags = flags;
  pblock->index = free_idx;
  pblock->dbgtag = tag;
  return (uint8_t*)pblock + sizeof(dg_pool_block_t);
}

int mempool_free(dg_mempool_t* psrc, void* block)
{
  if (!block) {
    DG_ERROR("mempool_free(): block ptr is NULL");
    return 0;
  }

  dg_pool_block_t* pblock = (dg_pool_block_t*)((uint8_t*)block - sizeof(dg_pool_block_t));
  if (pblock->magic == POOL_MAGIC) { //validate magic
    if (pblock->index < psrc->poolsize) { //check bounds
      pblock->flags &= ~MPF_BUSY; // mark as free
      mpmc_queue_add_back(&psrc->fqueue, &pblock->index); //ONLY LAST! add index to free indices queue
      return 1; //OK
    }
    // index out of bounds
  }
  return 0; //invalid magic
}
