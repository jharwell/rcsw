/**
 * \file
 *
 * \copyright 2023 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/al/types.h"
#include "rcsw/core/compilers.h"

/*******************************************************************************
 * Type Definitions
 ******************************************************************************/
/**
 * \brief The direction of iteration.
 */
enum ds_iter_type {
  ekITER_FORWARD,
  ekITER_BACKWARD,
};

/*
 * Forward declaration -- ds_iterator references ds_ops, ds_ops references
 * ds_iterator, so we need the forward decl to break the cycle.
 */
struct ds_iterator;

/**
 * \brief Vtable of traversal operations for a data structure.
 *
 * Each iterable data structure defines a static instance of this struct and
 * points \ref ds_iterator.ops at it during \ref ds_iter_init(). Adding a new
 * iterable data structure requires no changes to iter.c -- only a new static
 * \ref ds_ops instance in the new DS's own source file.
 *
 * \note \p prev may be NULL for data structures that only support forward
 *       iteration (e.g. a singly-linked list or a FIFO).
 */
struct ds_ops {
  /**
   * \brief Advance the iterator forward and return the current element's data.
   *
   * \return Pointer to element data, or NULL when the sequence is exhausted.
   */
  void* (*next)(struct ds_iterator* iter);

  /**
   * \brief Advance the iterator backward and return the current element's data.
   *
   * May be NULL if the data structure does not support backward iteration. If
   * NULL and \ref ekITER_BACKWARD is requested, \ref ds_iter_init() will return
   * NULL.
   */
  void* (*prev)(struct ds_iterator* iter);
};

/**
 * \brief A position-independent iterator over a data structure.
 *
 * The iterator is a value type: the caller allocates it (on the stack or heap)
 * and initialises it with \ref ds_iter_init(). Multiple independent iterators
 * over the same container can exist simultaneously because no state is stored
 * inside the container itself.
 *
 * Typical usage:
 *
 * \code
 * struct ds_iterator it;
 * ds_iter_init(&it, my_darray, ekITER_FORWARD,
 *              &darray_iter_ops,   // defined in darray.c
 *              NULL);              // no filter
 * void* e;
 * while ((e = ds_iter_next(&it)) != NULL) {
 *     process(e);
 * }
 * \endcode
 *
 * For a filtered iteration pass a classify callback:
 *
 * \code
 * ds_iter_init(&it, my_llist, ekITER_FORWARD, &llist_iter_ops, my_pred);
 * \endcode
 */
struct ds_iterator {
  /** Vtable set by ds_iter_init(); do not modify directly. */
  const struct ds_ops* ops;

  /** Opaque pointer to the container being iterated. */
  void* container;

  /**
   * DS-specific cursor. For indexed structures this is the current index cast
   * to a pointer; for pointer-chased structures (llist) it is the current node
   * pointer. Managed entirely by the ops callbacks.
   */
  void* cursor;

  /** Direction of iteration; read by the ops callbacks. */
  enum ds_iter_type type;

  /**
   * Optional filter. If non-NULL, \ref ds_iter_next() only returns elements
   * for which this function returns true. Elements that do not pass are skipped
   * transparently.
   */
  bool_t (*classify)(void* e);
};

/*******************************************************************************
 * Public API
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Initialise an iterator.
 *
 * The caller is responsible for the storage of \p iter (stack or heap). No
 * allocation is performed by this function.
 *
 * \param iter   The iterator to initialise. Must be non-NULL.
 * \param ds     The data structure to iterate over. Must be non-NULL.
 * \param type   Direction of iteration.
 * \param ops    Vtable of traversal operations for this data structure type.
 *               Must be non-NULL. Each DS exposes its own \c ds_ops instance;
 *               see the individual DS headers (e.g. \ref darray_iter_ops,
 *               \ref llist_iter_ops, \ref rbuffer_iter_ops).
 * \param classify Optional filter predicate. Pass NULL for unfiltered
 *               iteration.
 *
 * \return \p iter on success, or NULL if \p ops->prev is NULL but
 *         \ref ekITER_BACKWARD was requested.
 */
RCSW_API struct ds_iterator* ds_iter_init(struct ds_iterator*  iter,
                                          void*                ds,
                                          enum ds_iter_type    type,
                                          const struct ds_ops* ops,
                                          bool_t (*classify)(void* e));

/**
 * \brief Return the next element satisfying the iteration conditions.
 *
 * Calls \ref ds_ops.next or \ref ds_ops.prev depending on the direction stored
 * in \p iter. If a \p classify predicate was set, elements that do not pass are
 * skipped and the next qualifying element is returned instead.
 *
 * \param iter The iterator.
 *
 * \return Pointer to the element data, or NULL when the sequence is exhausted.
 */
RCSW_API void* ds_iter_next(struct ds_iterator* iter);

END_C_DECLS
