/**
 * \file rawfifo.h
 * \ingroup ds
 * \brief "Raw" FIFO implementation.
 *
 * The rawfifo can handle only elements of 1,2 or 4 bytes. This is to make all
 * add/remove operations atomic at the instruction level via pointer arithmetic,
 * so that it can be used in ISRs safely.
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

#ifndef INCLUDE_RCSW_DS_RAWFIFO_H_
#define INCLUDE_RCSW_DS_RAWFIFO_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/common/common.h"
#include "rcsw/common/fpc.h"

/*******************************************************************************
 * Type Definitions
 ******************************************************************************/
/**
 * \brief A raw FIFO that is fast, and has a limited API.
 *
 * This is a VERY simple implementation (it does not even has a destroy()
 * function because all memory is always provided by the application). For
 * general FIFO things, the general FIFO should be used instead--it is much more
 * full featured.
 */
struct rawfifo {
    uint8_t *elements;  /// The actual elements.
    size_t to_i;        /// Element where we write next.
    size_t from_i;      /// Element where we read next.
    size_t max_elts;    /// Max # of elts = fifo elts + 1.
    size_t el_size;     /// Size of element in bytes.
};

/*******************************************************************************
 * Inline Functions
 ******************************************************************************/
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
static inline size_t rawfifo_n_elts(const struct rawfifo *const fifo) {
    RCSW_FPC_NV(0, NULL != fifo);
    if (fifo->to_i >= fifo->from_i) {
        return fifo->to_i - fifo->from_i;
    }
    return (fifo->to_i) + (fifo->max_elts - fifo->from_i);
} /* rawfifo_n_elts() */

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
    return fifo->max_elts - rawfifo_n_elts(fifo) - 1;
}

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
BEGIN_C_DECLS

/**
 *
 * \brief Initialize the raw fifo structure. Note: this FIFO holds 1 less than the
 * number of array/buffer elements specified by max_elts.
 *
 * \param fifo The FIFO handle, to be filled.
 * \param buf The provided space for the FIFO elements.
 * \param max_elts The max # of elements in the FIFO.
 * \param el_size Size of elements in bytes.
 *
 * \return \ref status_t.
 */
status_t rawfifo_init(struct rawfifo * fifo, uint8_t * buf,
                      size_t max_elts, size_t el_size);

/**
 * \brief Removes top N elements from the FIFO.
 *
 * \param fifo The FIFO handle.
 * \param e The array to dequeue elements into.
 * \param n_elts # elements to remove.
 *
 * \return # of elements removed from the FIFO.
 */
size_t rawfifo_deq(struct rawfifo * fifo, void * e, size_t n_elts);

/**
 * \brief Adds N elements to the FIFO.
 *
 * \param fifo The FIFO handle.
 * \param elts The elements to add.
 * \param n_elts # elements to remove.
 * \return # of elements added to the FIFO.
 */
size_t rawfifo_enq(struct rawfifo * fifo, const void * elts,
                   size_t n_elts);

END_C_DECLS

#endif /* INCLUDE_RCSW_DS_RAWFIFO_H_ */
