/**
 * \file iter.h
 *
 * \copyright 2023 John Harwell, All rights reserved.
 *
 * SPDX-License Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/ds.h"

/*******************************************************************************
 * Type Definitions
 ******************************************************************************/
/**
 * \brief The type of iteration to perform.
 */
enum ds_iter_type {
  ekRCSW_DS_ITER_FORWARD,
  ekRCSW_DS_ITER_BACKWARD,
};

/*******************************************************************************
 * Structure Definitions
 ******************************************************************************/
/**
 * \brief Data structure iterator.
 *
 * Used to provide a uniform interface for iterating through data
 * structures. Not implemented on all data structures in library.
 */
struct ds_iterator {
  union {
    struct darray *arr;
    struct rbuffer *rb;
    struct llist *list;
    struct llist_node *curr;
  };
  enum ds_tag       tag;
  enum ds_iter_type type;
  int               index;

  /**
   * Classification function. Used to determine which elements are returned
   * during iteration for \ref ekRCSW_DS_ITER_CUSTOM;
   */
  bool_t (*classify)(void *e);
};

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief - Get the next element that matches the iteration conditions
 *
 * \param iter The iterator
 *
 * \return The next element, or NULL if no more
 */
void *ds_iter_next(struct ds_iterator *iter);


/**
 * \brief Initialize an iterator
 *
 * Initialize an iterator to iterate over all of the elements of a
 * data structure. Implemented for:
 *
 * - \ref darray
 * - \ref rbuffer
 * - \ref llist
 *
 * \param ds The data structure to iterate over.
 *
 * \param tag What type of data structure to initialize for.
 *
 * \param type What type of iteration to perform.
 *
 * \return The initialized iterator, or NULL if an ERROR occurred
 */
struct ds_iterator * ds_iter_init(void *ds,
                                  enum ds_tag tag,
                                  enum ds_iter_type type);

/**
 * \brief Initialize an iterator for filtering
 *
 * Initialize an iterator to iterate over SOME of the elements of a data
 * structure. Implemented for:
 *
 * - \ref darray
 * - \ref rbuffer
 * - \ref llist
 *
 * \param ds The data structure to iterate over.
 *
 * \param tag What type of data structure to initialize for.
 *
 * \param f The function to determine if an element will be returned by the
 *          iterator or not. Elements are checked in forward iteration order.
 *
 * \return The initialized iterator, or NULL if an ERROR occurred
 */
struct ds_iterator * ds_filter_init(void *ds,
                                    enum ds_tag tag,
                                    bool_t (*f)(void *e));
END_C_DECLS
