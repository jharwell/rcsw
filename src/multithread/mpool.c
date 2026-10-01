/**
 * \file
 *
 * \copyright 2017 John Harwell
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/multithread/mpool.h"

#include <errno.h>
#include <stdint.h>

#include "rcsw/ds/llist.h"

#define RCSW_ER_MODNAME RCSW_ER_MODNAME_BUILDER("rcsw", "mt", "mpool")
#define RCSW_ER_MODID LOG4CL_MT_MPOOL
#include <string.h>

#include "rcsw/core/alloc.h"
#include "rcsw/core/compilers.h"
#include "rcsw/core/fpc.h"
#include "rcsw/er/client.h"

/*******************************************************************************
 * Public API
 ******************************************************************************/
BEGIN_C_DECLS

/** Reference count marking a chunk that is not allocated. */
#define MPOOL_REF_FREE (-1)

struct mpool* mpool_init(struct mpool* const              pool_in,
                         const struct mpool_config* const params) {
  RCSW_FPC_NV(NULL, params != NULL, params->max_elts > 0, params->elt_size > 0);
  RCSW_ER_MODULE_INIT();

  struct mpool* the_pool =
    rcsw_alloc(pool_in,
               sizeof(struct mpool),
               params->flags & (RCSW_NOALLOC_HANDLE | RCSW_ZALLOC));

  RCSW_CHECK_PTR(the_pool);
  memset(the_pool, 0, sizeof(*the_pool));

  the_pool->flags    = params->flags;
  the_pool->elt_size = params->elt_size;
  the_pool->max_elts = params->max_elts;

  ER_INFO("Init memory pool: max_elts=%zu,elt_size=%zu",
          the_pool->max_elts,
          the_pool->elt_size);

  /* allocate space for pool elements */
  the_pool->elements =
    rcsw_alloc(params->elements,
               params->max_elts * params->elt_size,
               params->flags & (RCSW_NOALLOC_DATA | RCSW_ZALLOC));
  RCSW_CHECK_PTR(the_pool->elements);

  /* allocate space for free/alloc list nodes */
  the_pool->meta = rcsw_alloc(params->meta,
                              llist_meta_space(params->max_elts) * 2,
                              params->flags & (RCSW_NOALLOC_META | RCSW_ZALLOC));
  RCSW_CHECK_PTR(the_pool->meta);

  struct llist_config llist_config = {
    .max_elts = (int)params->max_elts,
    .elt_size = params->elt_size,
    .cmpe     = NULL,
    .meta     = the_pool->meta,
    .flags    = RCSW_DS_LLIST_DB_DISOWN | RCSW_DS_LLIST_DB_PTR |
             RCSW_NOALLOC_HANDLE | RCSW_NOALLOC_META,
  };

  /* initialize free/alloc lists */
  RCSW_CHECK_PTR(llist_init(&the_pool->free, &llist_config));
  for (size_t i = 0; i < the_pool->max_elts; ++i) {
    RCSW_CHECK(OK == llist_append(
                       &the_pool->free,
                       (uint8_t*)the_pool->elements + (i * the_pool->elt_size)));

  } /* for() */
  size_t llist_meta_bytes = llist_meta_space(params->max_elts);

  /*
   * Spurious cast alignment warning: the target and destination pointers are
   * of the same alignment, we just need to convert to 1-byte alignment to get
   * the math to work.
   */
  llist_config.meta = (void*)((uint8_t*)the_pool->meta + llist_meta_bytes);
  RCSW_CHECK_PTR(llist_init(&the_pool->alloc, &llist_config));

  /* initialize reference counting */
  if (params->flags & RCSW_NOALLOC_META) {
    the_pool->refs = (void*)((uint8_t*)the_pool->meta + (llist_meta_bytes * 2));
    memset((uint8_t*)the_pool->meta + (llist_meta_bytes * 2),
           0,
           params->max_elts * sizeof(int));
  } else {
    the_pool->refs =
      rcsw_alloc(NULL, params->max_elts * sizeof(int), RCSW_ZALLOC);
  }

  RCSW_CHECK_PTR(the_pool->refs);
  /* Every chunk starts free (see MPOOL_REF_FREE) */
  for (size_t i = 0; i < params->max_elts; ++i) {
    the_pool->refs[i] = MPOOL_REF_FREE;
  } /* for(i..) */

  /* initialize locks */
  RCSW_CHECK_PTR(
    csem_init(&the_pool->slots_avail, the_pool->max_elts, RCSW_NOALLOC_HANDLE));
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
  rcsw_free(the_pool->meta, the_pool->flags & RCSW_NOALLOC_META);
  rcsw_free(the_pool->refs, the_pool->flags & RCSW_NOALLOC_META);
  rcsw_free(the_pool, the_pool->flags & RCSW_NOALLOC_HANDLE);
}

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
  size_t idx =
    (size_t)((uint8_t*)ptr - (uint8_t*)the_pool->elements) / the_pool->elt_size;
  the_pool->refs[idx] = 1; /* a fresh allocation has one owner */

  mutex_unlock(&the_pool->mutex);

  ER_DEBUG("Got buffer %p,ref_count=%d, n_free=%zu,n_alloc=%zu",
           ptr,
           the_pool->refs[idx],
           llist_size(&the_pool->free),
           llist_size(&the_pool->alloc));

  return ptr;
} /* mpool_req() */

status_t mpool_timedreq(struct mpool* const          the_pool,
                        const struct timespec* const to,
                        void**                       chunk) {
  RCSW_FPC_NV(ERROR, NULL != the_pool, NULL != to, NULL != chunk);

  ER_DEBUG("Wait for buffer to become available: n_free=%zu,n_alloc=%zu",
           llist_size(&the_pool->free),
           llist_size(&the_pool->alloc));
  /* wait for an entry to become available */
  RCSW_CHECK(OK == csem_timedwait(&the_pool->slots_avail, to));
  mutex_lock(&the_pool->mutex);

  /* Remove the entry from free list and add to allocated list */
  dptr_t* ptr = the_pool->free.first->data;
  llist_remove(&the_pool->free, ptr);
  llist_append(&the_pool->alloc, ptr);

  /* One more THING using this chunk */
  size_t idx =
    (size_t)((uint8_t*)ptr - (uint8_t*)the_pool->elements) / the_pool->elt_size;
  the_pool->refs[idx] = 1; /* a fresh allocation has one owner */

  if (NULL != chunk) {
    *chunk = ptr;
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
  ER_CHECK(-1 != index, "Buffer %p not found", ptr);

  mutex_lock(&the_pool->mutex);

  /* Releasing a chunk that is not allocated would corrupt the free list */
  if (MPOOL_REF_FREE == the_pool->refs[index]) {
    mutex_unlock(&the_pool->mutex);
    ER_ERR("Buffer %p is not allocated", ptr);
    errno = EINVAL;
    return ERROR;
  }

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
  the_pool->refs[index] = MPOOL_REF_FREE;
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

  ER_CHECK(the_pool->refs[index] >= 0, "Buffer %p is not allocated", ptr);
  the_pool->refs[index]++;
  ER_DEBUG("%s: buffer %p new refcount=%d", __func__, ptr, the_pool->refs[index]);

  rstat = OK;

error:
  mutex_unlock(&the_pool->mutex);
  return rstat;
} /* mpool_ref_add() */

status_t mpool_ref_remove(struct mpool* const the_pool, const void* const ptr) {
  RCSW_FPC_NV(ERROR, NULL != the_pool, NULL != ptr);

  status_t rstat = ERROR;
  mutex_lock(&the_pool->mutex);
  int index = mpool_ref_query(the_pool, ptr);
  ER_CHECK(-1 != index, "Buffer %p not found", ptr);

  ER_CHECK(the_pool->refs[index] >= 0, "Buffer %p is not allocated", ptr);
  /* Never frees the chunk (only mpool_release() does), so stop at 0 */
  the_pool->refs[index] = RCSW_MAX(0, the_pool->refs[index] - 1);
  ER_DEBUG("%s: buffer %p new refcount=%d", __func__, ptr, the_pool->refs[index]);

  rstat = OK;

error:
  mutex_unlock(&the_pool->mutex);
  return rstat;
} /* mpool_ref_remove() */

int mpool_ref_query(struct mpool* const the_pool, const void* const ptr) {
  RCSW_FPC_NV(-1, NULL != the_pool, NULL != ptr);

  /*
   * ptr must be the start of one of this pool's chunks. Note that we don't
   * query the alloc llist--we are not necessarily protected by a mutex here,
   * and the llist can be modified elsewhere.
   */
  const uint8_t* base = (const uint8_t*)the_pool->elements;
  const uint8_t* p    = (const uint8_t*)ptr;
  if (p < base) {
    return -1;
  }
  size_t offset = (size_t)(p - base);
  if (offset >= the_pool->elt_size * the_pool->max_elts ||
      0 != offset % the_pool->elt_size) {
    return -1;
  }
  return (int)(offset / the_pool->elt_size);
} /* mpool_ref_query() */

size_t mpool_ref_count(struct mpool* const the_pool, const void* const ptr) {
  RCSW_FPC_NV(SIZE_MAX, NULL != the_pool, NULL != ptr);

  int idx = mpool_ref_query(the_pool, ptr);
  if (-1 == idx) {
    return SIZE_MAX; /* not from this pool */
  }
  /* a free chunk has no references */
  return (size_t)RCSW_MAX(0, the_pool->refs[idx]);
} /* mpool_ref_count() */

END_C_DECLS
