/**
 * \file swbus.c
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/swbus/swbus.h"

#define RCSW_ER_MODNAME RCSW_ER_MODNAME_BUILDER("rcsw", "swb")
#define RCSW_ER_MODID ekLOG4CL_SWBUS
#include "rcsw/er/client.h"
#include "rcsw/common/fpc.h"
#include "rcsw/common/alloc.h"

/*******************************************************************************
 * Private Functions
 ******************************************************************************/

static status_t swbus_subscriber_notify(struct swbus* swb,
                                        struct mpool* bp,
                                        struct swbus_sub* sub,
                                        struct swbus_rxq_ent* rxq_ent) {
  RCSW_FPC_NV(ERROR,
              NULL != swb,
              NULL != bp,
              NULL != sub,
              NULL != sub->subscriber,
              NULL != rxq_ent);

  ER_TRACE("Notifying RXQ %zu subscribed to PID %d/0x%x on bus '%s', pending=%zu",
           sub->subscriber - swb->rxqs,
           sub->pid,
           sub->pid,
           swb->name,
           pcqueue_size(sub->subscriber));

  /* Add entry to subscriber receive queue */
  if (OK == pcqueue_push(sub->subscriber, rxq_ent)) {
    /*
     * Add a reference. This is not done during the reserve step as no
     * one is actually using the memory at that time.
     */
    RCSW_CHECK(OK == mpool_ref_add(bp, rxq_ent->data));
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
  } else if (s1->pid > s2->pid) {
    return 1;
  } else {
    return (int)(s1->subscriber - s2->subscriber);
  }
} /* swbus_sub_ent_cmp() */

/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS

struct swbus* swbus_init(struct swbus* swb_in,
                         const struct swbus_params* params) {
  RCSW_FPC_NV(NULL, params != NULL);
  RCSW_ER_MODULE_INIT();

  struct swbus* swb = rcsw_alloc(swb_in,
                                 sizeof(struct swbus),
                                 params->flags & RCSW_NOALLOC_HANDLE);

  RCSW_CHECK_PTR(swb);
  RCSW_CHECK_PTR(mutex_init(&swb->mutex, RCSW_NOALLOC_HANDLE));
  RCSW_CHECK_PTR(rdwrl_init(&swb->syncl, RCSW_NOALLOC_HANDLE));

  strncpy(swb->name, params->name, RCSW_SWBUS_MAX_NAMELEN);
  ER_DEBUG("Initializing SWB instance '%s'", swb->name);

  swb->flags = params->flags;
  swb->n_rxqs = 0;
  swb->n_pools = params->max_pools;
  swb->max_rxqs = params->max_rxqs;
  swb->max_subs = params->max_subs;

  ER_DEBUG("Initializing %zu buffer pools", swb->n_pools);

  /* initialize buffer pools */
  swb->pools = rcsw_alloc(NULL,
                          swb->n_pools * sizeof(struct mpool),
                          RCSW_NONE);

  RCSW_CHECK_PTR(swb->pools);

  for (size_t i = 0; i < swb->n_pools; i++) {
    params->pools[i].flags |= RCSW_NOALLOC_HANDLE;
    RCSW_CHECK_PTR(mpool_init(&swb->pools[i],
                              &params->pools[i]));
  } /* for() */

  ER_DEBUG("Allocating %zu receive queues, %zu max subscribers/queue",
           swb->max_rxqs,
           swb->max_subs);

  /* Allocate receive queues */
  swb->rxqs = rcsw_alloc(NULL,
                         swb->max_rxqs * sizeof(struct pcqueue),
                         RCSW_NONE);

  RCSW_CHECK_PTR(swb->rxqs);

  /* Initialize subscriber list */
  struct llist_params llparams = { .max_elts = (int)swb->max_subs,
    .elt_size = sizeof(struct swbus_sub),
    .cmpe = swbus_sub_cmp,
    .flags = RCSW_DS_SORTED };
  swb->subscribers = llist_init(NULL, &llparams);
  RCSW_CHECK_PTR(swb->subscribers);

  ER_DEBUG("Initialization complete for SWB instance '%s'", swb->name);
  return swb;

error:
  swbus_destroy(swb);
  return NULL;
} /* swbus_init() */

void swbus_destroy(struct swbus* swb) {
  RCSW_FPC_V(NULL != swb);

  if (swb->pools) {
    for (size_t i = 0; i < swb->n_pools; ++i) {
      mpool_destroy(&swb->pools[i]);
    } /* for(i..) */
    rcsw_free(swb->pools, RCSW_NONE);
  }
  if (swb->rxqs) {
    for (size_t i = 0; i < swb->n_rxqs; ++i) {
      pcqueue_destroy(&swb->rxqs[i]);
    } /* for(i..) */
    rcsw_free(swb->rxqs, RCSW_NONE);
  }
  if (swb->subscribers) {
    llist_destroy(swb->subscribers);
  }
  rcsw_free(swb, swb->flags & RCSW_NOALLOC_HANDLE);
} /* swbus_destroy() */

status_t swbus_publish(struct swbus* swb,
                       uint32_t pid,
                       size_t pkt_size,
                       const void* pkt) {
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
  ER_CHECK(OK == swbus_publish_release(swb,
                                       pid,
                                       &res,
                                       pkt_size),
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

status_t swbus_publish_reserve(struct swbus* swb,
                               struct swbus_rsrvn* res,
                               size_t pkt_size) {
  RCSW_FPC_NV(ERROR, NULL != swb, NULL != res, pkt_size > 0);

  ER_DEBUG("Reserving %zu byte buffer on bus '%s'", pkt_size, swb->name);


  for (size_t i = 0; i < swb->n_pools; i++) {
    struct mpool* pool = &swb->pools[i];

    /* can't use this buffer pool--buffers are too small or pool is full */
    if (pool->elt_size < pkt_size || mpool_isfull(pool)) {
      ER_TRACE("Skipping buffer pool %zu in reservation search: buf_size=%zu, "
               "pkt_size=%zu, full=%d",
               i,
               pool->elt_size,
               pkt_size,
               mpool_isfull(pool));
      continue;
    }
    dptr_t* space = mpool_req(pool);
    if (NULL != space) {
      res->data = space;
      res->bp = swb->pools + i;
      return OK;
    }
  } /*  for(i...) */

  /* no free buffer big enough found */
  ER_DEBUG("Failed to reserve %zu byte buffer on bus '%s'",
           pkt_size,
           swb->name);
  return ERROR;
} /* swbus_publish_reserve() */

status_t swbus_publish_release(struct swbus* swb,
                               uint32_t pid,
                               struct swbus_rsrvn* res,
                               size_t pkt_size) {
  RCSW_FPC_NV(ERROR,
              NULL != swb,
              NULL != res,
              pkt_size > 0);
  status_t rstat = OK;
  struct swbus_rxq_ent rxq_entry;

  rxq_entry.data = res->data;
  rxq_entry.bp = res->bp;
  rxq_entry.pkt_size = pkt_size;
  rxq_entry.pid = pid;

  mutex_lock(&swb->mutex);
  ER_TRACE("Releasing published data for PID=%d/0x%x on bus '%s'",
           pid,
           pid,
           swb->name);

  /* Keep application threads from servicing until all subscribers notified */
  if (!(swb->flags & RCSW_SWBUS_ASYNC)) {
    rdwrl_req(&swb->syncl, ekSCOPE_WR);
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
   */
  RCSW_CHECK(OK == mpool_release(res->bp, res->data));

error:
  /* all users counted now */
  if (!(swb->flags & RCSW_SWBUS_ASYNC)) {
    rdwrl_exit(&swb->syncl, ekSCOPE_WR);
  }
  mutex_unlock(&swb->mutex);
  return rstat;
} /* swbus_publish_release() */

struct pcqueue*
swbus_rxq_init(struct swbus* swb, void* buf_p, uint32_t n_entries) {
  RCSW_FPC_NV(NULL, swb != NULL);

  mutex_lock(&(swb->mutex));
  ER_DEBUG("Attempting allocation of RXQ %zu", swb->n_rxqs);

  /* If max number rxqs has not been reached then allocate one */
  ER_CHECK(swb->n_rxqs < swb->max_rxqs, "No available RXQs");
  struct pcqueue* rxq = swb->rxqs + swb->n_rxqs;

  /* create FIFO */
  struct pcqueue_params params = { .elt_size = sizeof(struct swbus_rxq_ent),
    .max_elts = n_entries,
    .elements = buf_p,
    .flags = RCSW_NOALLOC_HANDLE };
  params.flags |= (buf_p != NULL) ? RCSW_NOALLOC_DATA : RCSW_NONE;
  RCSW_CHECK(NULL != pcqueue_init(rxq, &params));

  swb->n_rxqs++;
  mutex_unlock(&swb->mutex);
  return rxq;

error:
  mutex_unlock(&swb->mutex);
  return NULL;
} /* swbus_rxq_init() */

status_t swbus_subscribe(struct swbus* swb,
                         struct pcqueue* queue,
                         uint32_t pid) {
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
  struct swbus_sub sub = { .pid = pid, .subscriber = queue };
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

status_t swbus_unsubscribe(struct swbus* swb,
                           struct pcqueue* queue,
                           uint32_t pid) {
  RCSW_FPC_NV(ERROR, swb != NULL, queue != NULL);
  status_t rstat = ERROR;
  mutex_lock(&swb->mutex);

  /* find index for insertion of new subscription */
  struct swbus_sub sub = { .pid = pid, .subscriber = queue };
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

struct swbus_rxq_ent* swbus_rxq_wait(struct swbus* swb,
                                     struct pcqueue* queue) {
  RCSW_FPC_NV(NULL, NULL != swb, NULL != queue);
  struct swbus_rxq_ent *ent = NULL;

  RCSW_CHECK(OK == pcqueue_peek(queue, (void**)&ent));

  /*
   * Put after you actually get data so that if you aren't supposed to start
   * processing data until all subscribers to a PID in a pool have received
   * their packets AND the call to swbus_publish_release() didn't start until
   * after you started waiting in pcqueue_pop() you wait until the parent
   * publish finishes as intended.
   */
  if (!(swb->flags & RCSW_SWBUS_ASYNC)) {
    rdwrl_req(&swb->syncl, ekSCOPE_RD);
  }

  if (!(swb->flags & RCSW_SWBUS_ASYNC)) {
    rdwrl_exit(&swb->syncl, ekSCOPE_RD);
  }

error:
  return ent;
} /* swbus_rxq_wait() */

struct swbus_rxq_ent* swbus_rxq_timedwait(struct swbus* swb,
                                          struct pcqueue* queue,
                                          struct timespec* to) {
  RCSW_FPC_NV(NULL, NULL != swb, NULL != queue, NULL != to);
  struct swbus_rxq_ent *ent = NULL;
  RCSW_CHECK(OK == pcqueue_timedpeek(queue, to, (void**)&ent));

  if (!(swb->flags & RCSW_SWBUS_ASYNC)) {
    rdwrl_req(&swb->syncl, ekSCOPE_RD);
  }

  if (!(swb->flags & RCSW_SWBUS_ASYNC)) {
    rdwrl_exit(&swb->syncl, ekSCOPE_RD);
  }

error:
  return ent;
} /* swbus_rxq_timedwait() */

status_t swbus_rxq_pop_front(struct pcqueue* queue,
                             struct swbus_rxq_ent* ent) {
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
} /* swbus_rxq_pop_front() */

struct swbus_rxq_ent* swbus_rxq_front(struct pcqueue *const queue) {
  RCSW_FPC_NV(NULL, queue != NULL);
  struct swbus_rxq_ent* ent = NULL;

  RCSW_CHECK(OK == pcqueue_peek(queue, (void**)&ent));
  return ent;

error:
  return NULL;
}

END_C_DECLS
