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
#include "rcsw/common/fpc.h"
#include "rcsw/algorithm/sort.h"

/*******************************************************************************
 * Structure Definitions
 ******************************************************************************/
/**
 * \brief Linked list node structure (note that this is for a doubly linked
 * list).
 */
struct llist_node {
  struct llist_node *next;  /// next node in the list
  struct llist_node *prev;  /// previous node in the list
  uint8_t *data;            /// the actual data for this node
};

struct llist_space_mgmt {
  uint8_t*             datablocks;  /// Ptr to space for the data elements.
  struct allocm_entry* db_map;

  struct llist_node*   nodes;  /// Ptr to space for the nodes in the list.
  struct allocm_entry* node_map;     /// Ptr to space for the nodes in the list.
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
  uint32_t flags;     /// Runtime configuration flags.
  bool_t sorted;      /// If TRUE, list is currently sorted.
  struct llist_node *first;  /// First node in the list (for easy prepending)
  struct llist_node *last;   /// Last node in the list (for easy appending)
  struct ds_iterator iter;   /// iterator
};


/*******************************************************************************
 * Macros
 ******************************************************************************/
/**
 * \brief Iterate over a \ref llist.
 *
 * To go forward, pass 'next' as the 'N' field; to iterate through the list
 * backward, pass 'prev' in the 'N' field.
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
    struct llist_node *VAR   = NULL;                                    \
    for (VAR = _node = (LIST)->first; _node != NULL; VAR = _node = _node->DIR)

/**
 * \brief Same as \ref LLIST_FOREACH(). but the 'S' parameter allows you to
 * start at an arbitrary location in the list. Pass list->first as the 'S'
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
 * Inline Functions
 ******************************************************************************/
/**
 * \brief Determine if the \ref llist is currently full.
 *
 * \param list The linked list handle.
 *
 * \return \ref bool_t
 */
static inline bool_t llist_isfull(const struct llist* const list) {
  RCSW_FPC_NV(FALSE, NULL != list);
  return (bool_t)(list->current == (size_t)list->max_elts);
}

/**
 * \brief Determine if the \ref llist is currently empty.
 *
 * \param list The linked list handle.
 *
 * \return \ref bool_t
 */
static inline bool_t llist_isempty(const struct llist* const list) {
  RCSW_FPC_NV(FALSE, NULL != list);
  return (bool_t)(list->current == 0);
}

/**
 * \brief Determine # elements currently in the \ref llist.
 *
 * \param list The linked list handle.
 *
 * \return # elements in linked list, or 0 on ERROR.
 */

static inline size_t llist_n_elts(const struct llist* const list) {
  RCSW_FPC_NV(0, NULL != list);
  return list->current;
}

/**
 * \brief Calculate the # of bytes that the \ref llist will require if
 * \ref RCSW_DS_NOALLOC_DATA is passed to manage a specified # of elements of a
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
 * Used in conjunction with \ref RCSW_DS_NOALLOC_NODES.
 *
 * \param max_elts # of desired elements the linked list will hold.
 *
 * \return The # of bytes required.
 */
static inline size_t llist_node_space(size_t max_elts) {
  return ds_elt_space_with_meta(max_elts, sizeof(struct llist_node));
}

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Initialize a llist.
 *
 * \param list_in The llist handle to be filled (can be NULL if
 *                \ref RCSW_DS_NOALLOC_HANDLE not passed).
 *
 * \param params The initialization parameters.
 *
 * \return The initialized list, or NULL if an error occured.
 */
struct llist *llist_init(
    struct llist *list_in, const struct ds_params *params) RCSW_CHECK_RET;

/**
 * \brief Destroy a \ref llist
 *
 * The entire list is iterated through once. Any further use of the pointer to
 * this llist is undefined. This function is idempotent.
 *
 * \param list The list to destroy.
 */
void llist_destroy(struct llist *list);

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
status_t llist_clear(struct llist *list);

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
status_t llist_remove(struct llist *list, const void *e);

/**
 * \brief Delete a node from a \ref llist.
 *
 * \param list The linked list handle.
 * \param victim the linked list node to delete from the list.
 * \param e To be filled with the data from the victim node if non-NULL.
 *
 * \return \ref status_t.
 */
status_t llist_delete(struct llist * list, struct llist_node * victim,
                      void *e);
/**
 * \brief Append an item to a \ref llist.
 *
 * \param list The linked list handle.
 * \param data The data to insert into the list.
 *
 * \return \ref status_t.
 */
status_t llist_append(struct llist *list, void *data);

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
status_t llist_prepend(struct llist *list, void *data);

/**
 * llist_print() - Print the llist
 *
 * \param list The linked list handle.
 */
void llist_print(struct llist *list);

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
void* llist_data_query(struct llist *list, const void *e);

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
struct llist_node* llist_node_query(struct llist *list,
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
 * \param type The sort method.
 *
 * \return \ref status_t.
 */
status_t llist_sort(struct llist *list, enum alg_sort_type type);

/**
 * \brief Create a copy of a \ref llist.
 *
 * The flags,elements, and nodes fields of cparams are used to determine how
 * memory should be managed for the new list;
 *
 * \param list The linked list handle
 *
 * \param cparams Initialization parameters for the new list. Flags, elements, and
 *        node fields are considered--all other fields inherited from parent list. If
 *        NULL, then I assume that the calling application wants all memory in
 *        the DS domain.
 *
 * \return The new list, or NULL if an error occurred..
 */
struct llist* llist_copy(struct llist *list,
                         const struct ds_params *cparams);

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
 * \param cparams Initialization parameters for the new list. Flags, elements,
 *                and node fields are considered--all other fields inherited
 *                from parent list. If NULL, then I assume that the calling
 *                application wants all memory in the DS domain.
 *
 * \return The new list, or NULL if an error occurred.
 */
struct llist *llist_copy2(struct llist *list,
                          bool_t (*pred)(const void *e),
                          const struct ds_params *cparams);

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
 * \param pred The predicate for determining element membership in the new list
 *
 * \param fparams Initialization parameters for the new list. Flags, elements,
 *                and node fields are considered--all other fields inherited
 *                from parent list. If NULL, then I assume that the calling
 *                application wants all memory in the DS domain.
 *
 * \return The new list, or NULL if an error occurred.
 */
struct llist *llist_filter(struct llist *list,
                           bool_t (*pred)(const void *const e),
                           const struct ds_params * fparams);

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
status_t llist_filter2(struct llist *list, bool_t (*pred)(const void * e));

/**
 * \brief Splice two \ref llists together.
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
status_t llist_splice(struct llist *list1,
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
status_t llist_map(struct llist *list, void (*f)(void *e));

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
status_t llist_inject(struct llist * list, void (*f)(void *e, void *res),
                      void *result);

/**
 * \brief Get # of bytes occupied on the heap by a \ref llist.
 *
 * \param list The linked list handle.
 *
 * \return # of bytes occupied.
 */
size_t llist_heap_footprint(const struct llist * list);

END_C_DECLS
