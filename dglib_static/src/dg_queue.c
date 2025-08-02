#include "dg_queue.h"
#include "dg_alloc.h"

static bool queue_allocate_if_needed(dg_queue_t* q) {
  if (!q->pdata) {
    q->pdata = malloc(q->capacity * q->elemsize);
    if (!q->pdata)
      return false;
  }
  return true;
}

inline void queue_reset_internal(dg_queue_t* pqueue)
{
  pqueue->count = 0;
  pqueue->head = 0;
  pqueue->tail = 0;
}

bool queue_alloc(dg_queue_t* pqueue)
{
  pqueue->pdata = calloc(pqueue->capacity, pqueue->elemsize);
  if (!pqueue->pdata)
    return false;

  queue_reset_internal(pqueue);
  return true;
}

bool queue_free(dg_queue_t* pqueue)
{
  if (!pqueue)
    return false;

  if (pqueue->pdata) {
    free(pqueue->pdata);
    pqueue->pdata=NULL;
  }
  return true;
}

/*
bool queue_add_at(dg_queue_t* pqueue, size_t idx, const void* psrc)
{
  size_t cap = pqueue->capacity;
  if (idx > pqueue->count)
    return false;

  //if (!queue_ensure_capacity(pqueue, pqueue->count + 1))
  //  return false;

  for (size_t i = pqueue->count; i > idx; --i) {
    size_t from = (pqueue->head + i - 1) % cap;
    size_t to = (pqueue->head + i) % cap;
    memcpy(pqueue->pdata + to * pqueue->elemsize,
      pqueue->pdata + from * pqueue->elemsize,
      pqueue->elemsize);
  }
  size_t pos = (pqueue->head + idx) % cap;
  memcpy(pqueue->pdata + pos * pqueue->elemsize, psrc, pqueue->elemsize);
  pqueue->count++;
  pqueue->tail = (pqueue->head + pqueue->count) % cap;
}

void* queue_get_at(dg_queue_t* pqueue, size_t idx)
{
  if (pqueue->pdata && idx < pqueue->capacity)
    return pqueue->pdata + pqueue->elemsize * idx;

  return NULL;
}
*/

bool queue_add_back(dg_queue_t* q, const void* src) {
  if (queue_is_full(q))
    return false;
  if (!queue_allocate_if_needed(q))
    return false;

  memcpy(q->pdata + q->tail * q->elemsize, src, q->elemsize);
  q->tail = (q->tail + 1) % q->capacity;
  q->count++;
  return true;
}

void* queue_get_back(dg_queue_t* q) {
  if (queue_is_empty(q) || !q->pdata)
    return NULL;
  q->tail = (q->tail + q->capacity - 1) % q->capacity;
  void* elem = q->pdata + q->tail * q->elemsize;
  q->count--;
  return elem;
}

bool queue_add_front(dg_queue_t* q, const void* src) {
  if (queue_is_full(q))
    return false;
  if (!queue_allocate_if_needed(q))
    return false;
  q->head = (q->head + q->capacity - 1) % q->capacity;
  memcpy(q->pdata + q->head * q->elemsize, src, q->elemsize);
  q->count++;
  return true;
}

void* queue_get_front(dg_queue_t* q)
{
  if (queue_is_empty(q) || !q->pdata)
    return NULL;
  void* elem = q->pdata + q->head * q->elemsize;
  q->head = (q->head + 1) % q->capacity;
  q->count--;
  return elem;
}

/* ------- mtqueue --------- */
static inline bool is_power_of_two(size_t x) {
  return x && ((x & (x - 1)) == 0);
}

bool mpmc_queue_alloc(dg_mtqueue_mpmc_t* q, size_t elemsize, size_t capacity)
{
  if (!is_power_of_two(capacity))
    return false;

  q->elemsize = elemsize;
  q->capacity = capacity;
  q->mask = capacity - 1;
  q->seq = malloc(sizeof(atomic_size_t) * capacity);
  q->pdata = malloc(elemsize * capacity);
  if (!q->seq || !q->pdata) {
    free(q->seq);
    free(q->pdata);
    return false;
  }
  // initialize sequence values
  for (size_t i = 0; i < capacity; ++i) {
    dg_atomic_store(&q->seq[i], i);
  }
  // initialize positions
  dg_atomic_store(&q->enqueue_pos, 0);
  dg_atomic_store(&q->dequeue_pos, 0);
  return true;
}

bool mpmc_queue_free(dg_mtqueue_mpmc_t* q)
{
  if (!q)
    return false;
  free(q->seq);
  free(q->pdata);
  q->seq = NULL;
  q->pdata = NULL;
  // reset positions
  dg_atomic_store(&q->enqueue_pos, 0);
  dg_atomic_store(&q->dequeue_pos, 0);
  return true;
}

bool mpmc_queue_add_back(dg_mtqueue_mpmc_t* q, const void* psrc)
{
  size_t pos = dg_atomic_fetch_add(&q->enqueue_pos, 1);
  size_t idx = pos & q->mask;
  // wait until slot is ready
  size_t seq;
  while (true) {
    seq = dg_atomic_load(&q->seq[idx]);
    if (seq == pos) break;           // free slot
    if (seq < pos) return false;     // queue full
    // otherwise spin
  }
  // write data
  void* slot = q->pdata + (idx * q->elemsize);
  memcpy(slot, psrc, q->elemsize);
  // publish
  dg_atomic_store(&q->seq[idx], pos + 1);
  return true;
}

bool mpmc_queue_get_front(void *pdst, dg_mtqueue_mpmc_t* q)
{
  size_t pos = dg_atomic_fetch_add(&q->dequeue_pos, 1);
  size_t idx = pos & q->mask;
  // wait until data is ready
  size_t seq;
  while (true) {
    seq = dg_atomic_load(&q->seq[idx]);
    if (seq == pos + 1)
      break;       // data ready
    if (seq < pos + 1)
      return false; // queue empty
    // otherwise spin
  }
  // read data
  void* slot = q->pdata + (idx * q->elemsize);
  memcpy(pdst, slot, q->elemsize);
  // mark slot free
  dg_atomic_store(&q->seq[idx], pos + q->capacity);
  return true;
}

bool mpmc_queue_is_empty(dg_mtqueue_mpmc_t* q)
{
  size_t enq = dg_atomic_load(&q->enqueue_pos);
  size_t deq = dg_atomic_load(&q->dequeue_pos);
  return enq == deq;
}

bool mpmc_queue_is_full(dg_mtqueue_mpmc_t* q)
{
  size_t enq = dg_atomic_load(&q->enqueue_pos);
  size_t deq = dg_atomic_load(&q->dequeue_pos);
  return (enq - deq) >= q->capacity;
}

/**
* MT Queue
*/
bool mtqueue_alloc(dg_mtqueue_t* q, size_t elemsize, size_t capacity)
{
  q->elemsize = elemsize;
  q->capacity = capacity;
  q->pdata = (uint8_t*)calloc(q->capacity, q->elemsize);
  dg_atomic_store(&q->count, 0);
  q->head = 0;
  q->tail = 0;
  q->head_mtx = mutex_alloc("dg_mtqueue_t:head_mtx");
  q->tail_mtx = mutex_alloc("dg_mtqueue_t:tail_mtx");
  q->slots_sem = semaphore_alloc((int)capacity, (int)capacity, "dg_mtqueue_t:slots_sem");
  q->items_sem = semaphore_alloc(0, (int)capacity, "dg_mtqueue_t:items_sem");
  return q->pdata && 
    q->head_mtx && 
    q->tail_mtx &&
    q->slots_sem &&
    q->items_sem;
}

bool mtqueue_free(dg_mtqueue_t* q)
{
  if (q->pdata) {
    free(q->pdata);
    q->pdata = NULL;
  }
  if (q->head_mtx) {
    mutex_free(q->head_mtx);
    q->head_mtx = NULL;
  }
  if (q->tail_mtx) {
    mutex_free(q->tail_mtx);
    q->tail_mtx = NULL;
  }
  if (q->slots_sem) {
    semaphore_free(q->slots_sem);
    q->slots_sem = NULL;
  }
  if (q->items_sem) {
    semaphore_free(q->items_sem);
    q->items_sem = NULL;
  }
  return true;
}

void mtqueue_add_back(dg_mtqueue_t* q, const void* src)
{
  semaphore_wait(q->slots_sem);
  mutex_lock(q->tail_mtx);
  memcpy(q->pdata + q->tail * q->elemsize, src, q->elemsize);
  q->tail = (q->tail + 1) % q->capacity;
  dg_atomic_fetch_add(&q->count, 1);
  mutex_unlock(q->tail_mtx);
  semaphore_post(q->items_sem);
}

void* mtqueue_get_back(dg_mtqueue_t* q)
{
  semaphore_wait(q->items_sem);
  mutex_lock(q->tail_mtx);
  q->tail = (q->tail + q->capacity - 1) % q->capacity;
  dg_atomic_fetch_sub(&q->count, 1); //decrement
  mutex_unlock(q->tail_mtx);
  semaphore_post(q->slots_sem);
  return (q->pdata + q->tail * q->elemsize);
}

void mtqueue_add_front(dg_mtqueue_t* q, const void* src)
{
  semaphore_wait(q->slots_sem);
  mutex_lock(q->head_mtx);
  q->head = (q->head + q->capacity - 1) % q->capacity;
  memcpy(q->pdata + q->head * q->elemsize, src, q->elemsize);
  dg_atomic_fetch_add(&q->count, 1);
  mutex_unlock(q->head_mtx);
  semaphore_post(q->items_sem);
}

void* mtqueue_get_front(dg_mtqueue_t* q)
{
  semaphore_wait(q->items_sem);
  mutex_lock(q->head_mtx);
  void* elem = q->pdata + q->head * q->elemsize;
  q->head = (q->head + 1) % q->capacity;
  dg_atomic_fetch_sub(&q->count, 1); //dec
  mutex_unlock(q->head_mtx);
  semaphore_post(q->slots_sem);
  return elem;
}

bool mtqueue_try_add_back(dg_mtqueue_t* q, const void* psrc)
{
  if (!semaphore_trywait(q->slots_sem))
    return false;

  mutex_lock(q->tail_mtx);
  memcpy(q->pdata + q->tail * q->elemsize, psrc, q->elemsize);
  q->tail = (q->tail + 1) % q->capacity;
  dg_atomic_fetch_add(&q->count, 1);
  mutex_unlock(q->tail_mtx);
  semaphore_post(q->items_sem);
  return true;
}

bool mtqueue_try_add_front(dg_mtqueue_t* q, const void* src)
{
  if (!semaphore_trywait(q->slots_sem))
    return false;

  mutex_lock(q->head_mtx);
  q->head = (q->head + q->capacity - 1) % q->capacity;
  memcpy(q->pdata + q->head * q->elemsize, src, q->elemsize);
  dg_atomic_fetch_add(&q->count, 1);
  mutex_unlock(q->head_mtx);
  semaphore_post(q->items_sem);
  return true;
}

bool mtqueue_is_empty(dg_mtqueue_t* pqueue)
{
  return dg_atomic_load(&pqueue->count) == 0;
}

bool mtqueue_is_full(dg_mtqueue_t* pqueue)
{
  return dg_atomic_load(&pqueue->count) >= pqueue->capacity;
}
