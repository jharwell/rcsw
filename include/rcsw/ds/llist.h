/**
 * \file llist.h
 * \ingroup ds
 * \brief Implementation of doubly-linked list.
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
#include "rcsw/algorithm/sort.h"

/*******************************************************************************
 * Structure Definitions
 ******************************************************************************/
/**
 * \brief Parameters for \ref llist.
 */
struct llist_params {
  /**
   * For comparing elements. If NULL, then \ref llist querying, sorting,
   * etc. are disabled.
   */
  int (*cmpe)(const void *const e1, const void *const e2);

  /**
   * For printing an element. Can be NULL. If NULL, \ref llist_print() cannot be
   * called.
   */
  void (*printe)(const void *e);

  /**
   * Pointer to application-allocated space for storing the nodes managed by the
   * \ref llist. Ignored unless \ref RCSW_NOALLOC_META is passed.
   */
  dptr_t *meta;

  /**
   * Pointer to application-allocated space for storing the datablocks managed
   * by the \ref llist. Ignored unless \ref RCSW_NOALLOC_DATA is passed.
   */
  dptr_t *elements;

  /**
   * Size of elements in bytes.
   */
  size_t elt_size;

  /**
   * Maximum Number of elements allowed. -1 = no upper limit.
   */
  int max_elts;

  /**
   * Configuration flags. See \ref llist.flags for valid flags.
   */
  uint32_t flags;
};

/**
 * \brief Linked list node for \ref llist.
 *
 * Must be packed and aligned to the same size as \ref dptr_t so that casts from
 * \ref llist_node.data are same on all targets.
 */
struct RCSW_ATTR(packed, aligned (sizeof(dptr_t))) llist_node {
  /**
   * Next node in the list.
   */
  struct llist_node *next;

  /**
   * Previous node in the list.
   */
  struct llist_node *prev;

  /**
   * Actual data associated with this node
   */
  dptr_t *data;
};

/**
 * \brief Container for all memory (space) used by a \ref llist.
 */
struct llist_space_mgmt {
  /**
   * Space for the data elements. Used if \ref RCSW_NOALLOC_DATA passed in \ref
   * llist_params.flags.
   */
  dptr_t*             datablocks;

  /**
   * Space for the allocation map for datablocks. Used if \ref RCSW_NOALLOC_DATA
   * passed in \ref llist_params.flags.
   */
  struct allocm_entry* db_map;

  /**
   * Space for \ref llist_node objects. Used if \ref RCSW_NOALLOC_META
   * passed in \ref llist_params.flags.
   */
  struct llist_node*   nodes;

  /**
   * Space for the allocation map for \ref llist_node objects. Used if \ref
   * RCSW_NOALLOC_META passed in \ref llist_params.flags.
   */
  struct allocm_entry* node_map;
};

/**
 * \brief Linked list data structure.
 */
struct llist {
  /** For comparing two elements. Can be NULL. */
  int (*cmpe)(const void *const e1, const void *const e2);
  void (*printe)(const void *e);  /// For printing an element. Can be NULL.
  struct llist_space_mgmt space;
  size_t current;     /// number of nodes currently in the list.
  int max_elts;       /// Maximum # of allowed elements. -1 = no upper limit.
  size_t elt_size;     /// Size in bytes of an element.
  bool_t sorted;      /// If true, list is currently sorted.
  struct llist_node *first;  /// First node in the list (for easy prepending)
  struct llist_node *last;   /// Last node in the list (for easy appending)
  struct ds_iterator iter;   /// iterator

  /**
   *
   * Configuration flags. Valid flags are:
   *
   * - \ref RCSW_ZALLOC
   * - \ref RCSW_NOALLOC_HANDLE
   * - \ref RCSW_NOALLOC_DATA
   * - \ref RCSW_DS_LLIST_DB_DISOWN
   * - \ref RCSW_DS_LLIST_DB_PTR
   * - \ref RCSW_DS_SORTED
   *
   * All other flags are ignored.
   */
  uint32_t flags;
};


/*******************************************************************************
 * Macros
 ******************************************************************************/
/**
 * \brief Iterate over a \ref llist.
 *
 * To go forward, pass 'next' as the \p DIR field; to iterate through the list
 * backward, pass 'prev' in the \p DIR field.
 *
 * You cannot use this macro directly if you are manipulating the next/prev
 * fields. You cannot call this macro on a NULL/unitialized list.
 *
 * \param LIST The linked list
 *
 * \param DIR The name of the field  used for traversal (next or prev), which
 *            sets the direction of traversal.
 *
 * \param VAR The name of the \ref llist_node* local variable to each node will be
 *            assigned to when iterating over the list.
 */
#define LLIST_FOREACH(LIST, DIR, VAR)                                   \
    struct llist_node *_node = NULL;                                    \
    struct llist_node *(VAR)   = NULL;                                  \
    for ((VAR) = _node = (LIST)->first; _node != NULL; (VAR) = _node = _node->DIR)

/**
 * \brief Same as \ref LLIST_FOREACH(). but the \p START parameter allows you to
 * start at an arbitrary location in the list. Pass list->first as the \p START
 * parameter to make it the same as \ref LLIST_FOREACH().
 *
 * You cannot use this macro directly if you are manipulating the next/prev
 * fields. You cannot call this macro on a NULL/unitialized list.
 *
 * \param LIST The \ref llist.
 *
 * \param START The starting locating with the linked list (must point to a node in
 *              the list).
 *
 * \param DIR The name of the field  used for traversal (next or prev) which
 *            sets the direction of traversal.
 *
 * \param VAR The name of the local variable you want to use when iterating over
 *            the list.
 */
#define LLIST_ITER(LIST, START, DIR, VAR)                       \
    struct llist_node *_node = NULL;                              \
    struct llist_node *(VAR)     = NULL;                                \
    for ((VAR) = _node = (START); _node != NULL; (VAR) = _node = _node->DIR)

/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Determine if the \ref llist is currently full.
 *
 * \param list The linked list handle.
 *
 * \return \ref bool_t
 */
static inline bool_t llist_isfull(const struct llist* const list) {
  RCSW_FPC_NV(false, NULL != list);
  return (list->current == (size_t)list->max_elts);
}

/**
 * \brief Determine if the \ref llist is currently empty.
 *
 * \param list The linked list handle.
 *
 * \return \ref bool_t
 */
static inline bool_t llist_isempty(const struct llist* const list) {
  RCSW_FPC_NV(false, NULL != list);
  return (list->current == 0);
}

/**
 * \brief Determine # elements currently in the \ref llist.
 *
 * \param list The linked list handle.
 *
 * \return # elements in linked list, or 0 on ERROR.
 */

static inline size_t llist_size(const struct llist* const list) {
  RCSW_FPC_NV(0, NULL != list);
  return list->current;
}

/**
 * \brief Calculate the # of bytes that the \ref llist will require if
 * \ref RCSW_NOALLOC_DATA is passed to manage a specified # of elements of a
 * specified size.
 *
 * \param max_elts # of desired elements the linked list will hold.
 * \param elt_size size of elements in bytes.
 *
 * \return The total # of bytes the application would need to allocate.
 */
static inline size_t llist_element_space(size_t max_elts, size_t elt_size) {
  return ds_elt_space_with_meta(max_elts, elt_size);
}

/**
 * \brief Calculate the space needed for the nodes in the \ref llist, given a
 * max # of elements.
 *
 * Used in conjunction with \ref RCSW_NOALLOC_META.
 *
 * \param max_elts # of desired elements the linked list will hold.
 *
 * \return The # of bytes required.
 */
static inline size_t llist_meta_space(size_t max_elts) {
  return ds_elt_space_with_meta(max_elts, sizeof(struct llist_node));
}

/**
 * \brief Initialize a llist.
 *
 * \param list_in The handle to be filled. Must be non-NULL if \ref
 *                RCSW_NOALLOC_HANDLE passed in \ref llist_params.flags.
 *
 * \param params The initialization parameters.
 *
 * \return The initialized list, or NULL if an error occured.
 */
RCSW_API struct llist *llist_init(struct llist *list_in,
                         const struct llist_params *params) RCSW_WUR;

/**
 * \brief Destroy a \ref llist
 *
 * The entire list is iterated through once. Any further use of the pointer to
 * this llist is undefined. This function is idempotent.
 *
 * \param list The list to destroy.
 */
RCSW_API void llist_destroy(struct llist *list);

/**
 * \brief Clear a \ref llist.
 *
 * This routine clears an llist. Every node with allocated memory for its data
 * field (non-NULL) will have that memory freed if it is in the DS domain, along
 * with the memory for the node.
 *
 * \param list The linked list handle.
 *
 * \return \ref status_t.
 */
RCSW_API status_t llist_clear(struct llist *list);

/**
 * \brief Remove an item from a \ref llist.
 *
 * Memory for the node and its data is deallocated.
 *
 * \param list The linked list handle.
 * \param e To be filled with the data from the removed item if non-NULL.
 *
 * \return \ref status_t.
 */
RCSW_API status_t llist_remove(struct llist *list, const void *e);

/**
 * \brief Delete a node from a \ref llist.
 *
 * \param list The linked list handle.
 * \param victim the linked list node to delete from the list.
 * \param e To be filled with the data from the victim node if non-NULL.
 *
 * \return \ref status_t.
 */
RCSW_API status_t llist_delete(struct llist * list, struct llist_node * victim,
                      void *e);
/**
 * \brief Append an item to a \ref llist.
 *
 * \param list The linked list handle.
 * \param data The data to insert into the list.
 *
 * \return \ref status_t.
 */
RCSW_API status_t llist_append(struct llist *list, void *data);

/**
 * \brief Prepend an item to the llist.
 *
 * This routine prepends an item to the start of the llist. Can be used
 * as a "push" function.
 *
 * \param list The linked list handle.
 * \param data The data to insert into the list.
 *
 * \return \ref status_t.
 */
RCSW_API status_t llist_prepend(struct llist *list, void *data);

/**
 * llist_print() - Print the llist
 *
 * \param list The linked list handle.
 */
RCSW_API void llist_print(struct llist *list);

/**
 * \brief Search a \ref llist for specific data
 *
 * This routine searches the linked list for a llist_node whose data matches
 * that of the provided argument according to the list->cmpe function.
 *
 * \param list The linked list handle.
 * \param e The data to search for.
 *
 * \return The matching data, or NULL if an error occured or no match was
 * found.
 */
RCSW_API void* llist_data_query(struct llist *list, const void *e);

/**
 * \brief Search a \ref llist for specific data
 *
 * This routine searches the linked list for a llist_node whose data matches
 * that of the provided argument according to the list->cmpe function.
 *
 * \param list The linked list handle
 * \param e The data to search for
 *
 * \return The node for which the data matched, or NULL if an error occured or
 * no match was found.
 */
RCSW_API struct llist_node* llist_node_query(struct llist *list,
                                    const void *e);
/**
 * \brief Sort a \ref llist.
 *
 * Sorts a linked list using the sort type specified. The recursive sort option
 * is more memory intensive that the iterative one, but can run faster under
 * certain conditions. The iterative sort should be used under conditions where
 * memory is at a premium.
 *
 * \param list The linked list handle.
 * \param type \ref exec_type.
 *
 * \return \ref status_t.
 */
RCSW_API status_t llist_sort(struct llist *list, enum exec_type type);

/**
 * \brief Create a copy of a \ref llist.
 *
 * The flags,elements, and nodes fields of cparams are used to determine how
 * memory should be managed for the new list;
 *
 * \param list The linked list handle.
 *
 * \param flags Initialization flags for the new list.
 *
 * \param elements Space for elements in the new list. Must be non-NULL if \ref
 *                 RCSW_NOALLOC_DATA is passed in \p flags; can be NULL
 *                 otherwise.
 *
 * \param nodes Space for nodes in the new list. Must be non-NULL if \ref
 *              RCSW_NOALLOC_META is passed in \p flags; can be NULL otherwise.
 *
 * \return The new list, or NULL if an error occurred..
 */
RCSW_API struct llist* llist_copy(struct llist *list,
                         uint32_t flags,
                         void* elements,
                         void* nodes);

/**
 * \brief Create a copy of part of a \ref llist (conditional copy).
 *
 * This routine iterates through the list and finds all the items that satisfy
 * the predicate, and duplicates them, creating a new list contain all the nodes
 * in the orginal list that satisfied the predicate. If no elements are found
 * that fulfill the predicate, an empty list is returned.
 *
 * \param list The linked list handle.
 *
 * \param pred The predicate for determining element membership in the new list
 *
 * \param flags Initialization flags for the new list.
 *
 * \param elements Space for elements in the new list. Must be non-NULL if \ref
 *                 RCSW_NOALLOC_DATA is passed in \p flags; can be NULL
 *                 otherwise.
 *
 * \param nodes Space for nodes in the new list. Must be non-NULL if \ref
 *              RCSW_NOALLOC_META is passed in \p flags; can be NULL otherwise.

 * \return The new list, or NULL if an error occurred.
 */
RCSW_API struct llist *llist_copy2(struct llist *list,
                          bool_t (*pred)(const void *e),
                          uint32_t flags,
                          void* elements,
                          void* nodes);

/**
 * \brief  Filter out elements from one \ref llist into another.
 *
 * This routine iterates through the llist and finds all the items that satisfy
 * the predicate, and moves them into a new list (they are removed from the
 * original list). If no elements are found that fulfill the predicate, an empty
 * list is returned.
 *
 * \param list The linked list handle
 *
 * \param flags Initialization flags for the new list.
 *
 * \param elements Space for elements in the new list. Must be non-NULL if \ref
 *                 RCSW_NOALLOC_DATA is passed in \p flags; can be NULL
 *                 otherwise.
 *
 * \param nodes Space for nodes in the new list. Must be non-NULL if \ref
 *              RCSW_NOALLOC_META is passed in \p flags; can be NULL otherwise.
 *
 * \param pred The predicate for determining element membership in the new list
 *
 * \return The new list, or NULL if an error occurred.
 */
RCSW_API struct llist *llist_filter(struct llist *list,
                           bool_t (*pred)(const void *const e),
                           uint32_t flags,
                           void* elements,
                           void* nodes);

/**
 * \brief - Filter out items that satisfy a predicate from a \ref llist.
 *
 * This routine iterates through the llist and finds all the items that satisfy
 * the predicate, and removes them from the list. If no elements are found that
 * fulfill the predicate, no modifications are made to the list.  Memory for
 * both the matching llist_nodes and the data they contain are deallocated.
 *
 * \param list The linked list handle.
 * \param pred The predicate for determining element membership in the new
 *             list.
 *
 * \return \ref status_t.
 */
RCSW_API status_t llist_filter2(struct llist *list,
                                bool_t (*pred)(const void * e));

/**
 * \brief Splice two \ref llist objects together.
 *
 * This function inserts the second list at the position of the specified node
 * in list1. To append list2 to list1, pass list1->last as the node. To prepend
 * list2 to list1, pass list1->first as the node. Any further use of the list2
 * pointer after this function is called is undefined.
 *
 * This works as expected except if list1 only has a single node, because its
 * first and last fields both point to the same node in that case. For this,
 * passing list->last (appending)produces the same result as passing list->first
 * (prepending).
 *
 * \param list1 The linked list handle for the splicer.
 * \param list2 The linked list handle for the splicee.
 * \param node The node in list1 to insert the 2nd list at.
 *
 * \return \ref status_t
 */
RCSW_API status_t llist_splice(struct llist *list1,
                      struct llist *list2,
                      const struct llist_node * node);

/**
 * \brief Apply a function to all elements in the \ref llist.
 *
 * \param list The linked list handle.
 * \param f The function to apply, which CAN modify elements.
 *
 * \return \ref status_t.
 */
RCSW_API status_t llist_map(struct llist *list, void (*f)(void *e));

/**
 * \brief Compute a cumulative SOMETHING using all elements in the \ref llist.
 *
 * \param list The linked list handle.
 *
 * \param f The mapping function, which CAN modify elements.
 *
 * \param result The initial result, which is passed to the callback along with
 *               each element in the linked list.
 *
 * \return \ref status_t
 */
RCSW_API status_t llist_inject(struct llist * list,
                               void (*f)(void *e, void *res),
                               void *result);

/**
 * \brief Get # of bytes occupied on the heap by a \ref llist.
 *
 * \param list The linked list handle.
 *
 * \return # of bytes occupied.
 */
RCSW_API size_t llist_heap_footprint(const struct llist * list);

END_C_DECLS
