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
  if (NULL == the_pool) {
    errno = ENOMEM;
    return NULL;
  }
  memset(the_pool, 0, sizeof(*the_pool));

  the_pool->cfg = *params;

  ER_INFO("Init memory pool: max_elts=%zu,elt_size=%zu",
          the_pool->cfg.max_elts,
          the_pool->cfg.elt_size);

  /*
   * Each step is undone in reverse on failure. The embedded llists, csem and
   * mutex are only zero-filled until their *_init() succeeds, and destroying
   * one before then is undefined (a zeroed llist would even free() its own
   * handle, which lives inside the pool), so mpool_destroy() can't be used.
   */

  /* allocate space for pool elements */
  the_pool->elements =
    rcsw_alloc(params->elements,
               params->max_elts * params->elt_size,
               params->flags & (RCSW_NOALLOC_DATA | RCSW_ZALLOC));
  if (NULL == the_pool->elements) {
    goto free_pool;
  }

  /* allocate space for free/alloc list nodes */
  the_pool->meta = rcsw_alloc(params->meta,
                              llist_meta_space(params->max_elts) * 2,
                              params->flags & (RCSW_NOALLOC_META | RCSW_ZALLOC));
  if (NULL == the_pool->meta) {
    goto free_elements;
  }

  struct llist_config llist_config = {
    .max_elts = (int)params->max_elts,
    .elt_size = params->elt_size,
    .cmpe     = NULL,
    .meta     = the_pool->meta,
    .flags    = RCSW_DS_LLIST_DB_DISOWN | RCSW_DS_LLIST_DB_PTR |
             RCSW_NOALLOC_HANDLE | RCSW_NOALLOC_META,
  };

  /* initialize free/alloc lists */
  if (NULL == llist_init(&the_pool->free, &llist_config)) {
    goto free_meta;
  }
  for (size_t i = 0; i < the_pool->cfg.max_elts; ++i) {
    if (OK != llist_append(
                &the_pool->free,
                (uint8_t*)the_pool->elements + (i * the_pool->cfg.elt_size))) {
      goto destroy_free;
    }
  } /* for() */
  size_t llist_meta_bytes = llist_meta_space(params->max_elts);

  /*
   * Spurious cast alignment warning: the target and destination pointers are
   * of the same alignment, we just need to convert to 1-byte alignment to get
   * the math to work.
   */
  llist_config.meta = (void*)((uint8_t*)the_pool->meta + llist_meta_bytes);
  if (NULL == llist_init(&the_pool->alloc, &llist_config)) {
    goto destroy_free;
  }

  /* initialize reference counting */
  if (params->flags & RCSW_NOALLOC_META) {
    the_pool->refs = (void*)((uint8_t*)the_pool->meta + (llist_meta_bytes * 2));
  } else {
    the_pool->refs = rcsw_alloc(NULL, params->max_elts * sizeof(int), RCSW_NONE);
  }
  if (NULL == the_pool->refs) {
    goto destroy_alloc;
  }
  /* Every chunk starts free (see MPOOL_REF_FREE) */
  for (size_t i = 0; i < params->max_elts; ++i) {
    the_pool->refs[i] = MPOOL_REF_FREE;
  } /* for(i..) */

  /* initialize locks */
  if (NULL == csem_init(&the_pool->slots_avail,
                        the_pool->cfg.max_elts,
                        RCSW_NOALLOC_HANDLE)) {
    goto free_refs;
  }
  if (NULL == mutex_init(&the_pool->mutex, RCSW_NOALLOC_HANDLE)) {
    goto destroy_csem;
  }
  return the_pool;

destroy_csem:
  csem_destroy(&the_pool->slots_avail);
free_refs:
  rcsw_free(the_pool->refs, the_pool->cfg.flags & RCSW_NOALLOC_META);
destroy_alloc:
  llist_destroy(&the_pool->alloc);
destroy_free:
  llist_destroy(&the_pool->free);
free_meta:
  rcsw_free(the_pool->meta, the_pool->cfg.flags & RCSW_NOALLOC_META);
free_elements:
  rcsw_free(the_pool->elements, the_pool->cfg.flags & RCSW_NOALLOC_DATA);
free_pool:
  rcsw_free(the_pool, the_pool->cfg.flags & RCSW_NOALLOC_HANDLE);
  errno = ENOMEM;
  return NULL;
} /* mpool_init() */

void mpool_destroy(struct mpool* const the_pool) {
  RCSW_FPC_V(NULL != the_pool);

  csem_destroy(&the_pool->slots_avail);
  mutex_destroy(&the_pool->mutex);

  llist_destroy(&the_pool->free);
  llist_destroy(&the_pool->alloc);

  rcsw_free(the_pool->elements, the_pool->cfg.flags & RCSW_NOALLOC_DATA);
  rcsw_free(the_pool->meta, the_pool->cfg.flags & RCSW_NOALLOC_META);
  rcsw_free(the_pool->refs, the_pool->cfg.flags & RCSW_NOALLOC_META);
  rcsw_free(the_pool, the_pool->cfg.flags & RCSW_NOALLOC_HANDLE);
}

/**
 * \brief Move a free chunk to the allocated list and give it one reference.
 *
 * The caller has already taken a slot from \ref mpool.slots_avail, so a free
 * chunk exists.
 */
static void* mpool_take_chunk(struct mpool* const the_pool) {
  mutex_lock(&the_pool->mutex);

  /* get the entry from free list and add to allocated list */
  void* ptr = the_pool->free.first->data;
  llist_remove(&the_pool->free, ptr);
  llist_append(&the_pool->alloc, ptr);

  /* One more THING using this chunk */
  size_t idx = (size_t)((uint8_t*)ptr - (uint8_t*)the_pool->elements) /
               the_pool->cfg.elt_size;
  the_pool->refs[idx] = 1; /* a fresh allocation has one owner */

  mutex_unlock(&the_pool->mutex);

  ER_DEBUG("Got buffer %p, n_free=%zu,n_alloc=%zu",
           ptr,
           llist_size(&the_pool->free),
           llist_size(&the_pool->alloc));
  return ptr;
} /* mpool_take_chunk() */

void* mpool_req(struct mpool* const the_pool) {
  RCSW_FPC_NV(NULL, NULL != the_pool);

  ER_DEBUG("Wait for buffer to become available: n_free=%zu,n_alloc=%zu",
           llist_size(&the_pool->free),
           llist_size(&the_pool->alloc));

  /* wait for an entry to become available */
  csem_wait(&the_pool->slots_avail);
  return mpool_take_chunk(the_pool);
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
  *chunk = mpool_take_chunk(the_pool);
  return OK;

error:
  return ERROR;
} /* mpool_timedreq() */

status_t mpool_tryreq(struct mpool* const the_pool, void** chunk) {
  RCSW_FPC_NV(ERROR, NULL != the_pool, NULL != chunk);

  if (OK != csem_trywait(&the_pool->slots_avail)) {
    errno = ENOSPC; /* none free: don't wait */
    return ERROR;
  }
  *chunk = mpool_take_chunk(the_pool);
  return OK;
} /* mpool_tryreq() */

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
  if (offset >= the_pool->cfg.elt_size * the_pool->cfg.max_elts ||
      0 != offset % the_pool->cfg.elt_size) {
    return -1;
  }
  return (int)(offset / the_pool->cfg.elt_size);
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
