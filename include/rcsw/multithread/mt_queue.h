/**
 * \file mt_queue.h
 * \ingroup multithread
 * \brief Producer-consumer queue implementation.
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * This file is part of RCSW.
 *
 * RCSW is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * RCSW is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * RCSW.  If not, see <http://www.gnu.org/licenses/
 */

#ifndef INCLUDE_RCSW_MULTITHREAD_MT_QUEUE_H_
#define INCLUDE_RCSW_MULTITHREAD_MT_QUEUE_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/multithread/mt_csem.h"
#include "rcsw/multithread/mt_mutex.h"
#include "rcsw/ds/fifo.h"

/*******************************************************************************
 * Structure Definitions
 ******************************************************************************/
/**
 * \brief Producer-consumer queue initialization parameters.
 */
struct mt_queue_params {
    size_t el_size;     /// Size of each element in the queue.
    size_t max_elts;    /// # of elements the queue will hold.
    uint8_t *elements;  /// The elements the FIFO will manage. Can be NULL.

    /**
     * Configuration flags. You can pass an DS flag that applies to a FIFO.
     */
    uint32_t flags;
};

/**
 * \brief Producer-consumer queue, providing thread-safe access to data at both
 * ends of a FIFO.
 */
struct mt_queue {
    struct fifo fifo;   /// The underlying FIFO.
    uint32_t flags;     /// Configuration flags.
    mt_mutex_t mutex;   /// Mutex protecting buffer, in, out, count.
    mt_csem_t empty;    /// Semaphore counting empty slots in buffer.
    mt_csem_t full;     /// Semaphore counting full slots in buffer.
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
static inline bool_t mt_queue_isfull(const struct mt_queue* const queue) {
    RCSW_FPC_NV(FALSE, NULL != queue);
    return fifo_isfull(&queue->fifo);
}

/**
 * \brief Determine if the queue is currently empty.
 *
 * \param queue The linked queue handle.
 *
 * \return \ref bool_t
 */
static inline bool_t mt_queue_isempty(const struct mt_queue* const queue) {
    RCSW_FPC_NV(FALSE, NULL != queue);
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
static inline size_t mt_queue_n_elts(const struct mt_queue* const queue) {
    RCSW_FPC_NV(0, NULL != queue);
    return fifo_n_elts(&queue->fifo);
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
static inline size_t mt_queue_capacity(const struct mt_queue* const queue) {
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
static inline size_t mt_queue_n_free(const struct mt_queue* const queue) {
    RCSW_FPC_NV(0, NULL != queue);
    return mt_queue_capacity(queue) - mt_queue_n_elts(queue);
}

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Initialize a producer-consumer queue.
 *
 * \param mt_queue_in An application allocated handle for the queue. Can be
 * NULL, depending on if \ref DS_APP_DOMAIN_HANDLE is passed or not.
 * \param params The initialization parameters.
 *
 * \return The initialized queue, or NULL if an error occurred.
 */
struct mt_queue * mt_queue_init(
    struct mt_queue *mt_queue_in,
    const struct mt_queue_params * params) RCSW_CHECK_RET;

/**
 * \brief Destroy a producer-consumer queue. Any further use of the queue handle
 * after calling this function is undefined.
 *
 * \param mt_queue The queue handle.
 */
void mt_queue_destroy(struct mt_queue * mt_queue);

/**
 * \brief Push an item to the back of the queue, waiting if necessary for space
 * to become available.
 *
 * \param mt_queue The queue handle.
 * \param e The item to enqueue.
 *
 * \return \ref status_t.
 */
status_t mt_queue_push(struct mt_queue * mt_queue, const void * e);

/**
 * \brief Pop and return the first element in the queue, waiting if
 * necessary for the queue to become non-empty.
 *
 * \param mt_queue The queue handle.
 * \param e The item to dequeue. Can be NULL.
 *
 * \return \ref status_t.
 */
status_t mt_queue_pop(struct mt_queue * mt_queue, void * e);

/**
 * \brief Pop and return the first element in the queue, waiting until the
 * timeout if necessary for the queue to become non-empty.
 *
 * \param mt_queue The queue handle.
 * \param to A RELATIVE timeout.
 * \param e The item to dequeue. Can be NULL.
 *
 * \return \ref status_t.
 */
status_t mt_queue_timed_pop(struct mt_queue * mt_queue,
                            const struct timespec * to, void * e);

/**
 * \brief Get a reference to the first element in the queue if it exists. The
 * non-NULL value returned by this function cannot be relied upon in a
 * multi-threaded context without additional synchronization.
 *
 * \param mt_queue The queue handle.
 *
 * \return A reference to the first element in the queue, or NULL if no such
 * element or an error occurred.
 */
void* mt_queue_peek(struct mt_queue * mt_queue);

END_C_DECLS

#endif /* INCLUDE_RCSW_MULTITHREAD_MT_QUEUE_H_  */
