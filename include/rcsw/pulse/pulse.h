/**
 * \file pulse.h
 * \ingroup pulse
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/multithread/cvm.h"
#include "rcsw/multithread/mutex.h"
#include "rcsw/multithread/pcqueue.h"
#include "rcsw/rcsw.h"
#include "rcsw/multithread/mpool.h"
#include "rcsw/ds/rbuffer.h"
#include "rcsw/multithread/rdwrlock.h"

/*******************************************************************************
 * Constant Definitions
 ******************************************************************************/
/* Just to make things easy */
#define RCSW_PULSE_MAX_NAMELEN 32

/**
 * \brief Declare that space for ALL of the buffer pools is provided by the
 * application (both its elements and the nodes it uses to manage the
 * elements).
 *
 * Not passing this flag will cause PULSE to malloc() for the memory needed for
 * each buffer pool.
 */
#define RCSW_PULSE_NOALLOC_POOLS (RCSW_NOALLOC_DATA | RCSW_NOALLOC_META)

/**
 * \brief Declare that \ref pulse subscribers subscribed to the same PID can
 * service received packets in their respective queues before all packets have
 * been pushed to all subscribers during a given \ref pulse_publish_release().
 */
#define RCSW_PULSE_ASYNC (1 << RCSW_MODFLAGS_START )

/*******************************************************************************
 * Structure Definitions
 ******************************************************************************/
/**
 * \brief PULSE initialization parameters.
 */
struct pulse_params {
  /**
   * Each PULSE can have any # of buffer pools (each pool can have any number
   * of entries). If you need more than 8 or so, you are probably doing
   * something weird (read: wrong)...
   */
  struct mpool_params* pools;

  /** Max # of buffer pools to create for the bus. */
  size_t max_pools;

  /** Max # of receive queues for the bus. */
  size_t max_rxqs;

  /** Max # of subscribers to a particular packet ID. */
  size_t max_subs;

  /**
   * Configuration flags.
   *
   * Valid flags are:
   *
   * - \ref RCSW_ZALLOC
   * - \ref RCSW_NOALLOC_HANDLE
   * - \ref RCSW_PULSE_NOALLOC_POOLS
   * - \ref RCSW_PULSE_ASYNC
   *
   * All other flags are ignored.
   */
  uint32_t flags;

  /**
   * Name for \ref pulse instance. Used to assist with debugging if multiple
   * PULSE instances are active. Has no effect on PULSE operation.
   */
  char name[RCSW_PULSE_MAX_NAMELEN];
};

/**
 * \brief PULSE receive queue (RXQ) entry.
 *
 * When a packet is published to the bus, a receive queue entry for the packet
 * is placed in each subscribed receive queue.
 */
struct pulse_rxq_ent {
  /** Pointer to the buffer with the actual data. */
  uint8_t* data;

  /** Received packet size in bytes. */
  size_t pkt_size;

  /** ID of received packet. */
  uint32_t pid;

  /** The buffer pool entry that the data resides in. */
  struct mpool *bp;
};

/**
 * \brief A reservation which can later be used to publish some data.
 *
 * Can be used in 3 ways:
 *
 * - Internally when \ref pulse_publish() is called by the API.
 *
 * - Received by the application when \ref pulse_publish_reserve() is called,
 *   and should eventually be passed to \ref
 *   pulse_publish_release(). Reservation is good indefinitely.
 *
 * - Manually created by the application with \ref pulse_rsrvn.data pointing to
 *   data the application is already filling to avoid the mempy() which happens
 *   if you just \ref pulse_publish() directly. In this case \ref pulse_rsrvn.bp
 *   should be NULL.
 *
 * All of the pulse_rxq_XX() functions can be used regardless of which way is
 * chosen.
 */
struct pulse_rsrvn {
  /** Pointer to the buffer with the actual data. */
  uint8_t* data;

  /** Received packet size in bytes. */
  size_t pkt_size;

  /** The \ref mpool that the actual data resides in. */
  struct mpool *bp;
};

/**
 * \brief PULSE subscription (maps a PID to an RXQ).
 *
 * Every time a task/thread subscribes to a packet ID, they get an subscription
 * entry, which is inserted into the sorted subscriber array for the pulse
 * instance.
 */
struct pulse_sub {
  /**
   * ID of subscribed packet.
   */
  uint32_t pid;

  /**
   * The \ref pcqueue (RXQ) subscriber.
   */
  struct pcqueue *subscriber;
};

/**
 * \brief Manage publisher-subscriber needs in an embedded environment.
 *
 * Memory efficient. Basically a fully connected network (in the parallel
 * computing sense).
 */
struct pulse {
  /** # buffer pools (static during lifetime). */
  size_t n_pools;

  /** Number of active receive queues (dynamic during lifetime). */
  size_t n_rxqs;

  /** Max number of receive queues allowed. */
  size_t max_rxqs;

  /** Max number of subscribers (RXQ-pid pairs) allowed. */
  size_t max_subs;

  /** Mutex to protect access to bus metadata. */
  struct mutex mutex;

  /**
   * Configuration flags.
   *
   * Valid flags are:
   *
   * - \ref RCSW_NOALLOC_HANDLE
   * - \ref RCSW_PULSE_NOALLOC_POOLS
   * - \ref RCSW_PULSE_ASYNC
   *
   * All other flags are ignored.
   */
  uint32_t flags;

  /**
   * Array of buffer pool entries. Published data stored here. This is
   * always allocated by PULSE during initialization.
   */
  struct mpool *pools;

  /**
   * Array of receive queues. Used by the application to subscribe to packets
   * and to receive published packets. This is always allocated during
   * initialization.
   */
  struct pcqueue *rxqs;

  /** List of \ref pulse_sub. Always sorted. */
  struct llist *subscribers;

  /**
   * Prevents applications from servicing their receive queues for the packet
   * currently being pushed out via \ref pulse_publish_release() until all
   * subscribers have been notified if \ref RCSW_PULSE_ASYNC is not passed.
   */
  struct rdwrlock syncl;

  /**
   * Name for instance. Used to assist with debugging if multiple PULSE
   * instances are active. Has no effect on PULSE operation.
   */
  char name[RCSW_PULSE_MAX_NAMELEN];
};

/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Get pointer to the top packet on a receive queue.
 *
 * Use this function if a given RXQ is only subscribed to a single packet type,
 * so there's no ambiguity about what comes out of the queue in terms of what to
 * do with it.
 *
 * Note that if the queue is currently empty this function will wait until
 * there's something in it before returning.
 *
 * \return The top of the queue, or NULL if no such packet or an error occurred.
 */
struct pulse_rxq_ent* pulse_rxq_front(struct pcqueue *const queue);

/**
 * \brief Initialize a \ref pulse instance.
 *
 * \param pulse_in The pulse handle to be filled (can be NULL if
 *                 \ref RCSW_NOALLOC_HANDLE not passed).
 *
 * \param params The initialization parameters.
 *
 * \return Initialized pulse instance, or NULL if an error occurred.
 */
struct pulse *pulse_init(struct pulse *pulse_in,
                         const struct pulse_params * params) RCSW_CHECK_RET;

/**
 * \brief Destroy a \ref pulse instance
 *
 * Any further use of th pulse handle after calling this function is undefined.
 *
 * \param pulse The pulse handle.
 */
void pulse_destroy(struct pulse *pulse);

/**
 * \brief Allocate and initialize a receive queue.
 *
 * \param pulse The pulse handle.
 *
 * \param buf_p Space for the rxq entries. Can be NULL (pulse will malloc() for
 *              space).
 *
 * \param n_entries Max # of entries for rxq.
 *
 * \return Pointer to new receive queue, or NULL if an error occurred.
 */
struct pcqueue *pulse_rxq_init(struct pulse * pulse,
                                void * buf_p,
                                uint32_t n_entries) RCSW_CHECK_RET;

/**
 * \brief Subscribe the specified RXQ to the specified packet ID.
 *
 * \param pulse The pulse handle.
 * \param queue The RXQ to subscribe.
 * \param pid The PID to subscribe to.
 *
 * \return \ref status_t.
 */
status_t pulse_subscribe(struct pulse * pulse,
                         struct pcqueue * queue,
                         uint32_t pid);

/**
 * \brief Unsubscribe the specified RXQ from the specified packet ID
 *
 * \param pulse The pulse handle.
 * \param queue The RXQ to unsubscribe.
 * \param pid The PID to unsubscribe from.
 *
 * \return \ref status_t.
 */
status_t pulse_unsubscribe(struct pulse * pulse,
                           struct pcqueue * queue,
                           uint32_t pid);

/**
 * \brief Publish a packet to the bus.
 *
 * A memcpy() will be performed. If the packet is very large, consider using
 * \ref pulse_publish_release() instead; it will not perform a memcpy().
 *
 * \param pulse The pulse handle.
 * \param pid The packet ID.
 * \param pkt_size The size of the packet in bytes.
 * \param pkt The packet to publish.
 *
 * \return \ref status_t
 */
status_t pulse_publish(struct pulse * pulse,
                       uint32_t pid,
                       size_t pkt_size,
                       const uint8_t* pkt);

/**
 * \brief Reserve a buffer on a \ref pulse instance.
 *
 * A suitable buffer will be found in the first pool large enough to contain the
 * packet and has free space.
 *
 * \param pulse The pulse handle.
 *
 * \param res The reservation for the publish (to be filled on success).
 *
 * \param pkt_size Size of the packet in bytes.
 *
 * \return \ref status_t.
 */
status_t pulse_publish_reserve(struct pulse* pulse,
                               struct pulse_rsrvn* res,
                               size_t pkt_size);
/**
 * \brief Release a published entry (i.e. send it to all subscribed receive
 * queues).
 *
 * If a given receive queue is full, ERROR will be returned, but the bus will
 * still attempt to publish to the remaining queues. If the application takes on
 * the task of synchronization/allocating memory for very large packets, then
 * this function can be called directly, avoiding a potentially expensive memory
 * copy.
 *
 * \param pulse The pulse handle.
 *
 * \param pid The packet ID.
 *
 * \param res The space reserved for the packet in a particular buffer
 *            pool. The pointed-to value must have been the result of a
 *            successful \ref pulse_publish_reserve().
 *
 * \param pkt_size Size of the packet in bytes.
 *
 * \return \ref status_t.
 */
status_t pulse_publish_release(struct pulse* pulse,
                               uint32_t pid,
                               struct pulse_rsrvn* res,
                               size_t pkt_size);

/**
 * \brief Wait (indefinitely) until the given receive queue is not empty,
 * returning a reference to the first item in the queue.
 *
 * \param pulse The pulse handle.
 *
 * \param queue The receive queue to wait on.
 *
 * Regardless of how the packet was published, you need to call \ref
 * pulse_rxq_pop_front() when you are finished with the packet.
 *
 * \return A reference to the first item in the queue, or NULL if an ERROR
 * occurred.
 */
struct pulse_rxq_ent* pulse_rxq_wait(struct pulse* pulse,
                                     struct pcqueue * queue) RCSW_CHECK_RET;

/**
 * \brief Wait (until a timeout) until the given receive queue is not empty.
 *
 * \param pulse The pulse handle.
 *
 * \param queue The receive queue to wait on.
 *
 * \param to A RELATIVE timeout.
 *
 * Regardless of how the packet was published, you need to call \ref
 * pulse_rxq_pop_front() when you are finished with the packet.
 *
 * \return A reference to the first item in the queue, or NULL if an ERROR or a
 * timeout occurred.
 */
struct pulse_rxq_ent* pulse_rxq_timedwait(struct pulse* pulse,
                                          struct pcqueue * queue,
                                          struct timespec * to) RCSW_CHECK_RET;

/**
 * \brief Remove and release the front element from the selected receive queue.
 *
 * \param queue The parent \ref pcqueue of the packet.
 *
 * \param ent The previously "peeked" element from \ref pulse_rxq_front(), \ref
 *            pulse_rxq_wait(), or \ref pulse_rxq_timedwait().
 *
 * \return \ref status_t.
 */
status_t pulse_rxq_pop_front(struct pcqueue* queue,
                             struct pulse_rxq_ent * ent);

END_C_DECLS
