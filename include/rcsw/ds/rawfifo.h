/**
 * \file rawfifo.h
 * \ingroup ds
 * \brief "Raw" FIFO implementation.
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/rcsw.h"
#include "rcsw/common/fpc.h"

/*******************************************************************************
 * Type Definitions
 ******************************************************************************/
/**
 * \brief A raw FIFO that is fast, and has a limited API.
 *
 * Can (and should) only be used with elements of 1, 2 or 4 bytes. This is to
 * make all add/remove operations atomic at the instruction level via pointer
 * arithmetic, so that it can be used in ISRs safely.
 *
 * For general (read: non-ISR) FIFO things, \ref fifo should be used
 * instead.
 */
struct rawfifo {
  /**
   * The actual elements.
   */
  dptr_t *elements;

  /**
   * Element where we write next.
   */
  size_t to_i;

  /**
   * Element where we read next.
   */
  size_t from_i;
  /**
   * Max # of elts = fifo elts + 1.
   */
  size_t max_elts;

  /**
   * Size of element in bytes.
   */
  size_t elt_size;
};

/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS
/**
 * \brief  Empty the FIFO (don't deallocate its data)
 *
 * \param fifo The FIFO handle.
 *
 * \return \ref status_t.
 */
static inline status_t rawfifo_clear(struct rawfifo *const fifo) {
    RCSW_FPC_NV(ERROR, NULL != fifo);
    fifo->to_i = fifo->from_i;
    return OK;
}

/**
 * \brief Get # elements currently in FIFO.
 *
 * \param fifo The FIFO handle.
 *
 * \return # element on the FIFO; 0 on ERROR.
 */
static inline size_t rawfifo_size(const struct rawfifo *const fifo) {
    RCSW_FPC_NV(0, NULL != fifo);
    if (fifo->to_i >= fifo->from_i) {
        return fifo->to_i - fifo->from_i;
    }
    return (fifo->to_i) + (fifo->max_elts - fifo->from_i);
} /* rawfifo_size() */

/**
 * \brief Get # of free slots remaining in FIFO.
 *
 * \param fifo The FIFO handle.
 *
 * \return # free elements; 0 on ERROR.
 */
static inline size_t rawfifo_n_free(const struct rawfifo *const fifo) {
    RCSW_FPC_NV(0, NULL != fifo);
    /* One elt must be wasted to make n_elts determination unambiguous */
    return fifo->max_elts - rawfifo_size(fifo) - 1;
}

/**
 *
 * \brief Initialize the raw fifo structure. Note: this FIFO holds 1 less than
 * the number of array/buffer elements specified by max_elts.
 *
 * \param fifo The FIFO handle, to be filled.
 * \param buf The provided space for the FIFO elements.
 * \param max_elts The max # of elements in the FIFO.
 * \param elt_size Size of elements in bytes.
 *
 * \return \ref status_t.
 */
RCSW_API status_t rawfifo_init(struct rawfifo * fifo,
                      void* buf,
                      size_t max_elts,
                      size_t elt_size);

/**
 * \brief Removes top N elements from the FIFO.
 *
 * \param fifo The FIFO handle.
 * \param e The array to dequeue elements into.
 * \param n_elts # elements to remove.
 *
 * \return # of elements removed from the FIFO.
 */
RCSW_API size_t rawfifo_deq(struct rawfifo * fifo, void * e, size_t n_elts);

/**
 * \brief Adds N elements to the FIFO.
 *
 * \param fifo The FIFO handle.
 * \param elts The elements to add.
 * \param n_elts # elements to remove.
 * \return # of elements added to the FIFO.
 */
RCSW_API size_t rawfifo_enq(struct rawfifo * fifo,
                            const void * elts,
                            size_t n_elts);

END_C_DECLS
