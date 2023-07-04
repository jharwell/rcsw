/**
 * \file pcqueue.h
 * \ingroup multithread
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/multithread/csem.h"
#include "rcsw/multithread/mutex.h"
#include "rcsw/ds/fifo.h"

/*******************************************************************************
 * Structure Definitions
 ******************************************************************************/
/**
 * \brief Producer-consumer queue initialization parameters.
 */
#define pcqueue_params fifo_params

/**
 * \brief Producer-consumer queue, providing thread-safe access to data at both
 * ends of a FIFO.
 */
struct pcqueue {
  struct fifo fifo;    /// The underlying FIFO.
  struct mutex mutex;  /// Mutex protecting fifo.
  struct csem slots_avail;   /// Semaphore counting empty slots in buffer.
  struct csem slots_inuse;    /// Semaphore counting full slots in buffer.
  uint32_t flags;      /// Configuration flags.
};

/*******************************************************************************
 * Macros
 ******************************************************************************/
/**
 * \brief Determine if the queue is currently full.
 *
 * \param queue The queue  handle.
 *
 * \return \ref bool_t
 */
static inline bool_t pcqueue_isfull(const struct pcqueue* const queue) {
    RCSW_FPC_NV(false, NULL != queue);
    return fifo_isfull(&queue->fifo);
}

/**
 * \brief Determine if the queue is currently empty.
 *
 * \param queue The linked queue handle.
 *
 * \return \ref bool_t
 */
static inline bool_t pcqueue_isempty(const struct pcqueue* const queue) {
    RCSW_FPC_NV(false, NULL != queue);
    return fifo_isempty(&queue->fifo);
}

/**
 * \brief Determine # elements currently in the queue. The value returned by
 * this function should not be relied upon for accuracy among multiple threads
 * without additional synchronization.
 *
 * \param queue The queue handle.
 *
 * \return # elements in queue, or 0 on ERROR.
 */
static inline size_t pcqueue_size(const struct pcqueue* const queue) {
    RCSW_FPC_NV(0, NULL != queue);
    return fifo_size(&queue->fifo);
}

/**
 * \brief Get the capacity of the queue. The value returned can be relied upon
 * in a multi-thread context, because it does not change during the lifetime of
 * the queue.
 *
 * \param queue The queue handle.
 *
 * \return Queue capacity, or 0 on ERROR.
 */
static inline size_t pcqueue_capacity(const struct pcqueue* const queue) {
    RCSW_FPC_NV(0, NULL != queue);
    return fifo_capacity(&queue->fifo);
}

/**
 * \brief Get the # slots available in the queue. The value returned cannot be
 * relied upon in a multi-thread context without additional synchronization.
 *
 * \param queue The queue handle.
 *
 * \return # free slots, or 0 on ERROR.
 */
static inline size_t pcqueue_n_free(const struct pcqueue* const queue) {
    RCSW_FPC_NV(0, NULL != queue);
    return pcqueue_capacity(queue) - pcqueue_size(queue);
}

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Initialize a producer-consumer queue.
 *
 * \param pcqueue_in An application allocated handle for the queue. Can be NULL,
 *                   depending on if \ref RCSW_NOALLOC_HANDLE is passed or not.
 *
 * \param params The initialization parameters.
 *
 * \return The initialized queue, or NULL if an error occurred.
 */
struct pcqueue * pcqueue_init(
    struct pcqueue *pcqueue_in,
    const struct pcqueue_params * params) RCSW_CHECK_RET;

/**
 * \brief Destroy a producer-consumer queue.
 *
 * Any further use of the queue handle after calling this function is undefined.
 *
 * \param pcqueue The queue handle.
 */
void pcqueue_destroy(struct pcqueue * pcqueue);

/**
 * \brief Push an item to the back of the queue, waiting if necessary for space
 * to become available.
 *
 * \param pcqueue The queue handle.
 * \param e The item to enqueue.
 *
 * \return \ref status_t.
 */
status_t pcqueue_push(struct pcqueue * pcqueue, const void * e);

/**
 * \brief Pop and return the first element in the queue, waiting if
 * necessary for the queue to become non-empty.
 *
 * \param pcqueue The queue handle.
 * \param e The item to dequeue. Can be NULL.
 *
 * \return \ref status_t.
 */
status_t pcqueue_pop(struct pcqueue * pcqueue, void * e);

/**
 * \brief Pop and return the first element in the queue, waiting until the
 * timeout if necessary for the queue to become non-empty.
 *
 * \param pcqueue The queue handle.
 * \param to A RELATIVE timeout.
 * \param e The item to dequeue. Can be NULL.
 *
 * \return \ref status_t.
 */
status_t pcqueue_timedpop(struct pcqueue * pcqueue,
                            const struct timespec * to,
                           void * e);

/**
 * \brief Get the first element in the queue if it exists.
 *
 * \note The filled value returned by this function cannot be relied upon in a
 * multi-threaded context without additional synchronization.
 *
 * \param pcqueue The queue handle.
 *
 * \param e To be filled with the address of the first element, if it exists,
 *          and set to NULL otherwise.
 *
 * \return \ref status_t.
 */
status_t pcqueue_timedpeek(struct pcqueue* const queue,
                           const struct timespec* const to,
                           uint8_t** const e);
/**
 * \brief Get the first element in the queue if it exists, with a timeout.
 *
 * \note The filled value returned by this function cannot be relied upon in a
 * multi-threaded context without additional synchronization.
 *
 * \param pcqueue The queue handle.
 *
 * \param e To be filled with the address of the first element, if it exists,
 *          and set to NULL otherwise.
 *
 * \return \ref status_t.
 */
status_t pcqueue_peek(struct pcqueue* const queue, uint8_t** const e);

END_C_DECLS
