/**
 * \file rbuffer.h
 * \ingroup ds
 * \brief Implementation of a ringbuffer.
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/ds.h"
#include "rcsw/ds/iter.h"
#include "rcsw/common/fpc.h"

/*******************************************************************************
 * Structure Definitions
 ******************************************************************************/
/**
 * \brief Parameters for \ref rbuffer.
 */
struct rbuffer_params {
  /**
   * For comparing elements. Can be NULL. If NULL, \ref rbuffer_index_query() is
   * disabled.
   */
  int (*cmpe)(const void *const e1, const void *const e2);

  /**
   * For printing an element. Can be NULL. If NULL, \ref rbuffer_print() is
   * disabled.
   */
  void (*printe)(const void *e);

  /**
   * Pointer to application-allocated space for storing the elements managed by
   * the \ref rbuffer. Ignored unless \ref RCSW_NOALLOC_DATA is passed.
   */
  uint8_t *elements;

  /**
   * Size of elements in bytes.
   */
  size_t elt_size;

  /**
   * Maximum number of elements allowed.
   */
  size_t max_elts;

  /**
   * Configuration flags. See \ref rbuffer.flags for valid flags.
   */
  uint32_t flags;
};

/**
 * \brief Ringbuffer data structure. Implemented using a fixed-size array.
 */
struct rbuffer {
  /**
   * For printing an element. Can be NULL. If NULL, calling \ref rbuffer_print()
   * is undefined.
   */
  void (*printe)(const void *const e);

  /**
   * For comparing elements. Can be NULL. If NULL, calling \ref
   * rbuffer_index_query() is undefined.
   */
  int (*cmpe)(const void *const e1, const void *const e2);

  struct ds_iterator iter;

  /**
   * The actual data.
   */
  uint8_t *elements;

  /**
   * Current # of elements in buffer.
   */
  size_t current;

  /**
   * Maximum number of elements in buffer.
   */
  size_t max_elts;

  /**
   * Size of an element in bytes
   */
  size_t elt_size;

  /**
   * Next element insert index.
   */
  size_t start;

  /**
   * \brief Run-time configuration flags.
   *
   * Valid flags are:
   *
   * - \ref RCSW_NOALLOC_HANDLE
   * - \ref RCSW_NOALLOC_DATA
   * - \ref RCSW_DS_RBUFFER_AS_FIFO
   *
   * All other flags are ignored.
   */
  uint32_t flags;
};

/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Determine if the ringbuffer is currently full.
 *
 * \note If \ref RCSW_DS_RBUFFER_AS_FIFO is not passed (i.e., regular
 * ringbuffers), the concept of "full" doesn't really make sense.
 *
 * \param rb The ringbuffer handle.
 *
 * \return \ref bool_t
 */
static inline bool_t rbuffer_isfull(const struct rbuffer* const rb) {
  RCSW_FPC_NV(false, NULL != rb);

  return rb->current == rb->max_elts;
}

/**
 * \brief Determine if the ringbuffer is currently empty.
 *
 * \param rb The ringbuffer handle.
 *
 * \return \ref bool_t
 */
static inline bool_t rbuffer_isempty(const struct rbuffer* const rb) {
  RCSW_FPC_NV(false, NULL != rb);
  return (rb->current == 0);
}

/**
 * \brief Determine # elements currently in the ringbuffer.
 *
 * \param rb The ringbuffer handle.
 *
 * \return # elements in ringbuffer, or 0 on ERROR.
 */
static inline size_t rbuffer_size(const struct rbuffer* const rb) {
  RCSW_FPC_NV(0, NULL != rb);
  return rb->current;
}

/**
 * \brief Get the capacity of the ringbuffer.
 *
 * \param rb The ringbuffer handle.
 *
 * \return The capacity of the ringbuffer, or 0 on ERROR.
 */
static inline size_t rbuffer_capacity(const struct rbuffer* const rb) {
  RCSW_FPC_NV(0, NULL != rb);
  return rb->max_elts;
}

/**
 * \brief Calculate the # of bytes that the ringbuffer will require if \ref
 * RCSW_NOALLOC_DATA is passed to manage a specified # of elements of a
 * specified size.
 *
 * \param max_elts # of desired elements the ringbuffer will hold.
 * \param elt_size size of elements in bytes.
 *
 * \return The total # of bytes the application would need to allocate.
 */
static inline size_t rbuffer_element_space(size_t max_elts, size_t elt_size) {
  return ds_elt_space_simple(max_elts, elt_size);
}

/**
 * \brief Initialize a ringbuffer.
 *
 * \param rb_in An application allocated handle for the ringbuffer. Can be NULL,
 *        depending on if \ref RCSW_NOALLOC_HANDLE is passed or not.
 *
 * \param params The initialization parameters.
 *
 * \return The initialized ringbuffer, or NULL if an error occurred.
 */
struct rbuffer *rbuffer_init(struct rbuffer *rb_in,
                             const struct rbuffer_params * params) RCSW_CHECK_RET;

/**
 * \brief Delete a ringbuffer.
 *
 * Any further reference to the ringbuffer after calling this function is
 * undefined.
 *
 * \param rb The ringbuffer handle.
 */
void rbuffer_destroy(struct rbuffer *rb);

/**
 * \brief Add an item into the ringbuffer.
 *
 * This function adds an item into the ringbuffer. If it is
 * currently full, the new entry will be added, and previous data overwritten,
 * UNLESS the ringbuffer is configured to action like a FIFO (\ref
 * RCSW_DS_RBUFFER_AS_FIFO passed during initialization).
 *
 * Sets errno to ENOSPC on error.
 *
 * \param rb The ringbuffer handle.
 *
 * \param e The element to add.
 *
 * \return \ref status_t.
 */
status_t rbuffer_add(struct rbuffer * rb, const void * e);

/**
 * \brief Remove the next item from the ringbuffer.
 *
 * The head/tail indexes are modified when the element is removed.
 *
 * \param rb The ringbuffer handle.
 * \param e To be filled with the removed element, if non-NULL.
 *
 * \return \ref status_t.
 */
status_t rbuffer_remove(struct rbuffer * rb, void * e);

/**
 * \brief Get the element in the ringbuffer at the specified index.
 *
 * The ringbuffer is not modified.
 *
 * \param rb The ringbuffer handle.
 *
 * \param idx The index. If >= max # elements for rbuffer, it is wrapped around
 *            (this IS a ringbuffer after all).
 *
 * \note if \p idx is > rbuffer_size() and < max_elts, then this function
 * _might_ return potentially uninitialized/garbage/stale data past the end of
 * the valid data currently in the rbuffer. Whether or not this is _actually_
 * bad data depends on your application.
 *
 * \return The element, or NULL if an error occurred.
 */
void* rbuffer_data_get(const struct rbuffer * rb, size_t idx);

/**
 * \brief  Get the index of an element in the ringbuffer.
 *
 * If no element compare callback was passed during initialization, then calling
 * this function is undefined.
 *
 * \param rb The ringbuffer handle.
 *
 * \param e The element to attempt to get the index of.
 *
 * \return The index of the first element in the rbuffer that matches according
 * to the compare function, or -1 on error.
 */
int rbuffer_index_query(struct rbuffer * rb, const void * e);

/**
 * \brief Retrieve the first entry from the ringbuffer.
 *
 * The ringbuffer is not modified.
 *
 * \param rb The ringbuffer handle.
 * \param e To be filled with the first element.
 *
 * \return \ref status_t.
 */
status_t rbuffer_serve_front(const struct rbuffer * rb, void * e);

/**
 * \brief Get a reference to the first entry in the ringbuffer.
 *
 * The ringbuffer is not modified.
 *
 * \param rb The ringbuffer handle.
 *
 * \return A reference to the first element, or NULL if no such element or an
 * error occurred.
 */
void* rbuffer_front(const struct rbuffer * rb);

/**
 * \brief Clear a rbuffer, but do not deallocate its memory.
 *
 * This resets the head/tail pointers, and zeros the managed memory.
 *
 * \param rb The ringbuffer handle.
 *
 * \return \ref status_t.
 */
status_t rbuffer_clear(struct rbuffer * rb);

/**
 * \brief Apply a function to all elements in the ringbuffer.
 *
 * \param rb The ringbuffer handle.
 * \param f The mapping function, which CAN modify elements.
 *
 * \return \ref status_t.
 */
status_t rbuffer_map(struct rbuffer * rb, void (*f)(void *e));

/**
 * \brief Compute a cumulative SOMETHING using elements in the ringbuffer.
 *
 * \param rb The ringbuffer handle.
 *
 * \param f The mapping function, which CAN modify elements. First argument will
 *          point to each element in the ringbuffer in sequence. Second argument
 *          is \p result.
 *
 * \param result The initial result, which is passed to the callback along with
 *               each element in the ringbuffer.
 *
 * \return \ref status_t.
 */
status_t rbuffer_inject(struct rbuffer * rb,
                        void (*f)(void *elt, void *res),
                        void *result);

/**
 * \brief Print the ringbuffer.
 *
 * If no element print callback was passed during initialization, then calling
 * this function is undefined.
 *
 * \param rb The ringbuffer handle.
 */
void rbuffer_print(struct rbuffer * rb);

END_C_DECLS
