/**
 * \file pulse.c
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/pulse/pulse.h"

#define RCSW_ER_MODNAME "rcsw.pulse"
#define RCSW_ER_MODID ekLOG4CL_PULSE
#include "rcsw/er/client.h"
#include "rcsw/common/fpc.h"
#include "rcsw/common/alloc.h"

/*******************************************************************************
 * Private Functions
 ******************************************************************************/

static status_t pulse_subscriber_notify(struct pulse* pulse,
                                        struct mpool* bp,
                                        struct pulse_sub* sub,
                                        struct pulse_rxq_ent* rxq_ent) {
  RCSW_FPC_NV(ERROR,
              NULL != pulse,
              NULL != bp,
              NULL != sub,
              NULL != sub->subscriber,
              NULL != rxq_ent);

  ER_TRACE("Notifying RXQ %zu subscribed to PID %d/0x%x on bus '%s', pending=%zu",
           sub->subscriber - pulse->rxqs,
           sub->pid,
           sub->pid,
           pulse->name,
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
} /* pulse_subscriber_notify() */

static int pulse_sub_cmp(const void* a, const void* b) {
  const struct pulse_sub* s1 = a;
  const struct pulse_sub* s2 = b;
  if (s1->pid < s2->pid) {
    return -1;
  } else if (s1->pid > s2->pid) {
    return 1;
  } else {
    return (int)(s1->subscriber - s2->subscriber);
  }
} /* pulse_sub_ent_cmp() */

/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS

struct pulse* pulse_init(struct pulse* pulse_in,
                         const struct pulse_params* params) {
  RCSW_FPC_NV(NULL, params != NULL);
  RCSW_ER_MODULE_INIT();

  struct pulse* pulse = rcsw_alloc(pulse_in,
                                   sizeof(struct pulse),
                                   params->flags & RCSW_NOALLOC_HANDLE);

  RCSW_CHECK_PTR(pulse);
  RCSW_CHECK_PTR(mutex_init(&pulse->mutex, RCSW_NOALLOC_HANDLE));
  RCSW_CHECK_PTR(rdwrl_init(&pulse->syncl, RCSW_NOALLOC_HANDLE));

  strncpy(pulse->name, params->name, RCSW_PULSE_MAX_NAMELEN);
  ER_DEBUG("Initializing PULSE instance '%s'", pulse->name);

  pulse->flags = params->flags;
  pulse->n_rxqs = 0;
  pulse->n_pools = params->max_pools;
  pulse->max_rxqs = params->max_rxqs;
  pulse->max_subs = params->max_subs;

  ER_DEBUG("Initializing %zu buffer pools", pulse->n_pools);

  /* initialize buffer pools */
  pulse->pools = rcsw_alloc(NULL,
                            pulse->n_pools * sizeof(struct mpool),
                            RCSW_NONE);

  RCSW_CHECK_PTR(pulse->pools);

  for (size_t i = 0; i < pulse->n_pools; i++) {
    params->pools[i].flags |= RCSW_NOALLOC_HANDLE;
    RCSW_CHECK_PTR(mpool_init(&pulse->pools[i],
                              &params->pools[i]));
  } /* for() */

  ER_DEBUG("Allocating %zu receive queues, %zu max subscribers/queue",
       pulse->max_rxqs,
       pulse->max_subs);

  /* Allocate receive queues */
  pulse->rxqs = rcsw_alloc(NULL,
                           pulse->max_rxqs * sizeof(struct pcqueue),
                            RCSW_NONE);

  RCSW_CHECK_PTR(pulse->rxqs);

  /* Initialize subscriber list */
  struct llist_params llparams = { .max_elts = (int)pulse->max_subs,
                                    .elt_size = sizeof(struct pulse_sub),
                                    .cmpe = pulse_sub_cmp,
                                    .flags = RCSW_DS_SORTED };
  pulse->subscribers = llist_init(NULL, &llparams);
  RCSW_CHECK_PTR(pulse->subscribers);

  ER_DEBUG("Initialization complete for PULSE instance '%s'", pulse->name);
  return pulse;

error:
  pulse_destroy(pulse);
  return NULL;
} /* pulse_init() */

void pulse_destroy(struct pulse* pulse) {
  RCSW_FPC_V(NULL != pulse);

  if (pulse->pools) {
    for (size_t i = 0; i < pulse->n_pools; ++i) {
      mpool_destroy(&pulse->pools[i]);
    } /* for(i..) */
    rcsw_free(pulse->pools, RCSW_NONE);
  }
  if (pulse->rxqs) {
    for (size_t i = 0; i < pulse->n_rxqs; ++i) {
      pcqueue_destroy(&pulse->rxqs[i]);
    } /* for(i..) */
    rcsw_free(pulse->rxqs, RCSW_NONE);
  }
  if (pulse->subscribers) {
    llist_destroy(pulse->subscribers);
  }
  rcsw_free(pulse, pulse->flags & RCSW_NOALLOC_HANDLE);
} /* pulse_destroy() */

status_t pulse_publish(struct pulse* pulse,
                       uint32_t pid,
                       size_t pkt_size,
                       const void* pkt) {
  RCSW_FPC_NV(ERROR, NULL != pulse, pkt_size > 0, NULL != pkt);

  ER_DEBUG("Publishing to bus '%s': PID=%d/0x%x, pkt=%p, pkt_size=%zu",
           pulse->name,
           pid,
           pid,
           pkt,
           pkt_size);

  /* get space on the software bus for the packet */
  struct pulse_rsrvn res;
  RCSW_CHECK(OK == pulse_publish_reserve(pulse, &res, pkt_size));

  /* the actual publish: copy the packet to the allocated buffer */
  memcpy(res.data, pkt, pkt_size);

  /* release the allocated buffer (i.e. push to receive queues) */
  ER_CHECK(OK == pulse_publish_release(pulse,
                                       pid,
                                       &res,
                                       pkt_size),
           "Could not release buffer for publish for PID=%d/0x%x, pkt_size=%zu "
           "on bus '%s'",
           pid,
           pid,
           pkt_size,
           pulse->name);
  return OK;

error:
  return ERROR;
} /* pulse_publish() */

status_t pulse_publish_reserve(struct pulse* pulse,
                                struct pulse_rsrvn* res,
                                size_t pkt_size) {
  RCSW_FPC_NV(ERROR, NULL != pulse, NULL != res, pkt_size > 0);

  ER_DEBUG("Reserving %zu byte buffer on bus '%s'", pkt_size, pulse->name);


  for (size_t i = 0; i < pulse->n_pools; i++) {
    struct mpool* pool = &pulse->pools[i];

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
      res->bp = pulse->pools + i;
      return OK;
    }
  } /*  for(i...) */

  /* no free buffer big enough found */
  ER_DEBUG("Failed to reserve %zu byte buffer on bus '%s'",
           pkt_size,
           pulse->name);
  return ERROR;
} /* pulse_publish_reserve() */

status_t pulse_publish_release(struct pulse* pulse,
                               uint32_t pid,
                               struct pulse_rsrvn* res,
                               size_t pkt_size) {
  RCSW_FPC_NV(ERROR,
              NULL != pulse,
              NULL != res,
              pkt_size > 0);
  status_t rstat = OK;
  struct pulse_rxq_ent rxq_entry;

  rxq_entry.data = res->data;
  rxq_entry.bp = res->bp;
  rxq_entry.pkt_size = pkt_size;
  rxq_entry.pid = pid;

  mutex_lock(&pulse->mutex);
  ER_TRACE("Releasing published data for PID=%d/0x%x on bus '%s'",
           pid,
           pid,
           pulse->name);

  /* Keep application threads from servicing until all subscribers notified */
  if (!(pulse->flags & RCSW_PULSE_ASYNC)) {
    rdwrl_req(&pulse->syncl, ekSCOPE_WR);
  }

  ER_TRACE("Check %zu total subscribers on bus '%s'",
           llist_size(pulse->subscribers),
           pulse->name);
  size_t count = 0;
  LLIST_FOREACH(pulse->subscribers, next, node) {
    struct pulse_sub* sub = (struct pulse_sub*)node->data;
    if (pid == sub->pid) {
      if (OK == pulse_subscriber_notify(pulse, res->bp, sub, &rxq_entry)) {
        ++count;
      } else {
        ER_WARN("Failed to notify RXQ %zu subscribed to PID %d/0x%x on bus '%s'",
                sub->subscriber - pulse->rxqs,
                sub->pid,
                sub->pid,
                pulse->name);

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
           pulse->name);
  /*
   * Unconditional call. If the reference count is currently 0 (i.e. no one
   * was subscribed to the packet ID), then it will be released.
   */
  RCSW_CHECK(OK == mpool_release(res->bp, res->data));

error:
  /* all users counted now */
  if (!(pulse->flags & RCSW_PULSE_ASYNC)) {
    rdwrl_exit(&pulse->syncl, ekSCOPE_WR);
  }
  mutex_unlock(&pulse->mutex);
  return rstat;
} /* pulse_publish_release() */

struct pcqueue*
pulse_rxq_init(struct pulse* pulse, void* buf_p, uint32_t n_entries) {
  RCSW_FPC_NV(NULL, pulse != NULL);

  mutex_lock(&(pulse->mutex));
  ER_DEBUG("Attempting allocation of RXQ %zu", pulse->n_rxqs);

  /* If max number rxqs has not been reached then allocate one */
  ER_CHECK(pulse->n_rxqs < pulse->max_rxqs, "No available RXQs");
  struct pcqueue* rxq = pulse->rxqs + pulse->n_rxqs;

  /* create FIFO */
  struct pcqueue_params params = { .elt_size = sizeof(struct pulse_rxq_ent),
                                    .max_elts = n_entries,
                                    .elements = buf_p,
                                    .flags = RCSW_NOALLOC_HANDLE };
  params.flags |= (buf_p != NULL) ? RCSW_NOALLOC_DATA : RCSW_NONE;
  RCSW_CHECK(NULL != pcqueue_init(rxq, &params));

  pulse->n_rxqs++;
  mutex_unlock(&pulse->mutex);
  return rxq;

error:
  mutex_unlock(&pulse->mutex);
  return NULL;
} /* pulse_rxq_init() */

status_t
pulse_subscribe(struct pulse* pulse, struct pcqueue* queue, uint32_t pid) {
  RCSW_FPC_NV(ERROR, pulse != NULL, queue != NULL);

  mutex_lock(&pulse->mutex);
  ER_CHECK(llist_size(pulse->subscribers) < pulse->max_subs,
           "Failed to subscribe RXQ %zu to PID %d/0x%x on bus '%s': "
           "subscription list full",
           queue - pulse->rxqs,
           pid,
           pid,
           pulse->name);

  /* find index for insertion of new subscription */
  struct pulse_sub sub = { .pid = pid, .subscriber = queue };
  ER_CHECK(NULL == llist_data_query(pulse->subscribers, &sub),
           "Failed to subscribe RXQ %zu to PID %d/0x%x on bus '%s': subscription "
           "exists ",
           queue - pulse->rxqs,
           pid,
           pid,
           pulse->name);
  RCSW_CHECK(OK == llist_append(pulse->subscribers, &sub));
  ER_DEBUG("Subscribed RXQ %zu to PID %d/0x%x on bus '%s'",
           queue - pulse->rxqs,
           pid,
           pid,
           pulse->name);
  mutex_unlock(&pulse->mutex);
  return OK;

error:
  mutex_unlock(&pulse->mutex);
  return ERROR;
} /* pulse_subscribe() */

status_t pulse_unsubscribe(struct pulse* pulse,
                           struct pcqueue* queue,
                           uint32_t pid) {
  RCSW_FPC_NV(ERROR, pulse != NULL, queue != NULL);
  status_t rstat = ERROR;
  mutex_lock(&pulse->mutex);

  /* find index for insertion of new subscription */
  struct pulse_sub sub = { .pid = pid, .subscriber = queue };
  struct llist_node* node = llist_node_query(pulse->subscribers, &sub);
  ER_CHECK(NULL != node,
           "Could not unsubscribe RXQ %zu from PID %d/0x%x on bus '%s': no "
           "such subscription",
           queue - pulse->rxqs,
           pid,
           pid,
           pulse->name);
  RCSW_CHECK(OK == llist_delete(pulse->subscribers, node, NULL));
  rstat = OK;

error:
  mutex_unlock(&pulse->mutex);
  return rstat;
} /* pulse_unsubscribe() */

struct pulse_rxq_ent* pulse_rxq_wait(struct pulse* pulse,
                                     struct pcqueue* queue) {
  RCSW_FPC_NV(NULL, NULL != pulse, NULL != queue);
  struct pulse_rxq_ent *ent = NULL;

  RCSW_CHECK(OK == pcqueue_peek(queue, (void**)&ent));

  /*
   * Put after you actually get data so that if you aren't supposed to start
   * processing data until all subscribers to a PID in a pool have received
   * their packets AND the call to pulse_publish_release() didn't start until
   * after you started waiting in pcqueue_pop() you wait until the parent
   * publish finishes as intended.
   */
  if (!(pulse->flags & RCSW_PULSE_ASYNC)) {
    rdwrl_req(&pulse->syncl, ekSCOPE_RD);
  }

  if (!(pulse->flags & RCSW_PULSE_ASYNC)) {
    rdwrl_exit(&pulse->syncl, ekSCOPE_RD);
  }

error:
  return ent;
} /* pulse_rxq_wait() */

struct pulse_rxq_ent* pulse_rxq_timedwait(struct pulse* pulse,
                                         struct pcqueue* queue,
                                         struct timespec* to) {
  RCSW_FPC_NV(NULL, NULL != pulse, NULL != queue, NULL != to);
  struct pulse_rxq_ent *ent = NULL;
  RCSW_CHECK(OK == pcqueue_timedpeek(queue, to, (void**)&ent));

  if (!(pulse->flags & RCSW_PULSE_ASYNC)) {
    rdwrl_req(&pulse->syncl, ekSCOPE_RD);
  }

  if (!(pulse->flags & RCSW_PULSE_ASYNC)) {
    rdwrl_exit(&pulse->syncl, ekSCOPE_RD);
  }

error:
  return ent;
} /* pulse_rxq_timedwait() */

status_t pulse_rxq_pop_front(struct pcqueue* queue,
                             struct pulse_rxq_ent* ent) {
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
} /* pulse_rxq_pop_front() */

struct pulse_rxq_ent* pulse_rxq_front(struct pcqueue *const queue) {
  RCSW_FPC_NV(NULL, queue != NULL);
  struct pulse_rxq_ent* ent = NULL;

  RCSW_CHECK(OK == pcqueue_peek(queue, (void**)&ent));
  return ent;

error:
  return NULL;
}

END_C_DECLS
