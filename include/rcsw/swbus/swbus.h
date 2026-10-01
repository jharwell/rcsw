/**
 * \file
 *
 * \copyright 2017 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * \ingroup swbus
 *
 * \brief Publisher-subscriber software bus.
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <time.h>

#include "rcsw/al/types.h"
#include "rcsw/core/compilers.h"
#include "rcsw/core/core.h"
#include "rcsw/core/flags.h"
#include "rcsw/ds/llist.h"
#include "rcsw/multithread/mpool.h"
#include "rcsw/multithread/mutex.h"
#include "rcsw/multithread/pcqueue.h"
#include "rcsw/multithread/rdwrlock.h"

/*******************************************************************************
 * Constant Definitions
 ******************************************************************************/
/** \brief Size of \ref swbus_config.name, including the NUL. */
#define RCSW_SWBUS_MAX_NAMELEN 32

/**
 * \brief Declare that \ref swbus subscribers subscribed to the same PID can
 * service received packets in their respective queues before all packets have
 * been pushed to all subscribers during a given \ref swbus_publish_release().
 */
#define RCSW_SWBUS_ASYNC (1 << RCSW_MODFLAGS_SHIFT)

/*******************************************************************************
 * Types
 ******************************************************************************/
/**
 * \brief SWBUS initialization parameters.
 */
struct swbus_config {
  /**
   * Buffer pool configurations, \ref swbus_config.max_pools of them. Each pool
   * can have any number of entries; a handful of pools is typical.
   *
   * Pools should be ordered from smallest to largest chunk size to get best-fit
   * behavior during \ref swbus_publish_release().
   */
  struct mpool_config* pools;

  /** Max # of buffer pools to create for the bus. */
  size_t max_pools;

  /** Max # of receive queues for the bus. */
  size_t max_rxqs;

  /** Max # of subscriptions (RXQ-PID pairs) on the bus. */
  size_t max_subs;

  /**
   * Application-allocated space for the bus metadata: the \ref mpool and \ref
   * pcqueue handles and the subscriber list. Must be at least \ref
   * swbus_meta_space() bytes. Like all caller-provided memory, regions within
   * it are aligned to \c RCSW_CONFIG_PTR_ALIGN, which must therefore be at
   * least the alignment of a pointer on the target.
   * Ignored unless \ref RCSW_NOALLOC_META is passed.
   *
   * The pools' element storage is configured per pool through \ref
   * swbus_config.pools, and each RXQ's through \ref swbus_rxq_init(); with all
   * three provided, the bus does no heap allocation (synchronization
   * primitives aside).
   */
  dptr_t* meta;

  /**
   * Configuration flags.
   *
   * Valid flags are:
   *
   * - \ref RCSW_ZALLOC
   * - \ref RCSW_NOALLOC_HANDLE
   * - \ref RCSW_NOALLOC_META
   * - \ref RCSW_SWBUS_ASYNC
   *
   * All other flags are ignored.
   */
  uint32_t flags;

  /**
   * Name for the \ref swbus instance. Used to assist with debugging if
   * multiple buses are active. Has no effect on operation.
   */
  char name[RCSW_SWBUS_MAX_NAMELEN];
};

/**
 * \brief swbus receive queue (RXQ) entry.
 *
 * When a packet is published to the bus, a receive queue entry for the packet
 * is placed in each subscribed receive queue.
 */
struct swbus_rxq_ent {
  /** Pointer to the buffer with the actual data. */
  dptr_t* data;

  /** Packet size in bytes. */
  size_t pkt_size;

  /** ID of received packet. */
  uint32_t pid;

  /** The buffer pool that the data resides in. */
  struct mpool* bp;
};

/**
 * \brief A reservation which can later be used to publish some data.
 *
 * Can be used in 3 ways:
 *
 * - Internally when \ref swbus_publish() is called by the API.
 *
 * - Received by the application when \ref swbus_publish_reserve() is called,
 *   and should eventually be passed to \ref
 *   swbus_publish_release(). Reservation is good indefinitely.
 *
 * - Manually created by the application with \ref swbus_rsrvn.data pointing to
 *   data the application is already filling to avoid the memcpy() which happens
 *   if you just \ref swbus_publish() directly. In this case \ref swbus_rsrvn.bp
 *   must be NULL and \ref swbus_rsrvn.pkt_size set. There is no reference
 *   counting: the application owns the buffer and must keep it valid until
 *   every subscriber has called \ref swbus_rxq_pop_front().
 *
 * All of the swbus_rxq_XX() functions can be used regardless of which way is
 * chosen.
 */
struct swbus_rsrvn {
  /** Pointer to the buffer with the actual data. */
  dptr_t* data;

  /**
   * Packet size in bytes. Set by \ref swbus_publish_reserve(); \ref
   * swbus_publish_release() publishes this many bytes.
   */
  size_t pkt_size;

  /**
   * The \ref mpool that the actual data resides in, or NULL for an
   * application-built reservation.
   */
  struct mpool* bp;
};

/**
 * \brief swbus subscription (maps a PID to an RXQ).
 *
 * Every time a task/thread subscribes to a packet ID, it gets a subscription
 * entry, which is inserted into the bus's sorted subscriber list.
 */
struct swbus_sub {
  /**
   * ID of subscribed packet.
   */
  uint32_t pid;

  /**
   * The \ref pcqueue (RXQ) subscriber.
   */
  struct pcqueue* subscriber;
};

/**
 * \brief Manage publisher-subscriber needs in an embedded environment.
 *
 * Memory efficient. Basically a fully connected network (in the parallel
 * computing sense).
 */
struct swbus {
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
   * - \ref RCSW_ZALLOC
   * - \ref RCSW_NOALLOC_HANDLE
   * - \ref RCSW_NOALLOC_META
   * - \ref RCSW_SWBUS_ASYNC
   *
   * All other flags are ignored.
   */
  uint32_t flags;

  /**
   * Array of buffer pool entries. Published data stored here. Carved from
   * \ref swbus_config.meta with \ref RCSW_NOALLOC_META, otherwise allocated.
   */
  struct mpool* pools;

  /**
   * Array of receive queues. Used by the application to subscribe to packets
   * and to receive published packets. Carved from \ref swbus_config.meta with
   * \ref RCSW_NOALLOC_META, otherwise allocated.
   */
  struct pcqueue* rxqs;

  /** List of \ref swbus_sub. Always sorted. */
  struct llist* subscribers;

  /**
   * Prevents applications from servicing their receive queues for the packet
   * currently being pushed out via \ref swbus_publish_release() until all
   * subscribers have been notified if \ref RCSW_SWBUS_ASYNC is not passed.
   */
  struct rdwrlock syncl;

  /**
   * Name for the instance. Used to assist with debugging if multiple buses
   * are active. Has no effect on operation.
   */
  char name[RCSW_SWBUS_MAX_NAMELEN];
};

/*******************************************************************************
 * Public API
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Offsets of each region within \ref swbus_config.meta.
 *
 * \cond INTERNAL
 */
struct swbus_meta_layout {
  size_t pools;
  size_t rxqs;
  size_t list;
  size_t list_meta;
  size_t list_elements;
  size_t total;
};

static inline size_t swbus_meta_round(size_t n) {
  return (n + sizeof(dptr_t) - 1) / sizeof(dptr_t) * sizeof(dptr_t);
}

/* NOLINTNEXTLINE(bugprone-easily-swappable-parameters) */
static inline struct swbus_meta_layout swbus_meta_layout_calc(size_t max_pools,
                                                              size_t max_rxqs,
                                                              size_t max_subs) {
  struct swbus_meta_layout l;
  l.pools         = 0;
  l.rxqs          = l.pools + swbus_meta_round(max_pools * sizeof(struct mpool));
  l.list          = l.rxqs + swbus_meta_round(max_rxqs * sizeof(struct pcqueue));
  l.list_meta     = l.list + swbus_meta_round(sizeof(struct llist));
  l.list_elements = l.list_meta + swbus_meta_round(llist_meta_space(max_subs));
  l.total =
    l.list_elements +
    swbus_meta_round(llist_element_space(max_subs, sizeof(struct swbus_sub)));
  return l;
}
/** \endcond */

/**
 * \brief Calculate the size of \ref swbus_config.meta for use with \ref
 * RCSW_NOALLOC_META.
 *
 * \param max_pools \ref swbus_config.max_pools.
 * \param max_rxqs \ref swbus_config.max_rxqs.
 * \param max_subs \ref swbus_config.max_subs.
 *
 * \return The # of bytes required.
 */
static inline size_t swbus_meta_space(size_t max_pools,
                                      size_t max_rxqs,
                                      size_t max_subs) {
  return swbus_meta_layout_calc(max_pools, max_rxqs, max_subs).total;
}

/**
 * \brief Get pointer to the top packet on a receive queue.
 *
 * Doesn't wait: if the queue is empty, returns NULL with \c errno set to
 * \c EAGAIN. Unlike \ref swbus_rxq_wait(), it takes no bus handle and so
 * doesn't wait for an in-progress \ref swbus_publish_release() to reach every
 * subscriber when \ref RCSW_SWBUS_ASYNC is not set.
 *
 * \param queue The receive queue.
 *
 * \return The top of the queue, or NULL if the queue is empty or an error
 * occurred.
 */
RCSW_API struct swbus_rxq_ent* swbus_rxq_front(struct pcqueue* queue);

/**
 * \brief Initialize a \ref swbus instance.
 *
 * \param swb_in Caller storage for the handle, used only if \ref
 *               RCSW_NOALLOC_HANDLE is passed; ignored (may be NULL)
 *               otherwise. See \rcswdoc{concepts/memory-model}.
 *
 * \param params The initialization parameters.
 *
 * \return The initialized bus, or NULL if an error occurred.
 */
RCSW_API struct swbus* swbus_init(struct swbus*              swb_in,
                                  const struct swbus_config* params) RCSW_WUR;

/**
 * \brief Destroy a \ref swbus instance.
 *
 * Any further use of the handle after calling this function is undefined.
 *
 * \param swb The bus handle.
 */
RCSW_API void swbus_destroy(struct swbus* swb);

/**
 * \brief Allocate and initialize a receive queue.
 *
 * \param swb The bus handle.
 *
 * \param buf_p Space for the RXQ entries, which are \ref swbus_rxq_ent
 *              objects. Can be NULL, in which case the bus allocates it.
 *
 * \param n_entries Max # of entries for rxq.
 *
 * \return Pointer to new receive queue, or NULL if an error occurred.
 */
RCSW_API struct pcqueue* swbus_rxq_init(struct swbus* swb,
                                        void*         buf_p,
                                        size_t        n_entries) RCSW_WUR;

/**
 * \brief Subscribe the specified RXQ to the specified packet ID.
 *
 * \param swb The bus handle.
 * \param queue The RXQ to subscribe.
 * \param pid The PID to subscribe to.
 *
 * \return \ref status_t.
 */
RCSW_API status_t swbus_subscribe(struct swbus*   swb,
                                  struct pcqueue* queue,
                                  uint32_t        pid);

/**
 * \brief Unsubscribe the specified RXQ from the specified packet ID.
 *
 * \param swb The bus handle.
 * \param queue The RXQ to unsubscribe.
 * \param pid The PID to unsubscribe from.
 *
 * \return \ref status_t.
 */
RCSW_API status_t swbus_unsubscribe(struct swbus*   swb,
                                    struct pcqueue* queue,
                                    uint32_t        pid);

/**
 * \brief Publish a packet to the bus.
 *
 * A memcpy() will be performed. If the packet is very large, consider using
 * \ref swbus_publish_release() instead; it will not perform a memcpy().
 *
 * \param swb The bus handle.
 * \param pid The packet ID.
 * \param pkt_size The size of the packet in bytes.
 * \param pkt The packet to publish.
 *
 * \return \ref status_t
 */
RCSW_API status_t swbus_publish(struct swbus* swb,
                                uint32_t      pid,
                                size_t        pkt_size,
                                const void*   pkt);

/**
 * \brief Reserve a buffer on a \ref swbus instance.
 *
 * A suitable buffer will be found in the first pool large enough to contain the
 * packet and has free space.
 *
 * \param swb The swb handle.
 *
 * \param res The reservation for the publish (to be filled on success).
 *
 * \param pkt_size Size of the packet in bytes; recorded in \ref
 *                 swbus_rsrvn.pkt_size.
 *
 * \return \ref status_t. ERROR with errno=ENOSPC if no pool has a free buffer
 * of at least \p pkt_size bytes.
 */
RCSW_API status_t swbus_publish_reserve(struct swbus*       swb,
                                        struct swbus_rsrvn* res,
                                        size_t              pkt_size);

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
 * \note The bus mutex is held for the entire release operation by design, not
 * oversight. Dynamic subscription/de-subscription and RXQ initialization are
 * not common in target application environments, so this is unlikely to be a
 * bottleneck. Doing it this way also eliminates the need to snapshot the
 * subscribers to notify, which requires a \ref llist copy, VLA, or hard-coded
 * max to the # subscribers.
 *
 * \param swb The bus handle.
 *
 * \param pid The packet ID.
 *
 * \param res The reservation. \p res->pkt_size is the packet size subscribers
 *            see in \ref swbus_rxq_ent.pkt_size; to publish fewer bytes than
 *            were reserved, lower it before calling.
 *
 * \return \ref status_t.
 */
RCSW_API status_t swbus_publish_release(struct swbus*       swb,
                                        uint32_t            pid,
                                        struct swbus_rsrvn* res);

/**
 * \brief Wait (indefinitely) until the given receive queue is not empty,
 * returning a reference to the first item in the queue.
 *
 * \param swb The bus handle.
 *
 * \param queue The receive queue to wait on.
 *
 * Regardless of how the packet was published, you need to call \ref
 * swbus_rxq_pop_front() when you are finished with the packet.
 *
 * \return A reference to the first item in the queue, or NULL if an ERROR
 * occurred.
 */
RCSW_API struct swbus_rxq_ent* swbus_rxq_wait(struct swbus*   swb,
                                              struct pcqueue* queue) RCSW_WUR;

/**
 * \brief Wait (until a timeout) until the given receive queue is not empty.
 *
 * \param swb The bus handle.
 *
 * \param queue The receive queue to wait on.
 *
 * \param to A relative timeout. See \rcswdoc{concepts/concurrency/timeouts}.
 *
 * Regardless of how the packet was published, you need to call \ref
 * swbus_rxq_pop_front() when you are finished with the packet.
 *
 * \return A reference to the first item in the queue, or NULL if an ERROR or a
 * timeout occurred.
 */
RCSW_API struct swbus_rxq_ent* swbus_rxq_timedwait(struct swbus*    swb,
                                                   struct pcqueue*  queue,
                                                   struct timespec* to) RCSW_WUR;

/**
 * \brief Remove and release the front element from the selected receive queue.
 *
 * \param queue The parent \ref pcqueue of the packet.
 *
 * \param ent The previously "peeked" element from \ref swbus_rxq_front(), \ref
 *            swbus_rxq_wait(), or \ref swbus_rxq_timedwait().
 *
 * \return \ref status_t.
 */
RCSW_API status_t swbus_rxq_pop_front(struct pcqueue*       queue,
                                      struct swbus_rxq_ent* ent);

END_C_DECLS
