/**
 * \file mpool.c
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/multithread/mpool.h"

#define RCSW_ER_MODNAME RCSW_ER_MODNAME_BUILDER("rcsw", "mt", "mpool")
#define RCSW_ER_MODID ekLOG4CL_MT_MPOOL
#include "rcsw/er/client.h"
#include "rcsw/common/fpc.h"
#include "rcsw/rcsw.h"
#include "rcsw/common/alloc.h"

/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS

struct mpool* mpool_init(struct mpool* const pool_in,
                         const struct mpool_params* const params) {
  RCSW_FPC_NV(NULL, params != NULL, params->max_elts > 0, params->elt_size > 0);
  RCSW_ER_MODULE_INIT();

  struct mpool* the_pool = rcsw_alloc(pool_in,
                                      sizeof(struct mpool),
                                      params->flags & RCSW_NOALLOC_HANDLE);

  RCSW_CHECK_PTR(the_pool);

  the_pool->flags = params->flags;
  the_pool->elt_size = params->elt_size;
  the_pool->max_elts = params->max_elts;
  the_pool->flags = params->flags;

  ER_INFO("Init memory pool: max_elts=%zu,elt_size=%zu",
          the_pool->max_elts,
          the_pool->elt_size);

  /* allocate space for pool elements */
  the_pool->elements = rcsw_alloc(params->elements,
                                  params->max_elts * params->elt_size,
                                  params->flags & RCSW_NOALLOC_DATA);
  RCSW_CHECK_PTR(the_pool->elements);

  /* allocate space for free/alloc list nodes */
  the_pool->nodes = rcsw_alloc(params->meta,
                               llist_meta_space(params->max_elts) * 2,
                               params->flags & RCSW_NOALLOC_META);
  RCSW_CHECK_PTR(the_pool->nodes);

  struct llist_params llist_params = {
    .max_elts = params->max_elts,
    .elt_size = params->elt_size,
    .cmpe = NULL,
    .meta = (dptr_t*)the_pool->nodes,
    .flags = RCSW_DS_LLIST_DB_DISOWN | RCSW_DS_LLIST_DB_PTR |
             RCSW_NOALLOC_HANDLE | RCSW_NOALLOC_META,
  };

  /* initialize free/alloc lists */
  RCSW_CHECK_PTR(llist_init(&the_pool->free, &llist_params));
  for (size_t i = 0; i < the_pool->max_elts; ++i) {
    RCSW_CHECK(OK == llist_append(&the_pool->free,
                                  (uint8_t*)the_pool->elements + i * the_pool->elt_size));

  } /* for() */
  size_t n_bytes = llist_meta_space(params->max_elts);

  /*
   * Spurious cast alignment warning: the target and destination pointers are
   * of the same alignment, we just need to convert to 1-byte alignment to get
   * the math to work.
   */
  llist_params.meta = (void*)((uint8_t*)the_pool->nodes + n_bytes);
  RCSW_CHECK_PTR(llist_init(&the_pool->alloc, &llist_params));

  /* initialize reference counting */
  the_pool->refs = rcsw_alloc(NULL,
                              params->max_elts * sizeof(int),
                              RCSW_ZALLOC);
  RCSW_CHECK_PTR(the_pool->refs);

  /* initialize locks */
  RCSW_CHECK_PTR(csem_init(&the_pool->slots_avail,
                           the_pool->max_elts,
                           RCSW_NOALLOC_HANDLE));
  RCSW_CHECK_PTR(mutex_init(&the_pool->mutex, RCSW_NOALLOC_HANDLE));

  return the_pool;

error:
  mpool_destroy(the_pool);
  errno = EAGAIN;
  return NULL;
} /* mpool_init() */

void mpool_destroy(struct mpool* const the_pool) {
  RCSW_FPC_V(NULL != the_pool);

  csem_destroy(&the_pool->slots_avail);
  mutex_destroy(&the_pool->mutex);

  llist_destroy(&the_pool->free);
  llist_destroy(&the_pool->alloc);

  rcsw_free(the_pool->elements, the_pool->flags & RCSW_NOALLOC_DATA);
  rcsw_free(the_pool->nodes, the_pool->flags & RCSW_NOALLOC_META);

  rcsw_free(the_pool->refs, RCSW_NONE);
  rcsw_free(the_pool, the_pool->flags & RCSW_NOALLOC_HANDLE);
} /* mpool_destroy() */

void* mpool_req(struct mpool* const the_pool) {
  RCSW_FPC_NV(NULL, NULL != the_pool);

  void* ptr = NULL;
  ER_DEBUG("Wait for buffer to become available: n_free=%zu,n_alloc=%zu",
           llist_size(&the_pool->free),
           llist_size(&the_pool->alloc));

  /* wait for an entry to become available */
  csem_wait(&the_pool->slots_avail);
  mutex_lock(&the_pool->mutex);

  /* get the entry from free list and add to allocated list */
  ptr = the_pool->free.first->data;
  llist_remove(&the_pool->free, the_pool->free.first->data);
  llist_append(&the_pool->alloc, ptr);

  /* One more THING using this chunk */
  size_t idx = (size_t)((uint8_t*)ptr - (uint8_t*)the_pool->elements) / the_pool->elt_size;
  the_pool->refs[idx]++;

  mutex_unlock(&the_pool->mutex);

  ER_DEBUG("Got buffer %p,ref_count=%d, n_free=%zu,n_alloc=%zu",
           ptr,
           the_pool->refs[idx],
           llist_size(&the_pool->free),
           llist_size(&the_pool->alloc));

  return ptr;
} /* mpool_req() */

status_t mpool_timedreq(struct mpool* const the_pool,
                        const struct timespec* const to,
                        void** chunk) {
  RCSW_FPC_NV(ERROR, NULL != the_pool, NULL != to);

  ER_DEBUG("Wait for buffer to become available: n_free=%zu,n_alloc=%zu",
           llist_size(&the_pool->free),
           llist_size(&the_pool->alloc));
  /* wait for an entry to become available */
  RCSW_CHECK(OK == csem_timedwait(&the_pool->slots_avail, to));
  mutex_lock(&the_pool->mutex);

  /* Remove the entry from free list and add to allocated list */
  dptr_t* ptr = (dptr_t*)the_pool->free.first->data;
  llist_remove(&the_pool->free, ptr);
  llist_append(&the_pool->alloc, ptr);

  /* One more THING using this chunk */
  size_t idx = ((uint8_t*)ptr - (uint8_t*)the_pool->elements) / the_pool->elt_size;
  the_pool->refs[idx]++;

  if (NULL != chunk) {
    *chunk = (dptr_t*)ptr;
  }

  mutex_unlock(&the_pool->mutex);

  ER_DEBUG("Got buffer %p,ref_count=%d, n_free=%zu,n_alloc=%zu",
           ptr,
           the_pool->refs[idx],
           llist_size(&the_pool->free),
           llist_size(&the_pool->alloc));
  return OK;

error:
  return ERROR;
} /* mpool_timedreq() */

status_t mpool_release(struct mpool* const the_pool, void* const ptr) {
  RCSW_FPC_NV(ERROR, NULL != the_pool, NULL != ptr);

  ER_DEBUG("Attempting release of buf=%p", ptr);

  int index = mpool_ref_query(the_pool, ptr);
  ER_CHECK(-1 != index,
           "Buffer %p not found", ptr);

  mutex_lock(&the_pool->mutex);

  /*
   * One less person using this chunk. The reference count may have been
   * decreased to 0 by a call to mpool_ref_remove(), so use RCSW_MAX() so stay
   * non-negative.
   */
  the_pool->refs[index] = RCSW_MAX(0, the_pool->refs[index] - 1);

  /* Someone else is still using this chunk--don't free it yet */
  if (the_pool->refs[index] > 0) {
    mutex_unlock(&the_pool->mutex);
    ER_DEBUG("Buffer %p,idx=%d not ready for release: refcount=%d",
             ptr,
             index,
             the_pool->refs[index]);

    return OK;
  }
  llist_remove(&the_pool->alloc, ptr);
  llist_append(&the_pool->free, ptr);
  csem_post(&the_pool->slots_avail);

  mutex_unlock(&the_pool->mutex);

  ER_DEBUG("Released buffer %p, n_free=%zu,n_alloc=%zu",
           ptr,
           llist_size(&the_pool->free),
           llist_size(&the_pool->alloc));

  return OK;

error:
  ER_DEBUG("Failed to release buffer %p", ptr);
  return ERROR;
} /* mpool_release() */

status_t mpool_ref_add(struct mpool* const the_pool, const void* const ptr) {
  RCSW_FPC_NV(ERROR, NULL != the_pool, NULL != ptr);

  status_t rstat = ERROR;
  mutex_lock(&the_pool->mutex);
  int index = mpool_ref_query(the_pool, ptr);
  ER_CHECK(-1 != index, "Buffer %p not found", ptr);

  the_pool->refs[index]++;
  ER_DEBUG("%s: buffer %p new refcount=%d",
           __FUNCTION__,
           ptr,
           the_pool->refs[index]);

  rstat = OK;

error:
  mutex_unlock(&the_pool->mutex);
  return rstat;
} /* mpool_ref_add() */

status_t mpool_ref_remove(struct mpool* const the_pool,
                          const void* const ptr) {
  RCSW_FPC_NV(ERROR, NULL != the_pool, NULL != ptr);

  status_t rstat = ERROR;
  mutex_lock(&the_pool->mutex);
  int index = mpool_ref_query(the_pool, ptr);
  ER_CHECK(-1 != index, "Buffer %p not found", ptr);

  the_pool->refs[index]--;
  ER_DEBUG("%s: buffer %p new refcount=%d",
           __FUNCTION__,
           ptr,
           the_pool->refs[index]);

  rstat = OK;

error:
  mutex_unlock(&the_pool->mutex);
  return rstat;
} /* mpool_ref_remove() */

int mpool_ref_query(struct mpool* const the_pool, const void* const ptr) {
  RCSW_FPC_NV(-1, NULL != the_pool, NULL != ptr);

  /*
   * If this is not true, then ptr did not come from this pool. Note that we
   * don't query the alloc llist--we are not necessarily protected by a mutex
   * here, and the llist can be modified elsewhere.
   */
  size_t memsize = mpool_element_space(the_pool->elt_size, the_pool->max_elts);
  RCSW_CHECK(RCSW_IS_BETWEENHO((const uint8_t*)ptr,
                               (uint8_t*)the_pool->elements,
                               (uint8_t*)the_pool->elements + memsize));
  return ((const uint8_t*)ptr - (uint8_t*)the_pool->elements) / the_pool->elt_size;

error:
  return -1;
} /* mpool_ref_query() */

size_t mpool_ref_count(struct mpool* const the_pool, const void* const ptr) {
  RCSW_FPC_NV(-1, NULL != the_pool, NULL != ptr);

  /*
   * If this is not true, then ptr did not come from this pool, or has not
   * yet been allocated.
   */
  int idx = mpool_ref_query(the_pool, ptr);
  RCSW_CHECK(-1 != idx);
  return the_pool->refs[idx];

error:
  return 0;
} /* mpool_ref_count() */

END_C_DECLS
