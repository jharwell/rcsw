/**
 * \file fifo.h
 * \ingroup ds
 * \brief Implementation of simple FIFO.
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/rbuffer.h"
#include "rcsw/common/fpc.h"

/*******************************************************************************
 * Structure Definitions
 ******************************************************************************/
/**
 * \brief Parameters for \ref fifo.
 */
struct fifo_params {
  /**
   * For printing an element. Can be NULL. If NULL, you can't use \ref
   * fifo_print().
   */
  void (*printe)(const void *e);

  /**
   * Pointer to application-allocated space for storing data managed by the \ref
   * fifo. Ignored unless \ref RCSW_NOALLOC_DATA is passed.
   */
  dptr_t *elements;

  /**
   * Size of elements in bytes.
   */
  size_t elt_size;

  /**
   * Maximum number of elements allowed
   * fifo).
   */
  size_t max_elts;

  /**
   * Configuration flags. See \ref fifo.flags for valid flags.
   */
  uint32_t flags;
};

/**
 * \brief Out general purpose FIFO.
 *
 * This implementation is more full-featured than \ref rawfifo, but is not safe
 * to use in ISRs.
 */
struct fifo {
  /**
   * Underlying ringbuffer the FIFO is built on top of.
   */
  struct rbuffer rb;

  /**
   * Run-time configuration parameters. Valid flags are:
   *
   * - \ref RCSW_ZALLOC
   * - \ref RCSW_NOALLOC_HANDLE
   * - \ref RCSW_NOALLOC_DATA
   *
   * All other flags are ignored.
   */
  uint32_t flags;
};

BEGIN_C_DECLS

/*******************************************************************************
 * Inline Functions
 ******************************************************************************/
/**
 * \brief Determine if the FIFO is currently full
 *
 * \param fifo The FIFO handle
 *
 * \return \ref bool_t
 */
static inline bool_t fifo_isfull(const struct fifo* const fifo) {
    RCSW_FPC_NV(false, NULL != fifo);
    return rbuffer_isfull(&fifo->rb);
}

/**
 * \brief Determine if the FIFO is currently empty
 *
 * \param fifo The FIFO handle
 *
 * \return \ref bool_t
 */
static inline bool_t fifo_isempty(const struct fifo* const fifo) {
    RCSW_FPC_NV(false, NULL != fifo);
    return rbuffer_isempty(&fifo->rb);
}

/**
 * \brief Determine # elements currently in the FIFO.
 *
 * \param fifo The FIFO handle.
 *
 * \return # elements in FIFO, or 0 on ERROR.
 */

static inline size_t fifo_size(const struct fifo* const fifo) {
    RCSW_FPC_NV(0, NULL != fifo);
    return rbuffer_size(&fifo->rb);
}

/**
 * \brief Get the FIFO capacity.
 *
 * \param fifo The FIFO handle.
 *
 * \return Capacity of the FIFO, or 0 on ERROR.
 */
static inline size_t fifo_capacity(const struct fifo* const fifo) {
    RCSW_FPC_NV(0, NULL != fifo);
    return rbuffer_capacity(&fifo->rb);
}

/**
 * \brief Get the first FIFO item without removing it.
 *
 * \param fifo The FIFO handle.
 *
 * \return Pointer to the first element, or NULL if no such element or an error
 * occurred.
 */
static inline void* fifo_front(const struct fifo* const fifo) {
    RCSW_FPC_NV(0, NULL != fifo);
    return rbuffer_front(&fifo->rb);
}

/**
 * \brief Calculate the # of bytes that the FIFO will require if \ref
 * RCSW_NOALLOC_DATA is passed to manage a specified # of elements of a
 * specified size
 *
 * \param max_elts # of desired elements the FIFO will hold
 * \param el_size size of elements in bytes
 *
 * \return The total # of bytes the application would need to allocate
 */
static inline size_t fifo_element_space(size_t max_elts, size_t el_size) {
    return rbuffer_element_space(max_elts, el_size);
}

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
/**
 * \brief Initialize a FIFO.
 *
 * \param fifo_in An application allocated handle for the FIFO. Cannot be NULL,
 *                if \ref RCSW_NOALLOC_HANDLE is passed in \ref
 *                fifo_params.flags.
 *
 * \param params The initialization parameters.
 *
 * \return The initialized FIFO, or NULL if an error occurred.
 */
struct fifo *fifo_init(struct fifo *fifo_in,
                       const struct fifo_params * params) RCSW_CHECK_RET;

/**
 * \brief Destroy a FIFO.
 *
 * Any further use of the FIFO after this function is called is undefined.
 *
 * \param fifo The FIFO to destroy.
 */
void fifo_destroy(struct fifo *fifo);

/**
 * \brief Enqueue an element into the FIFO.
 *
 * \param fifo The FIFO handle.
 * \param e The element to enqueue. Cannot be NULL.
 *
 * \return \ref status_t.
 */
status_t fifo_add(struct fifo * fifo, const void * e);

/**
 * \brief Dequeue an element from the FIFO.
 *
 * \param fifo The FIFO handle.
 * \param e The element to dequeue into from the FIFO. Can be NULL.
 *
 * \return \ref status_t.
 */
status_t fifo_remove(struct fifo * fifo, void * e);

/**
 * \brief Clear a FIFO.
 *
 * Empty the FIFO, but do not deallocate its memory.
 *
 * \return \ref status_t.
 */
status_t fifo_clear(struct fifo * fifo);

/**
 * \brief Apply an operation to all elements of the FIFO.
 *
 * \param fifo The FIFO handle.
 * \param f A function pointer which will be called for every element in the.
 * FIFO. This function CAN modify FIFO elements.
 *
 * \return \ref status_t.
 */
status_t fifo_map(struct fifo * fifo, void (*f)(void *e));

/**
 * \brief Compute a cumulative SOMETHING over all elements of a FIFO.
 *
 * \param fifo The FIFO handle.
 * \param f A function point which will be called for every element in the
 * FIFO. This function CAN modify FIFO elements.
 * \param result The initial value for the cumulative SOMETHING to be
 * computed. This will be passed to each invocation of the callback.
 *
 * \return \ref status_t.
 */
status_t fifo_inject(struct fifo * fifo,
                     void (*f)(void *e, void *res), void *result);

/**
 * \brief Print a FIFO.
 *
 * \param fifo The FIFO handle.
 **/
void fifo_print(struct fifo * fifo);

END_C_DECLS
