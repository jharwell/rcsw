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
#include "rcsw/swbus/swbus.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#define RCSW_ER_MODNAME RCSW_ER_MODNAME_BUILDER("rcsw", "swb")
#define RCSW_ER_MODID LOG4CL_SWBUS
#include "rcsw/core/alloc.h"
#include "rcsw/core/fpc.h"
#include "rcsw/er/client.h"

/*******************************************************************************
 * Private Functions
 ******************************************************************************/

static status_t swbus_subscriber_notify(struct swbus*         swb,
                                        struct mpool*         bp,
                                        struct swbus_sub*     sub,
                                        struct swbus_rxq_ent* rxq_ent) {
  /* bp is NULL for an application-built reservation: no refcounting then */
  RCSW_FPC_NV(ERROR,
              NULL != swb,
              NULL != sub,
              NULL != sub->subscriber,
              NULL != rxq_ent);

  ER_TRACE("Notifying RXQ %zu subscribed to PID %d/0x%x on bus '%s', pending=%zu",
           sub->subscriber - swb->rxqs,
           sub->pid,
           sub->pid,
           swb->name,
           pcqueue_size(sub->subscriber));

  /*
   * Take the subscriber's reference BEFORE the entry becomes visible to it:
   * otherwise it could pop and release the packet first, freeing it early.
   * (Not done during the reserve step, as no one is using the memory yet.)
   */
  if (NULL != bp) {
    RCSW_CHECK(OK == mpool_ref_add(bp, rxq_ent->data));
  }

  /*
   * Never block: the publisher holds the bus mutex (and, in sync mode, the
   * write lock that subscribers need in order to drain their queues), so
   * waiting for space here would deadlock.
   */
  if (OK != pcqueue_trypush(sub->subscriber, rxq_ent)) {
    if (NULL != bp) {
      mpool_ref_remove(bp, rxq_ent->data);
    }
    errno = ENOSPC;
    return ERROR;
  }
  return OK;

error:
  return ERROR;
} /* swbus_subscriber_notify() */

static int swbus_sub_cmp(const void* a, const void* b) {
  const struct swbus_sub* s1 = a;
  const struct swbus_sub* s2 = b;
  if (s1->pid < s2->pid) {
    return -1;
  }
  if (s1->pid > s2->pid) {
    return 1;
  }

  if (s1->subscriber < s2->subscriber) {
    return -1;
  }
  if (s1->subscriber > s2->subscriber) {
    return 1;
  }
  return 0;
}

/*******************************************************************************
 * Public API
 ******************************************************************************/
BEGIN_C_DECLS

struct swbus* swbus_init(struct swbus*              swb_in,
                         const struct swbus_config* params) {
  RCSW_FPC_NV(NULL,
              params != NULL,
              params->pools != NULL || 0 == params->max_pools,
              !(params->flags & RCSW_NOALLOC_META) || NULL != params->meta);
  RCSW_ER_MODULE_INIT();

  struct swbus* swb =
    rcsw_alloc(swb_in,
               sizeof(struct swbus),
               params->flags & (RCSW_NOALLOC_HANDLE | RCSW_ZALLOC));
  if (NULL == swb) {
    errno = ENOMEM;
    return NULL;
  }

  /* Everything swbus_destroy() looks at, before anything can fail */
  swb->flags       = params->flags;
  swb->pools       = NULL;
  swb->rxqs        = NULL;
  swb->subscribers = NULL;
  swb->n_pools     = 0; /* counts pools as they are initialized */
  swb->n_rxqs      = 0;
  swb->max_rxqs    = params->max_rxqs;
  swb->max_subs    = params->max_subs;

  if (NULL == mutex_init(&swb->mutex, RCSW_NOALLOC_HANDLE)) {
    goto free_handle;
  }
  if (NULL == rdwrl_init(&swb->syncl, RCSW_NOALLOC_HANDLE)) {
    goto destroy_mutex;
  }

  (void)snprintf(swb->name, RCSW_SWBUS_MAX_NAMELEN, "%s", params->name);
  ER_DEBUG("Initializing SWB instance '%s'", swb->name);
  ER_DEBUG("Initializing %zu buffer pools", params->max_pools);

  /*
   * With RCSW_NOALLOC_META, the pool/RXQ handle arrays and the subscriber list
   * are carved from the caller's meta space, in the order given by
   * swbus_meta_layout_calc(); otherwise they are allocated.
   */
  struct swbus_meta_layout layout =
    swbus_meta_layout_calc(params->max_pools, swb->max_rxqs, swb->max_subs);
  uint8_t* meta       = (uint8_t*)params->meta;
  uint32_t meta_flags = params->flags & RCSW_NOALLOC_META;

  /* initialize buffer pools */
  swb->pools = rcsw_alloc(meta_flags ? meta + layout.pools : NULL,
                          params->max_pools * sizeof(struct mpool),
                          meta_flags ? RCSW_NOALLOC_HANDLE : RCSW_NONE);
  RCSW_CHECK_PTR(swb->pools);

  for (size_t i = 0; i < params->max_pools; i++) {
    /* Don't modify the caller's config */
    struct mpool_config pool_config = params->pools[i];
    pool_config.flags |= RCSW_NOALLOC_HANDLE;
    RCSW_CHECK_PTR(mpool_init(&swb->pools[i], &pool_config));
    swb->n_pools++;
  } /* for() */

  ER_DEBUG("Allocating %zu receive queues, %zu max subscribers/queue",
           swb->max_rxqs,
           swb->max_subs);

  /* Allocate receive queues */
  swb->rxqs = rcsw_alloc(meta_flags ? meta + layout.rxqs : NULL,
                         swb->max_rxqs * sizeof(struct pcqueue),
                         meta_flags ? RCSW_NOALLOC_HANDLE : RCSW_NONE);
  RCSW_CHECK_PTR(swb->rxqs);

  /* Initialize subscriber list */
  struct llist_config llparams = {.max_elts = (int)swb->max_subs,
                                  .elt_size = sizeof(struct swbus_sub),
                                  .cmpe     = swbus_sub_cmp,
                                  .flags    = RCSW_DS_SORTED};
  struct llist*       list_in  = NULL;
  if (meta_flags) {
    list_in           = (struct llist*)(meta + layout.list);
    llparams.meta     = (dptr_t*)(meta + layout.list_meta);
    llparams.elements = (dptr_t*)(meta + layout.list_elements);
    llparams.flags |= RCSW_NOALLOC_HANDLE | RCSW_NOALLOC_META | RCSW_NOALLOC_DATA;
  }
  swb->subscribers = llist_init(list_in, &llparams);
  RCSW_CHECK_PTR(swb->subscribers);

  ER_DEBUG("Initialization complete for SWB instance '%s'", swb->name);
  return swb;

error:
  swbus_destroy(swb); /* everything it touches is initialized or NULL */
  errno = ENOMEM;
  return NULL;

  /* Undo only what was initialized; errno is already set */
destroy_mutex:
  mutex_destroy(&swb->mutex);
free_handle:
  rcsw_free(swb, params->flags & RCSW_NOALLOC_HANDLE);
  return NULL;
} /* swbus_init() */

void swbus_destroy(struct swbus* swb) {
  RCSW_FPC_V(NULL != swb);

  if (swb->pools) {
    for (size_t i = 0; i < swb->n_pools; ++i) {
      mpool_destroy(&swb->pools[i]);
    } /* for(i..) */
    rcsw_free(swb->pools,
              (swb->flags & RCSW_NOALLOC_META) ? RCSW_NOALLOC_HANDLE : RCSW_NONE);
  }
  if (swb->rxqs) {
    for (size_t i = 0; i < swb->n_rxqs; ++i) {
      pcqueue_destroy(&swb->rxqs[i]);
    } /* for(i..) */
    rcsw_free(swb->rxqs,
              (swb->flags & RCSW_NOALLOC_META) ? RCSW_NOALLOC_HANDLE : RCSW_NONE);
  }
  if (swb->subscribers) {
    llist_destroy(swb->subscribers);
  }
  rdwrl_destroy(&swb->syncl);
  mutex_destroy(&swb->mutex);
  rcsw_free(swb, swb->flags & RCSW_NOALLOC_HANDLE);
} /* swbus_destroy() */

status_t swbus_publish(struct swbus* swb,
                       uint32_t      pid,
                       size_t        pkt_size,
                       const void*   pkt) {
  RCSW_FPC_NV(ERROR, NULL != swb, pkt_size > 0, NULL != pkt);

  ER_DEBUG("Publishing to bus '%s': PID=%d/0x%x, pkt=%p, pkt_size=%zu",
           swb->name,
           pid,
           pid,
           pkt,
           pkt_size);

  /* get space on the software bus for the packet */
  struct swbus_rsrvn res;
  RCSW_CHECK(OK == swbus_publish_reserve(swb, &res, pkt_size));

  /* the actual publish: copy the packet to the allocated buffer */
  memcpy(res.data, pkt, pkt_size);

  /* release the allocated buffer (i.e. push to receive queues) */
  ER_CHECK(OK == swbus_publish_release(swb, pid, &res),
           "Could not release buffer for publish for PID=%d/0x%x, pkt_size=%zu "
           "on bus '%s'",
           pid,
           pid,
           pkt_size,
           swb->name);
  return OK;

error:
  return ERROR;
} /* swbus_publish() */

status_t swbus_publish_reserve(struct swbus*       swb,
                               struct swbus_rsrvn* res,
                               size_t              pkt_size) {
  RCSW_FPC_NV(ERROR, NULL != swb, NULL != res, pkt_size > 0);

  ER_DEBUG("Reserving %zu byte buffer on bus '%s'", pkt_size, swb->name);

  for (size_t i = 0; i < swb->n_pools; i++) {
    struct mpool* pool = &swb->pools[i];

    /* can't use this buffer pool--buffers are too small or pool is full */
    if (pool->elt_size < pkt_size || mpool_isfull(pool)) {
      ER_TRACE(
        "Skipping buffer pool %zu in reservation search: buf_size=%zu, "
        "pkt_size=%zu, full=%d",
        i,
        pool->elt_size,
        pkt_size,
        mpool_isfull(pool));
      continue;
    }
    dptr_t* space = mpool_req(pool);
    if (NULL != space) {
      res->data     = space;
      res->pkt_size = pkt_size;
      res->bp       = swb->pools + i;
      return OK;
    }
  } /*  for(i...) */

  /* no free buffer big enough found */
  ER_DEBUG("Failed to reserve %zu byte buffer on bus '%s'", pkt_size, swb->name);
  errno = ENOSPC;
  return ERROR;
} /* swbus_publish_reserve() */

status_t swbus_publish_release(struct swbus*       swb,
                               uint32_t            pid,
                               struct swbus_rsrvn* res) {
  RCSW_FPC_NV(ERROR, NULL != swb, NULL != res, res->pkt_size > 0);
  status_t             rstat = OK;
  struct swbus_rxq_ent rxq_entry;

  rxq_entry.data     = res->data;
  rxq_entry.bp       = res->bp;
  rxq_entry.pkt_size = res->pkt_size;
  rxq_entry.pid      = pid;

  mutex_lock(&swb->mutex);
  ER_TRACE("Releasing published data for PID=%d/0x%x on bus '%s'",
           pid,
           pid,
           swb->name);

  /* Keep application threads from servicing until all subscribers notified */
  if (!(swb->flags & RCSW_SWBUS_ASYNC)) {
    rdwrl_req(&swb->syncl, SCOPE_WR);
  }

  ER_TRACE("Check %zu total subscribers on bus '%s'",
           llist_size(swb->subscribers),
           swb->name);
  size_t count = 0;
  LLIST_FOREACH(swb->subscribers, next, node) {
    struct swbus_sub* sub = (struct swbus_sub*)node->data;
    if (pid == sub->pid) {
      if (OK == swbus_subscriber_notify(swb, res->bp, sub, &rxq_entry)) {
        ++count;
      } else {
        ER_WARN("Failed to notify RXQ %zu subscribed to PID %d/0x%x on bus '%s'",
                sub->subscriber - swb->rxqs,
                sub->pid,
                sub->pid,
                swb->name);

        rstat = ERROR;
      }

    } else {
      ER_TRACE("Skip notifying subscriber: PID mismatch: %d/0x%x != %d/0x%x ",
               sub->pid,
               sub->pid,
               pid,
               pid);
    }
  } /* LLIST_FOREACH() */

  ER_DEBUG("Notified %zu subscribers subscribed to PID %d/0x%x on bus '%s'",
           count,
           pid,
           pid,
           swb->name);
  /*
   * Unconditional call. If the reference count is currently 0 (i.e. no one
   * was subscribed to the packet ID), then it will be released.
   *
   * This is in an if() instead of RCSW_CHECK() to catch case when all
   * subscriber notifications succeed but releasing fails for some reason.
   * An application-built reservation (bp == NULL) has no pool to release to.
   */
  if (NULL != res->bp && OK != mpool_release(res->bp, res->data)) {
    rstat = ERROR;
  }

  /* all users counted now */
  if (!(swb->flags & RCSW_SWBUS_ASYNC)) {
    rdwrl_exit(&swb->syncl, SCOPE_WR);
  }
  mutex_unlock(&swb->mutex);
  return rstat;
}

struct pcqueue* swbus_rxq_init(struct swbus* swb, void* buf_p, size_t n_entries) {
  RCSW_FPC_NV(NULL, swb != NULL);

  mutex_lock(&(swb->mutex));
  ER_DEBUG("Attempting allocation of RXQ %zu", swb->n_rxqs);

  /* If max number rxqs has not been reached then allocate one */
  ER_CHECK(swb->n_rxqs < swb->max_rxqs, "No available RXQs");
  struct pcqueue* rxq = swb->rxqs + swb->n_rxqs;

  /* create FIFO */
  struct pcqueue_config params = {.elt_size = sizeof(struct swbus_rxq_ent),
                                  .max_elts = n_entries,
                                  .elements = buf_p,
                                  .flags    = RCSW_NOALLOC_HANDLE};
  params.flags |= (buf_p != NULL) ? RCSW_NOALLOC_DATA : RCSW_NONE;
  RCSW_CHECK(NULL != pcqueue_init(rxq, &params));

  swb->n_rxqs++;
  mutex_unlock(&swb->mutex);
  return rxq;

error:
  mutex_unlock(&swb->mutex);
  return NULL;
} /* swbus_rxq_init() */

status_t swbus_subscribe(struct swbus* swb, struct pcqueue* queue, uint32_t pid) {
  RCSW_FPC_NV(ERROR, swb != NULL, queue != NULL);

  mutex_lock(&swb->mutex);
  ER_CHECK(llist_size(swb->subscribers) < swb->max_subs,
           "Failed to subscribe RXQ %zu to PID %d/0x%x on bus '%s': "
           "subscription list full",
           queue - swb->rxqs,
           pid,
           pid,
           swb->name);

  /* find index for insertion of new subscription */
  struct swbus_sub sub = {.pid = pid, .subscriber = queue};
  ER_CHECK(NULL == llist_data_query(swb->subscribers, &sub),
           "Failed to subscribe RXQ %zu to PID %d/0x%x on bus '%s': subscription "
           "exists ",
           queue - swb->rxqs,
           pid,
           pid,
           swb->name);
  RCSW_CHECK(OK == llist_append(swb->subscribers, &sub));
  ER_DEBUG("Subscribed RXQ %zu to PID %d/0x%x on bus '%s'",
           queue - swb->rxqs,
           pid,
           pid,
           swb->name);
  mutex_unlock(&swb->mutex);
  return OK;

error:
  mutex_unlock(&swb->mutex);
  return ERROR;
} /* swbus_subscribe() */

status_t swbus_unsubscribe(struct swbus*   swb,
                           struct pcqueue* queue,
                           uint32_t        pid) {
  RCSW_FPC_NV(ERROR, swb != NULL, queue != NULL);
  status_t rstat = ERROR;
  mutex_lock(&swb->mutex);

  /* find index for insertion of new subscription */
  struct swbus_sub   sub  = {.pid = pid, .subscriber = queue};
  struct llist_node* node = llist_node_query(swb->subscribers, &sub);
  ER_CHECK(NULL != node,
           "Could not unsubscribe RXQ %zu from PID %d/0x%x on bus '%s': no "
           "such subscription",
           queue - swb->rxqs,
           pid,
           pid,
           swb->name);
  RCSW_CHECK(OK == llist_delete(swb->subscribers, node, NULL));
  rstat = OK;

error:
  mutex_unlock(&swb->mutex);
  return rstat;
} /* swbus_unsubscribe() */

struct swbus_rxq_ent* swbus_rxq_wait(struct swbus* swb, struct pcqueue* queue) {
  RCSW_FPC_NV(NULL, NULL != swb, NULL != queue);
  struct swbus_rxq_ent* ent = NULL;

  /*
   * pcqueue_peek() doesn't wait, so wait for an entry here. Taking and
   * returning a slots_inuse count leaves the queue's accounting unchanged.
   */
  RCSW_CHECK(OK == pcqueue_waitpeek(queue, (void**)&ent));

  /*
   * Put after you actually get data so that if you aren't supposed to start
   * processing data until all subscribers to a PID in a pool have received
   * their packets AND the call to swbus_publish_release() didn't start until
   * after you started waiting in pcqueue_pop() you wait until the parent
   * publish finishes as intended.
   */
  if (!(swb->flags & RCSW_SWBUS_ASYNC)) {
    rdwrl_req(&swb->syncl, SCOPE_RD);
  }

  if (!(swb->flags & RCSW_SWBUS_ASYNC)) {
    rdwrl_exit(&swb->syncl, SCOPE_RD);
  }

error:
  return ent;
} /* swbus_rxq_wait() */

struct swbus_rxq_ent* swbus_rxq_timedwait(struct swbus*    swb,
                                          struct pcqueue*  queue,
                                          struct timespec* to) {
  RCSW_FPC_NV(NULL, NULL != swb, NULL != queue, NULL != to);
  struct swbus_rxq_ent* ent = NULL;
  RCSW_CHECK(OK == pcqueue_timedpeek(queue, to, (void**)&ent));

  if (!(swb->flags & RCSW_SWBUS_ASYNC)) {
    rdwrl_req(&swb->syncl, SCOPE_RD);
  }

  if (!(swb->flags & RCSW_SWBUS_ASYNC)) {
    rdwrl_exit(&swb->syncl, SCOPE_RD);
  }

error:
  return ent;
} /* swbus_rxq_timedwait() */

status_t swbus_rxq_pop_front(struct pcqueue* queue, struct swbus_rxq_ent* ent) {
  RCSW_FPC_NV(ERROR, NULL != queue, NULL != ent);

  /* If the application did not use the release() function directly. */
  if (ent->bp) {
    /*
     * Unconditional release. If the reference count for the chunk of memory
     * for the top packet reaches 0, then it will be freed.
     */
    RCSW_CHECK(OK == mpool_release(ent->bp, ent->data));
  }

  /*
   * Assuming the application has already done whatever it needs to with the
   * front item.
   */
  RCSW_CHECK(OK == pcqueue_pop(queue, NULL));
  return OK;

error:
  return ERROR;
}

struct swbus_rxq_ent* swbus_rxq_front(struct pcqueue* const queue) {
  RCSW_FPC_NV(NULL, queue != NULL);
  struct swbus_rxq_ent* ent = NULL;

  RCSW_CHECK(OK == pcqueue_peek(queue, (void**)&ent));
  return ent;

error:
  return NULL;
}

END_C_DECLS
