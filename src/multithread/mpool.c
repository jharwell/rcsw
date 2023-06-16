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

#include "rcsw/common/dbg.h"
#include "rcsw/common/fpc.h"
#include "rcsw/rcsw.h"

/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS

struct mpool* mpool_init(struct mpool* const pool_in,
                         const struct mpool_params* const params) {
  RCSW_FPC_NV(NULL, params != NULL, params->max_elts > 0, params->elt_size > 0);

  struct mpool* the_pool = NULL;
  if (params->flags & RCSW_DS_NOALLOC_HANDLE ||
      params->flags & MT_APP_DOMAIN_MEM) {
    RCSW_CHECK_PTR(pool_in);
    the_pool = pool_in;
  } else {
    the_pool = calloc(1, sizeof(struct mpool));
    RCSW_CHECK_PTR(the_pool);
  }
  the_pool->flags = params->flags;
  if (params->flags & RCSW_DS_NOALLOC_DATA || params->flags & MT_APP_DOMAIN_MEM) {
    RCSW_CHECK_PTR(params->elements);
    the_pool->elements = params->elements;
  } else {
    the_pool->elements = calloc(params->max_elts, params->elt_size);
    RCSW_CHECK_PTR(the_pool->elements);
  }

  if (params->flags & RCSW_DS_NOALLOC_NODES ||
      params->flags & MT_APP_DOMAIN_MEM) {
    RCSW_CHECK_PTR(params->nodes);
    the_pool->nodes = params->nodes;
  } else {
    the_pool->nodes =
        (uint8_t*)calloc(params->max_elts, sizeof(struct llist_node));
    RCSW_CHECK_PTR(the_pool->nodes);
  }

  the_pool->flags = params->flags;
  the_pool->elt_size = params->elt_size;
  the_pool->n_free = params->max_elts;
  the_pool->max_elts = params->max_elts;
  the_pool->n_alloc = 0;

  struct ds_params llist_params = {
    .max_elts = (int)params->max_elts,
    .elt_size = params->elt_size,
    .cmpe = NULL,
    .tag = ekRCSW_DS_LLIST,
    .nodes = params->nodes,
    .flags = RCSW_DS_LLIST_NO_DB | RCSW_DS_LLIST_PTR_CMP |
             RCSW_DS_NOALLOC_HANDLE | (params->flags & RCSW_DS_NOALLOC_NODES),
  };
  /* initialize free/alloc lists */
  RCSW_CHECK_PTR(llist_init(&the_pool->free, &llist_params));
  llist_params.nodes = params->nodes + llist_node_space(params->max_elts);
  RCSW_CHECK_PTR(llist_init(&the_pool->alloc, &llist_params));

  size_t i;
  for (i = 0; i < params->max_elts; ++i) {
    RCSW_CHECK(OK == llist_append(&the_pool->free,
                                  the_pool->elements + i * the_pool->elt_size));
  } /* for() */

  /* initialize reference counting */
  the_pool->refs = calloc(the_pool->max_elts, sizeof(int));
  RCSW_CHECK_PTR(the_pool->refs);

  /* initialize locks */
  RCSW_CHECK_PTR(
      mt_csem_init(&the_pool->sem, FALSE, the_pool->max_elts, MT_APP_DOMAIN_MEM));
  RCSW_CHECK_PTR(mt_mutex_init(&the_pool->mutex, MT_APP_DOMAIN_MEM));

  return the_pool;

error:
  mpool_destroy(the_pool);
  errno = EAGAIN;
  return NULL;
} /* mpool_init() */

void mpool_destroy(struct mpool* const the_pool) {
  RCSW_FPC_V(NULL != the_pool);

  llist_destroy(&the_pool->free);
  llist_destroy(&the_pool->alloc);

  if (!(the_pool->flags & RCSW_DS_NOALLOC_DATA)) {
    free(the_pool->elements);
  }
  if (!(the_pool->flags & RCSW_DS_NOALLOC_NODES)) {
    free(the_pool->nodes);
  }
  if (the_pool->refs) {
    free(the_pool->refs);
  }
  if (!(the_pool->flags & RCSW_DS_NOALLOC_HANDLE)) {
    free(the_pool);
  }
} /* mpool_destroy() */

uint8_t* mpool_req(struct mpool* const the_pool) {
  RCSW_FPC_NV(NULL, NULL != the_pool);

  uint8_t* ptr = NULL;

  /* wait for an entry to become available */
  mt_csem_wait(&the_pool->sem);
  mt_mutex_lock(&the_pool->mutex);

  /* get the entry from free list and add to allocated list */
  ptr = the_pool->free.first->data;
  llist_remove(&the_pool->free, the_pool->free.first->data);
  llist_append(&the_pool->alloc, ptr);
  the_pool->n_alloc++;
  the_pool->n_free--;

  /* One more person using this chunk */
  the_pool->refs[(size_t)(ptr - the_pool->elements) / the_pool->elt_size]++;

  mt_mutex_unlock(&the_pool->mutex);
  return ptr;
} /* mpool_req() */

status_t mpool_release(struct mpool* const the_pool, uint8_t* const ptr) {
  RCSW_FPC_NV(ERROR, NULL != the_pool, NULL != ptr);

  size_t index = (size_t)(ptr - the_pool->elements) / the_pool->elt_size;
  mt_mutex_lock(&the_pool->mutex);

  /*
   * One less person using this chunk. The reference count may have been
   * decreased to 0 by a call to ref_remove, so use RCSW_MAX() so stay
   * non-negative.
   */
  the_pool->refs[index] = RCSW_MAX(0, the_pool->refs[index] - 1);

  /* Some else is still using this chunk--don't free it yet */
  if (((the_pool->flags & MPOOL_REF_COUNT_EN) && the_pool->refs[index] > 0)) {
    mt_mutex_unlock(&the_pool->mutex);
    return OK;
  }
  llist_remove(&the_pool->alloc, ptr);
  llist_append(&the_pool->free, ptr);
  mt_csem_post(&the_pool->sem);
  the_pool->n_free++;
  the_pool->n_alloc--;

  mt_mutex_unlock(&the_pool->mutex);
  return OK;
} /* mpool_release() */

status_t mpool_ref_add(struct mpool* const the_pool, const uint8_t* const ptr) {
  RCSW_FPC_NV(ERROR, NULL != the_pool, NULL != ptr);

  status_t rstat = ERROR;
  mt_mutex_lock(&the_pool->mutex);
  int index = mpool_ref_query(the_pool, ptr);
  RCSW_CHECK(-1 != index);
  the_pool->refs[index]++;
  rstat = OK;

error:
  mt_mutex_unlock(&the_pool->mutex);
  return rstat;
} /* mpool_ref_add() */

status_t mpool_ref_remove(struct mpool* const the_pool,
                          const uint8_t* const ptr) {
  RCSW_FPC_NV(ERROR, NULL != the_pool, NULL != ptr);

  status_t rstat = ERROR;
  mt_mutex_lock(&the_pool->mutex);
  int index = mpool_ref_query(the_pool, ptr);
  RCSW_CHECK(-1 != index);
  the_pool->refs[index]--;
  rstat = OK;

error:
  mt_mutex_unlock(&the_pool->mutex);
  return rstat;
} /* mpool_ref_remove() */

int mpool_ref_query(struct mpool* const the_pool, const uint8_t* const ptr) {
  RCSW_FPC_NV(-1, NULL != the_pool, NULL != ptr);

  /*
   * If this is not true, then ptr did not come from this pool, or has not
   * yet been allocated.
   */
  RCSW_CHECK(NULL != llist_data_query(&the_pool->alloc, ptr));
  return (int)((size_t)(ptr - the_pool->elements) / the_pool->elt_size);

error:
  return -1;
} /* mpool_ref_query() */

END_C_DECLS
