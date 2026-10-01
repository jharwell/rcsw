/**
 * \file
 *
 * \copyright 2017 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * \ingroup multithread
 *
 * \brief Thread-safe producer-consumer queue.
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <time.h>

#include "rcsw/al/types.h"
#include "rcsw/core/compilers.h"
#include "rcsw/core/core.h"
#include "rcsw/core/fpc.h"
#include "rcsw/ds/fifo.h"
#include "rcsw/multithread/csem.h"
#include "rcsw/multithread/mutex.h"

/*******************************************************************************
 * Types
 ******************************************************************************/
/**
 * \brief Producer-consumer queue initialization parameters.
 */
// NOLINTNEXTLINE(readability-identifier-naming)
#define pcqueue_config fifo_config

/**
 * \brief Producer-consumer queue, providing thread-safe access to data at both
 * ends of a FIFO.
 */
struct pcqueue {
  /**
   * The underlying FIFO.
   */
  struct fifo fifo;
  /**
   * Mutex protecting fifo.
   */

  struct mutex mutex;

  /**
   * Semaphore counting empty slots in FIFO.
   */
  struct csem slots_avail;

  /**
   * Semaphore counting occupied/allocated slots in FIFO.
   */
  struct csem slots_inuse;

  /**
   * \brief Run-time configuration flags.
   *
   * Valid flags are:
   *
   * - \ref RCSW_ZALLOC
   * - \ref RCSW_NOALLOC_HANDLE
   * - \ref RCSW_NOALLOC_DATA
   *
   * All other flags are ignored.
   */
  uint32_t flags;
};

/*******************************************************************************
 * Public API
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Determine if the queue is currently full.
 *
 * \param queue The queue  handle.
 */
static inline bool_t pcqueue_isfull(const struct pcqueue* const queue) {
  RCSW_FPC_NV(false, NULL != queue);
  return fifo_isfull(&queue->fifo);
}

/**
 * \brief Determine if the queue is currently empty.
 *
 * \param queue The linked queue handle.
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

/**
 * \brief Initialize a producer-consumer queue.
 *
 * \param pcqueue_in Caller storage for the handle, used only if \ref
 *                   RCSW_NOALLOC_HANDLE is passed; ignored (may be NULL)
 *                   otherwise. See \rcswdoc{concepts/memory-model}.
 *
 * \param params The initialization parameters.
 *
 * \return The initialized queue, or NULL if an error occurred.
 */
RCSW_API struct pcqueue* pcqueue_init(
  struct pcqueue* pcqueue_in, const struct pcqueue_config* params) RCSW_WUR;

/**
 * \brief Destroy a producer-consumer queue.
 *
 * Any further use of the queue handle after calling this function is undefined.
 *
 * \param pcqueue The queue handle.
 */
RCSW_API void pcqueue_destroy(struct pcqueue* pcqueue);

/**
 * \brief Push an item to the back of the queue, waiting if necessary for space
 * to become available.
 *
 * \param pcqueue The queue handle.
 * \param e The item to enqueue.
 *
 * \return \ref status_t.
 */
RCSW_API status_t pcqueue_push(struct pcqueue* pcqueue, const void* e);

/**
 * \brief Push an item to the back of the queue if there is space, without
 * waiting.
 *
 * \param pcqueue The queue handle.
 * \param e The item to enqueue.
 *
 * \return \ref status_t. If the queue is full, ERROR with errno=ENOSPC.
 */
RCSW_API status_t pcqueue_trypush(struct pcqueue* pcqueue, const void* e);

/**
 * \brief Pop and return the first element in the queue, waiting if
 * necessary for the queue to become non-empty.
 *
 * \param pcqueue The queue handle.
 * \param e The item to dequeue. Can be NULL.
 *
 * \return \ref status_t.
 */
RCSW_API status_t pcqueue_pop(struct pcqueue* pcqueue, void* e);

/**
 * \brief Pop and return the first element in the queue, waiting until the
 * timeout if necessary for the queue to become non-empty.
 *
 * \param pcqueue The queue handle.
 * \param to A relative timeout. See \rcswdoc{concepts/concurrency/timeouts}.
 * \param e The item to dequeue. Can be NULL.
 *
 * \return \ref status_t.
 */
RCSW_API status_t pcqueue_timedpop(struct pcqueue*        pcqueue,
                                   const struct timespec* to,
                                   void*                  e);

/**
 * \brief Get the first element in the queue if it exists, with a timeout.
 *
 * \note The filled value returned by this function cannot be relied upon in a
 * multi-threaded context without additional synchronization.
 *
 * \param queue The queue handle.
 *
 * \param to A relative timeout. See
 *           \rcswdoc{concepts/concurrency/timeouts}.
 *
 * \param e To be filled with the address of the first element, if it exists,
 *          and set to NULL otherwise.
 *
 * \return \ref status_t.
 */
RCSW_API status_t pcqueue_timedpeek(struct pcqueue*        queue,
                                    const struct timespec* to,
                                    void**                 e);
/**
 * \brief Get the first element in the queue if it exists.
 *
 * \note The filled value returned by this function cannot be relied upon in a
 * multi-threaded context without additional synchronization.
 *
 * \param queue The queue handle.
 *
 * \param e To be filled with the address of the first element, if it exists,
 *          and set to NULL otherwise.
 *
 * \return \ref status_t.
 */
RCSW_API status_t pcqueue_peek(struct pcqueue* queue, void** e);

RCSW_API status_t pcqueue_waitpeek(struct pcqueue* queue, void** e);

END_C_DECLS
