#pragma once
#include "dg_libcommon.h"
#include "dg_atomic.h"
#include "dg_sync.h"

typedef struct dg_queue_s {
	size_t   elemsize;
	size_t   capacity;
	size_t   head;
	size_t   tail;
	size_t   count;
	uint8_t* pdata;
} dg_queue_t;

#define queue_init(type, inital_cap) {\
	.elemsize = sizeof(type),\
	.capacity = inital_cap,\
	.head=0, .tail=0,\
	.count=0,\
	.pdata=NULL\
}
bool  queue_alloc(dg_queue_t *pqueue);
bool  queue_free(dg_queue_t* pqueue);
bool  queue_add_at(dg_queue_t* pqueue, size_t idx, const void *psrc);
void *queue_get_at(dg_queue_t* pqueue, size_t idx);
bool  queue_add_back(dg_queue_t* pqueue, const void* psrc);
void *queue_get_back(dg_queue_t* pqueue);
bool  queue_add_front(dg_queue_t* pqueue, const void* psrc);
void *queue_get_front(dg_queue_t* pqueue);
#define queue_is_empty(p) ((p)->count==0)
#define queue_is_full(p) ((p)->count == (p)->capacity)

/**
* multithreading queue
*/

// Lock-free bounded multiple-producer/multiple-consumer queue
// Uses Dmitry Vyukov's MPMC algorithm
// Capacity must be a power of two.
typedef struct dg_mtqueue_mpmc_s {
	size_t         elemsize;    // size of each element in bytes
	size_t         capacity;    // number of slots (power of two)
	size_t         mask;        // capacity - 1, for index wrap
	atomic_size_t  enqueue_pos; // next position to enqueue
	atomic_size_t  dequeue_pos; // next position to dequeue
	atomic_size_t *seq;        // sequence for each slot, length = capacity
	uint8_t       *pdata;      // raw buffer for elements: capacity * elemsize
} dg_mtqueue_mpmc_t;

bool  mpmc_queue_alloc(dg_mtqueue_mpmc_t* q, size_t elemsize, size_t capacity);
bool  mpmc_queue_free(dg_mtqueue_mpmc_t* pqueue);
bool  mpmc_queue_add_back(dg_mtqueue_mpmc_t* pqueue, const void* psrc);
bool  mpmc_queue_get_front(void* pdst, dg_mtqueue_mpmc_t* q);
bool  mpmc_queue_is_empty(dg_mtqueue_mpmc_t* pqueue);
bool  mpmc_queue_is_full(dg_mtqueue_mpmc_t* pqueue);