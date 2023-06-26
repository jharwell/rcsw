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
#define RCSW_ER_MODID M_PULSE
#include "rcsw/er/client.h"
#include "rcsw/common/fpc.h"

/*******************************************************************************
 * Forward Declarations
 ******************************************************************************/
static int pulse_sub_ent_cmp(const void* a, const void* b);
static status_t pulse_subscriber_notify(struct pulse_bp_ent* bp_ent,
                                        const struct pulse_sub_ent* sub,
                                        struct pulse_rxq_ent* rxq_ent);

/**
 * \brief Reserve a buffer on a pulse instance. A suitable buffer will be found
 * in the first pool large enough to contain the packet and has free space.
 *
 * \param pulse The pulse handle.
 * \param bp_ent The buffer pool entry reserved for the packet (to be filled).
 * \param pkt_size Size of the packet in bytes.
 *
 * \return The allocated packet buffer, or NULL if none found or an error
 * occurred.
 */
static void* pulse_publish_reserve(struct pulse_inst* pulse,
                                   struct pulse_bp_ent** bp_ent,
                                   size_t pkt_size);
/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS

struct pulse_inst* pulse_init(struct pulse_inst* pulse_in,
                              const struct pulse_params* params) {
  RCSW_FPC_NV(NULL, params != NULL);
  RCSW_ER_MODULE_INIT();

  struct pulse_inst* pulse = NULL;
  uint16_t i = 0;

  /* initialize software bus */
  if (params->flags & PULSE_APP_DOMAIN_HANDLE) {
    RCSW_CHECK_PTR(pulse_in);
    pulse = pulse_in;
  } else {
    pulse = calloc(1, sizeof(struct pulse_inst));
  }
  RCSW_CHECK_PTR(mt_mutex_init(&pulse->mutex, MT_APP_DOMAIN_MEM));
  memcpy(pulse->name, params->name, PULSE_MAX_NAMELEN);
  pulse->flags = params->flags;
  pulse->n_rxqs = 0;
  pulse->n_pools = params->n_pools;
  pulse->max_rxqs = params->max_rxqs;
  pulse->max_subs = params->max_subs;
  ER_DEBUG("Initializing %zu buffer pools: %s allocation",
       pulse->n_pools,
       (params->flags & PULSE_APP_DOMAIN_POOLS) ? "STATIC" : "DYNAMIC");

  /* initialize buffer pools */
  pulse->buffer_pools = calloc(pulse->n_pools, sizeof(struct pulse_bp_ent));
  RCSW_CHECK_PTR(pulse->buffer_pools);

  uint32_t flags = 0;
  if (!(pulse->flags & PULSE_APP_DOMAIN_POOLS)) {
    flags = RCSW_DS_NOALLOC_DATA | RCSW_DS_NOALLOC_NODES;
  }

  for (i = 0; i < pulse->n_pools; i++) {
    struct mpool_params mpool_params = { .elt_size = params->pools[i].buf_size,
                                         .max_elts = params->pools[i].n_bufs,
                                         .nodes = params->pools[i].nodes,
                                         .elements = params->pools[i].elements,
                                         .flags = flags | RCSW_DS_NOALLOC_HANDLE |
                                                  MPOOL_REF_COUNT_EN };
    RCSW_CHECK_PTR(
        mt_mutex_init(&pulse->buffer_pools[i].mutex, MT_APP_DOMAIN_MEM));
    RCSW_CHECK(NULL != mpool_init(&pulse->buffer_pools[i].pool, &mpool_params));
  } /* for() */

  ER_DEBUG("Allocating %zu receive queues, %zu max subscribers",
       pulse->max_rxqs,
       pulse->max_subs);

  /* Initialize receive queues */
  pulse->rx_queues = calloc(pulse->max_rxqs, sizeof(struct mt_queue));
  RCSW_CHECK_PTR(pulse->rx_queues);

  /* initialize subscriber list */
  struct ds_params llist_params = { .max_elts = (int)pulse->max_subs,
                                    .elt_size = sizeof(struct pulse_sub_ent),
                                    .cmpe = pulse_sub_ent_cmp,
                                    .tag = ekRCSW_DS_LLIST,
                                    .flags = RCSW_DS_SORTED };
  pulse->sub_list = llist_init(NULL, &llist_params);
  RCSW_CHECK_PTR(pulse->sub_list);

  return pulse;

error:
  pulse_destroy(pulse);
  return NULL;
} /* pulse_init() */

void pulse_destroy(struct pulse_inst* pulse) {
  RCSW_FPC_V(NULL != pulse);

  if (pulse->buffer_pools) {
    for (size_t i = 0; i < pulse->n_pools; ++i) {
      mpool_destroy(&pulse->buffer_pools[i].pool);
    } /* for(i..) */
    free(pulse->buffer_pools);
  }
  if (pulse->rx_queues) {
    for (size_t i = 0; i < pulse->n_rxqs; ++i) {
      mt_queue_destroy(pulse->rx_queues + i);
    } /* for(i..) */
    free(pulse->rx_queues);
  }

  if (!(pulse->flags & PULSE_APP_DOMAIN_HANDLE)) {
    free(pulse);
  }
} /* pulse_destroy() */

status_t pulse_publish(struct pulse_inst* pulse,
                       uint32_t pid,
                       size_t pkt_size,
                       const void* pkt) {
  RCSW_FPC_NV(ERROR, pulse != NULL, pkt_size > 0);

  ER_TRACE("Publishing to bus %s", pulse->name);
  ER_TRACE("Packet ID   : 0x%08X", pid);
  ER_TRACE("Packet size : %zu2 bytes", pkt_size);

  struct pulse_bp_ent* bp_ent;

  /* get space on the software bus for the packet */
  uint8_t* reservation = pulse_publish_reserve(pulse, &bp_ent, pkt_size);
  RCSW_CHECK_PTR(reservation);

  /* the actual publish: copy the packet to the allocated buffer */
  memcpy(reservation, pkt, pkt_size);

  /* release the allocated buffer (i.e. push to receive queues) */
  ER_CHECK(
      OK == pulse_publish_release(pulse, pid, bp_ent, reservation, pkt_size),
      "Could not release allocated buffer");
  return OK;

error:
  return ERROR;
} /* pulse_publish() */

status_t pulse_publish_release(struct pulse_inst* pulse,
                               uint32_t pid,
                               struct pulse_bp_ent* bp_ent,
                               void* reservation,
                               size_t pkt_size) {
  RCSW_FPC_NV(
      ERROR, pulse != NULL, NULL != bp_ent, reservation != NULL, pkt_size > 0);
  status_t rstat = OK;
  struct pulse_rxq_ent rxq_entry;

  rxq_entry.buf = reservation;
  rxq_entry.bp_ent = bp_ent;
  rxq_entry.pkt_size = pkt_size;
  rxq_entry.pid = pid;

  ER_TRACE("Releasing receive queue entry on bus %s", pulse->name);
  mt_mutex_lock(&pulse->mutex);

  struct pulse_sub_ent* sub = NULL;

  /* Keep application threads from servicing until all subscribers notified */
  if (!(pulse->flags & PULSE_SERVICE_ASYNC)) {
    mt_mutex_lock(&bp_ent->mutex);
  }

  LLIST_FOREACH(pulse->sub_list, next, node) {
    sub = (struct pulse_sub_ent*)node->data;
    if (pid == sub->pid) {
      ER_TRACE("Notifying RXQ %zu subscribed to PID 0x%08X",
           sub->subscriber - pulse->rx_queues,
           pid);
      if (pulse_subscriber_notify(bp_ent, sub, &rxq_entry)) {
        rstat = ERROR;
      }
      break;
    }
  } /* LLIST_FOREACH() */

  ER_DEBUG("Notified %d subscribers subscribed to PID 0x%08X",
       mpool_ref_query(&bp_ent->pool, reservation),
       pid);

  /*
   * Unconditional call. If the reference count is currently 0 (i.e. no one
   * was subscribed to the packet ID), then it will be released.
   */
  RCSW_CHECK(OK == mpool_release(&bp_ent->pool, reservation));

error:
  /* all users counted now */
  if (!(pulse->flags & PULSE_SERVICE_ASYNC)) {
    mt_mutex_unlock(&bp_ent->mutex);
  }
  mt_mutex_unlock(&pulse->mutex);
  return rstat;
} /* pulse_publish_release() */

struct mt_queue*
pulse_rxq_init(struct pulse_inst* pulse, void* buf_p, uint32_t n_entries) {
  RCSW_FPC_NV(NULL, pulse != NULL);

  ER_DEBUG("Initializing new receive queue");
  mt_mutex_lock(&(pulse->mutex));

  /* If max number rxqs has not been reached then allocate one */
  ER_CHECK(pulse->n_rxqs < pulse->max_rxqs,
                "Cannot allocate receive queue (no space)");
  struct mt_queue* rxq = pulse->rx_queues + pulse->n_rxqs;

  /* create FIFO */
  struct mt_queue_params params = { .elt_size = sizeof(struct pulse_rxq_ent),
                                    .max_elts = n_entries,
                                    .elements = buf_p,
                                    .flags = RCSW_DS_NOALLOC_HANDLE };
  params.flags |= (buf_p != NULL) ? RCSW_DS_NOALLOC_DATA : 0;
  RCSW_CHECK(NULL != mt_queue_init(rxq, &params));

  pulse->n_rxqs++;
  mt_mutex_unlock(&pulse->mutex);
  return rxq;

error:
  mt_mutex_unlock(&pulse->mutex);
  return NULL;
} /* pulse_rxq_init() */

status_t
pulse_subscribe(struct pulse_inst* pulse, struct mt_queue* queue, uint32_t pid) {
  RCSW_FPC_NV(ERROR, pulse != NULL, queue != NULL);

  mt_mutex_lock(&pulse->mutex);
  ER_CHECK(llist_n_elts(pulse->sub_list) < pulse->max_subs,
                "Cannot subscribe--all subscribers allocated");

  /* find index for insertion of new subscription */
  struct pulse_sub_ent sub = { .pid = pid, .subscriber = queue };
  ER_CHECK(NULL == llist_data_query(pulse->sub_list, &sub),
                "Cannot subscribe RXQ %zu to PID 0x%08x: duplicate "
                "subscription",
                queue - pulse->rx_queues,
                pid);
  RCSW_CHECK(OK == llist_append(pulse->sub_list, &sub));
  ER_DEBUG(
      "Subscribed RXQ %zu to PID 0x%08x on bus", queue - pulse->rx_queues, pid);
  mt_mutex_unlock(&pulse->mutex);
  return OK;

error:
  mt_mutex_unlock(&pulse->mutex);
  return ERROR;
} /* pulse_subscribe() */

status_t pulse_unsubscribe(struct pulse_inst* pulse,
                           struct mt_queue* queue,
                           uint32_t pid) {
  RCSW_FPC_NV(ERROR, pulse != NULL, queue != NULL);
  status_t rstat = ERROR;
  mt_mutex_lock(&pulse->mutex);

  /* find index for insertion of new subscription */
  struct pulse_sub_ent sub = { .pid = pid, .subscriber = queue };
  struct llist_node* node = llist_node_query(pulse->sub_list, &sub);
  ER_CHECK(NULL != node,
           "Could not unsubscribe RXQ %zu from PID "
           "0x%08X: PID no such subscription",
           queue - pulse->rx_queues,
           pid);
  RCSW_CHECK(OK == llist_delete(pulse->sub_list, node, NULL));
  rstat = OK;

error:
  mt_mutex_unlock(&pulse->mutex);
  return rstat;
} /* pulse_unsubscribe() */

void* pulse_wait_front(struct mt_queue* queue) {
  RCSW_FPC_NV(NULL, NULL != queue);
  uint8_t* pkt = NULL;
  struct pulse_rxq_ent ent;
  RCSW_CHECK(OK == mt_queue_pop(queue, &ent));
  pkt = ent.buf;

error:
  return pkt;
} /* pulse_wait_front() */

void* pulse_timedwait_front(struct mt_queue* queue, struct timespec* to) {
  RCSW_FPC_NV(NULL, NULL != queue, NULL != to);
  uint8_t* pkt = NULL;
  struct pulse_rxq_ent ent;
  RCSW_CHECK(OK == mt_queue_timed_pop(queue, to, &ent));
  pkt = ent.buf;

error:
  return pkt;
} /* pulse_timedwait_front() */

status_t pulse_pop_front(struct mt_queue* queue) {
  RCSW_FPC_NV(ERROR, queue != NULL);

  status_t rstat = ERROR;
  struct pulse_rxq_ent* ent = mt_queue_peek(queue);

  /* If the application did not use the release() function directly. */
  if (ent->bp_ent) {
    /*
     * Unconditional release. If the reference count for the chunk of memory
     * for the top packet reaches 0, then it will be freed.
     */
    RCSW_CHECK(OK == mpool_release(&ent->bp_ent->pool, ent->buf));
  }

  rstat = OK;

error:
  return rstat;
} /* pulse_pop_front() */

/*******************************************************************************
 * Static Functions
 ******************************************************************************/
static void* pulse_publish_reserve(struct pulse_inst* pulse,
                                   struct pulse_bp_ent** bp_ent,
                                   size_t pkt_size) {
  RCSW_FPC_NV(NULL, NULL != pulse, NULL != bp_ent, pkt_size > 0);

  ER_DEBUG("Reserving %zu byte buffer on bus %s", pkt_size, pulse->name);
  for (size_t i = 0; i < pulse->n_pools; i++) {
    struct mpool* pool_p = &pulse->buffer_pools[i].pool;

    /* can't use this buffer pool--buffers are too small or pool is full */
    if (pool_p->elt_size < pkt_size || mpool_isfull(pool_p)) {
      ER_TRACE("Skipping buffer pool %zu in reservation search: buf_size=%zu, "
           "pkt_size=%zu, full=%d",
           i,
           pool_p->elt_size,
           pkt_size,
           mpool_isfull(pool_p));
    } else {
      void* ret = mpool_req(pool_p);
      RCSW_CHECK_PTR(ret);
      *bp_ent = pulse->buffer_pools + i;
      return ret;
    }
  } /*  for(i...) */

error:
  /* no free buffer big enough found */
  ER_ERR("Could not reserve a buffer of sufficient size");
  return NULL;
} /* pulse_publish_reserve() */

status_t pulse_subscriber_notify(struct pulse_bp_ent* bp_ent,
                                 const struct pulse_sub_ent* sub,
                                 struct pulse_rxq_ent* rxq_ent) {
  RCSW_FPC_NV(ERROR,
              NULL != bp_ent,
              NULL != sub,
              NULL != sub->subscriber,
              NULL != rxq_ent);
  status_t rstat = ERROR;

  /* add entry to subscriber rx queue and signal */
  mt_mutex_lock(&(sub->subscriber->mutex));
  if (OK == mt_queue_push(sub->subscriber, &rxq_ent)) {
    /*
     * Add a reference. This is not done during the reserve step as no
     * one is actually using the memory at that time.
     */
    RCSW_CHECK(OK == mpool_ref_add(&bp_ent->pool, rxq_ent->buf));
  }
  mt_mutex_unlock(&(sub->subscriber->mutex));

  rstat = OK;
error:
  return rstat;
} /* pulse_subscriber_notify() */

static int pulse_sub_ent_cmp(const void* a, const void* b) {
  const struct pulse_sub_ent* s1 = a;
  const struct pulse_sub_ent* s2 = b;
  if (s1->pid < s2->pid) {
    return -1;
  } else if (s1->pid > s2->pid) {
    return 1;
  } else {
    return (int)(s1->subscriber - s2->subscriber);
  }
} /* pulse_sub_ent_cmp() */

END_C_DECLS
