/**
 * \file pulse.h
 * \ingroup pulse
 * \brief Implementation of memory efficient PUbLisher-SubscribEr
 * (PULSE) system.
 *
 * Basically a fully connected network (in the parallel computing sense).
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/multithread/mt_cvm.h"
#include "rcsw/multithread/mt_mutex.h"
#include "rcsw/multithread/mt_queue.h"
#include "rcsw/rcsw.h"
#include "rcsw/multithread/mpool.h"
#include "rcsw/ds/rbuffer.h"

/*******************************************************************************
 * Constant Definitions
 ******************************************************************************/
/* Just to make things easy */
#define PULSE_MAX_NAMELEN 32

/**
 * \brief Declare that the space for the PULSE handle will be provided by the
 * application.
 */
#define PULSE_APP_DOMAIN_HANDLE 0x1

/**
 * \brief Declare that space for ALL of the buffer pools is provided by the
 * application (both its elements and the nodes it uses to manage the
 * elements).
 *
 * Not passing this flag will cause PULSE to malloc() for the memory needed for
 * each buffer pool.
 */
#define PULSE_APP_DOMAIN_POOLS 0x2

/**
 * \brief Declare that it is OK for PULSE subscribers subscribers subscribed to
 * the same PID to service packets in their respective queues before all packets
 * have been pushed to all subscribers.
 */
#define PULSE_SERVICE_ASYNC 0x10

/*******************************************************************************
 * Structure Definitions
 ******************************************************************************/
/**
 * \brief PULSE buffer pool initialization parameters
 */
struct pulse_bp_params {
    uint16_t n_bufs;    /// # of buffers in this pool.
    uint16_t buf_size;  /// # of bytes in each buffer.
    uint8_t *elements;  /// Space for buffers. Can be NULL.
    /** Space for nodes used for buffer management. Can be NULL. */
    uint8_t *nodes;
};

/**
 * \brief PULSE framework initialization parameters.
 */
struct pulse_params {
    size_t n_pools;   /// # of buffer pools for the bus.
    size_t max_rxqs;  /// Max # of receive queues for the bus.
    size_t max_subs;
    uint32_t flags;   /// Configuration flags.

    /**
     * Each PULSE can have any # of buffer pools (each pool can have any number
     * of entries). If you need more than 8 or so, you are probably doing
     * something weird (read: wrong)...
     */
    struct pulse_bp_params* pools;
    char name[PULSE_MAX_NAMELEN];
};

/**
 * \brief PULSE Buffer pool entry.
 *
 * Contains n bufs of whatever size that are used to hold the published packets.
 */
struct pulse_bp_ent {
    struct mpool pool;  /// The buffer pool containing actual data.
    /**
     * mutex to protect buffer pool when pushing out published packets. Pool
     * protects itself during request/release. Only used if \ref
     * PULSE_SERVICE_ASYNC is not passed.
     */
    mt_mutex_t mutex;
};

/**
 * \brief PULSE receive queue entry.
 *
 * When a packet is published to the bus, a receive queue entry for the packet
 * is placed in each subscribed receive queue.
 */
struct pulse_rxq_ent {
    void* buf;     /// Pointer to the buffer with the actual data.
    size_t pkt_size;  /// Packet size in bytes.
    uint32_t pid;     /// packet ID.

    /** The buffer pool entry that the data resides in. */
    struct pulse_bp_ent *bp_ent;
};

/**
 * \brief PULSE subscription list entry.
 *
 * Every time a task/thread subscribes to a packet ID, they get an subscription
 * entry, which is inserted into the sorted subscriber array for the pulse
 * instance.
 */
struct pulse_sub_ent {
    uint32_t pid;                 /// ID of subscribed packet.
    struct mt_queue *subscriber;  /// Pointer to receive queue of subscriber.
};

/**
 * \brief PULSE framework, for managing a publisher-subscriber needs in an
 * embedded environment, and/or one where MPI named pipes, etc. are not
 * available or are not suitable.
 */
struct pulse_inst {
    size_t n_pools;    /// # buffer pools (static during lifetime).
    size_t n_rxqs;     /// # active receive queues (dynamic during lifetime).
    size_t max_rxqs;   /// Max # of receive queues allowed
    size_t max_subs;   /// Max # of subscribers (rxq-pid pairs) allowed.
    mt_mutex_t mutex;  /// mutex to protect access to bus metadata.
    uint32_t flags;    /// Run-time configuration flags.

    /**
     * Array of buffer pool entries. Published data stored here. This is
     * always allocated by PULSE during initialization.
     */
    struct pulse_bp_ent *buffer_pools;

    /**
     * Array of receive queues. Used by the application to subscribe to
     * packets and to receive published packets. This is always allocated by
     * PULSE during initialization.
     */
    struct mt_queue *rx_queues;

    /** List of subscribers (rxq, ID) pairs. Always sorted. */
    struct llist *sub_list;

    /**
     * Name for instance. Used to assist with debugging if multiple PULSE
     * instances are active. Has no effect on PULSE operation.
     */
    char name[PULSE_MAX_NAMELEN];
};

/*******************************************************************************
 * Inline Functions
 ******************************************************************************/
static inline size_t pulse_rxq_n_elts(const struct pulse_inst* const pulse,
                      const struct mt_queue *const queue) {
    RCSW_FPC_NV(0, pulse != NULL, queue != NULL);
    return mt_queue_n_elts(queue);
}

static inline size_t pulse_rxq_n_free(const struct pulse_inst* const pulse,
                                    const struct mt_queue *const queue) {
    RCSW_FPC_NV(0, pulse != NULL, queue != NULL);
    return mt_queue_n_free(queue);
}

/**
 * \brief Get pointer to the top packet on a receive queue.
 *
 * \return The top on the queue, or NULL if no such packet or an error occurred.
 */
static inline uint8_t * pulse_get_top(struct pulse_inst* const pulse,
                                    struct mt_queue *const queue) {
    RCSW_FPC_NV(NULL, pulse != NULL, queue != NULL);
    return (uint8_t*)mt_queue_peek(queue);
}

/*******************************************************************************
 * Inline Functions
 ******************************************************************************/
static inline size_t pulse_pool_space(size_t elt_size, size_t max_elts) {
    return mpool_element_space(elt_size, max_elts);
}

static inline size_t pulse_node_space(size_t max_elts) {
    return mpool_node_space(max_elts);
}

/*******************************************************************************
 * Forward Declarations
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Initialize a pulse instance.
 *
 * \param pulse_in The pulse handle to be filled (can be NULL if
 * \ref PULSE_APP_DOMAIN_HANDLE not passed).
 * \param params The initialization parameters.
 *
 * \return Initialized pulse instance, or NULL if an error occurred.
 */
struct pulse_inst *pulse_init(
    struct pulse_inst *pulse_in,
    const struct pulse_params * params) RCSW_CHECK_RET;

/**
 * \brief  Shutdown a PULSE instance and deallocate its memory.
 *
 * Any further use of th pulse handle after calling this function is undefined.
 *
 * \param pulse The pulse handle.
 */
void pulse_destroy(struct pulse_inst *pulse);

/**
 * \brief Allocate and initialize a receive queue.
 *
 * \param pulse The pulse handle.
 * \param buf_p Space for the rxq entries. Can be NULL (pulse will malloc() for
 *              space).
 * \param n_entries Max # of entries for rxq.
 *
 * \return Pointer to new receive queue, or NULL if an error occurred.
 */
struct mt_queue *pulse_rxq_init(struct pulse_inst * pulse,
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
status_t pulse_subscribe(struct pulse_inst * pulse,
                         struct mt_queue * queue, uint32_t pid);

/**
 * \brief Unsubscribe the specified RXQ from the specified packet ID
 *
 * \param pulse The pulse handle.
 * \param queue The RXQ to unsubscribe.
 * \param pid The PID to unsubscribe from.
 *
 * \return \ref status_t.
 */
status_t pulse_unsubscribe(struct pulse_inst * pulse,
                           struct mt_queue * queue, uint32_t pid);

/**
 * \brief Publish a packet to the bus. A memcpy() will be performed, if the
 * packet is very large, consider using \ref pulse_publish_release() instead;
 * it will not perform a memcpy().
 *
 * \param pulse The pulse handle.
 * \param pid The packet ID.
 * \param pkt_size The size of the packet in bytes.
 * \param pkt The packet to publish.
 *
 * \return \ref status_t
 */
status_t pulse_publish(struct pulse_inst * pulse, uint32_t pid,
                       size_t pkt_size, const void * pkt);
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
 * \param pid The packet ID.
 * \param bp_ent The buffer pool entry reserved for the packet, as a result of
 * \ref pulse_publish_reserve(). Should be NULL if called directly by
 * application.
 * \param reservation The space reserved for the packet in a particular buffer
 * pool. Should be NULL if called directly by the application.
 * \param pkt_size Size of the packet in bytes.
 *
 * \return \ref status_t.
 */
status_t pulse_publish_release(struct pulse_inst* pulse, uint32_t pid,
                               struct pulse_bp_ent* bp_ent,
                               void * reservation, size_t pkt_size);

/**
 * \brief Wait (indefinitely) until the given receive queue is not empty,
 * returning a reference to the first item in the queue.
 *
 * \param queue The receive queue to wait on.
 *
 * \return A reference to the first item in the queue, or NULL if an ERROR
 * occurred.
 */
void *pulse_wait_front(struct mt_queue * queue) RCSW_CHECK_RET;

/**
 * \brief Wait (until a timeout) until the given receive queue is not empty.
 *
 * \param queue The receive queue to wait on.
 * \param to A RELATIVE timeout.
 *
 * \return A reference to the first item in the queue, or NULL if an ERROR or a
 * timeout occurred.
 */
void *pulse_timedwait_front(struct mt_queue * queue,
                            struct timespec * to) RCSW_CHECK_RET;

/**
 * \brief Remove the front element from the selected receive queue.
 *
 * \param queue The receive queue remove from.
 *
 * \return \ref status_t.
 */
status_t pulse_pop_front(struct mt_queue * queue);

END_C_DECLS

