/**
 * \file
 *
 * \copyright 2017 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * \ingroup ds
 *
 * \brief Lock-free single-producer, single-consumer FIFO for 1, 2, or 4-byte
 * elements.
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <stdatomic.h>

#include "rcsw/al/types.h"
#include "rcsw/core/compilers.h"
#include "rcsw/core/core.h"
#include "rcsw/core/fpc.h"

/*******************************************************************************
 * Type Definitions
 ******************************************************************************/
/**
 * \brief A raw FIFO that is fast, and has a limited API.
 *
 * Elements must be 1, 2 or 4 bytes. The FIFO is lock-free and safe for exactly
 * one producer and one consumer running concurrently (e.g., an ISR and the main
 * loop, or two threads):
 *
 * - The producer calls \ref rawfifo_enq().
 * - The consumer calls \ref rawfifo_deq() and \ref rawfifo_clear().
 * - Either side may call \ref rawfifo_size() and \ref rawfifo_n_free(); the
 *   result is a snapshot which is conservative for the caller (the producer
 *   never sees fewer free slots than really exist, the consumer never sees
 *   more elements than really exist).
 *
 * Multiple producers or multiple consumers require external locking. For
 * general (read: non-ISR) FIFO things, \ref fifo should be used instead.
 */
struct rawfifo {
  /**
   * The actual elements.
   */
  dptr_t* elements;

  /**
   * Element where we write next (atomic). Written only by the producer.
   */
  _Atomic(size_t) to_i;

  /**
   * Element where we read next (atomic). Written only by the consumer.
   */
  _Atomic(size_t) from_i;

  /**
   * # of slots in the buffer. The FIFO holds at most \c max_elts - 1.
   */
  size_t max_elts;

  /**
   * Size of element in bytes.
   */
  size_t elt_size;
};

/*******************************************************************************
 * Public API
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Empty the FIFO by discarding all elements currently in it (doesn't
 * deallocate anything).
 *
 * This is a consumer-side operation.
 *
 * \param fifo The FIFO handle.
 *
 * \return \ref status_t.
 */
static inline status_t rawfifo_clear(struct rawfifo* const fifo) {
  RCSW_FPC_NV(ERROR, NULL != fifo);
  atomic_store_explicit(&fifo->from_i,
                        atomic_load_explicit(&fifo->to_i, memory_order_acquire),
                        memory_order_release);
  return OK;
}

/**
 * \brief Get # elements currently in FIFO.
 *
 * \param fifo The FIFO handle.
 *
 * \return # elements in the FIFO; 0 on ERROR.
 */
static inline size_t rawfifo_size(const struct rawfifo* const fifo) {
  RCSW_FPC_NV(0, NULL != fifo);
  /* snapshot each index exactly once; the other side may be moving it */
  size_t to   = atomic_load_explicit(&fifo->to_i, memory_order_acquire);
  size_t from = atomic_load_explicit(&fifo->from_i, memory_order_acquire);
  if (to >= from) {
    return to - from;
  }
  return to + (fifo->max_elts - from);
} /* rawfifo_size() */

/**
 * \brief Get # of free slots remaining in FIFO.
 *
 * \param fifo The FIFO handle.
 *
 * \return # free elements; 0 on ERROR.
 */
static inline size_t rawfifo_n_free(const struct rawfifo* const fifo) {
  RCSW_FPC_NV(0, NULL != fifo);
  /* One elt must be wasted to make n_elts determination unambiguous */
  return fifo->max_elts - rawfifo_size(fifo) - 1;
}

/**
 * \brief Initialize a raw FIFO.
 *
 * \note The FIFO holds one less than \p max_elts elements.
 *
 * \param fifo The FIFO handle, to be filled.
 * \param buf Space for the elements: \p max_elts * \p elt_size bytes, aligned
 *            to \p elt_size. Always caller-provided.
 * \param max_elts # of slots in \p buf.
 * \param elt_size Size of elements in bytes: 1, 2, or 4.
 *
 * \return \ref status_t.
 */
RCSW_API status_t rawfifo_init(struct rawfifo* fifo,
                               void*           buf,
                               size_t          max_elts,
                               size_t          elt_size);

/**
 * \brief Remove up to \p n_elts elements from the front of the FIFO.
 *
 * Consumer side.
 *
 * \param fifo The FIFO handle.
 * \param e The array to dequeue elements into.
 * \param n_elts # elements to remove.
 *
 * \return # of elements removed, which is less than \p n_elts if the FIFO
 * held fewer.
 */
RCSW_API size_t rawfifo_deq(struct rawfifo* fifo, void* e, size_t n_elts);

/**
 * \brief Add up to \p n_elts elements to the FIFO.
 *
 * Producer side.
 *
 * \param fifo The FIFO handle.
 * \param elts The elements to add.
 * \param n_elts # elements to add.
 *
 * \return # of elements added, which is less than \p n_elts if the FIFO
 * filled up.
 */
RCSW_API size_t rawfifo_enq(struct rawfifo* fifo,
                            const void*     elts,
                            size_t          n_elts);

END_C_DECLS
