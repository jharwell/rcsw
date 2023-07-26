/**
 * \file ds.h
 * \brief Common definitions for all data structures.
 *
 * In this module \ref RCSW_NOALLOC_DATA declare that the space for
 * datablocks/data the data structure will manage is provided by the
 * application.
 *
 * Passing this flag causes the memory pointed to by the elements field of
 * ds_params to be used for storing the datablocks, instead of malloc()ing for
 * the space. Applies to all data structures.
 *
 * If this flag is passed to the following data structure init() functions,the
 * maximum number of elements for the data structure MUST be capped/set
 * (a -1 will cause an error):
 *
 * - \ref darray
 * - \ref llist
 * - \ref bstree
 *
 * The application is responsible for calculating/specifying an appropriate cap
 * value.
 *
 * The amount of space that must be allocated by the application for the data
 * elements can be calculated by using the appropriate sizing functions.
 *
 * In this module \ref RCSW_NOALLOC_META declares that space for the
 * nodes/metadata of the data structure will use is provided by the application.
 *
 * Passing this flag causes the memory pointed to by the nodes field of
 * ds_params to be used for storing the data struct nodes/metadata, instead of
 * malloc()ing for the space.
 *
 * Applies to:
 *
 * - \ref darray
 * - \ref llist
 * - \ref bstree
 * - \ref hashmap
 *
 * If this flag is passed to the following data structure init() functions,the
 * maximum number of elements for the data structure MUST be capped (a -1 will
 * cause an error):
 *
 * - \ref llist
 * - \ref bstree
 *
 * The application is responsible for calculating/specifying an appropriate cap
 * value.
 *
 * The amount of space that must be allocated by the application for the nodes
 * can be calculated by the XX_meta_space() function found in the header file
 * for the data structure. (i.e. \ref bstree_meta_space() for the binary search
 * tree).
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
#include "rcsw/common/flags.h"

#include "rcsw/ds/allocm.h"

/*******************************************************************************
 * Constant Definitions
 ******************************************************************************/
/**
 * Tags for data structures for use in initializing iterators.
 */
enum ds_tag {
  ekRCSW_DS_DARRAY,
  ekRCSW_DS_LLIST,
  ekRCSW_DS_RBUFFER,
};

/**
 * Keep the data structure sorted after insertions/deletions. Also implies
 * maintaining the relative ordering of elements.
 *
 * Applies to:
 *
 * - \ref darray
 * - \ref llist
 * - \ref hashmap
 */
#define RCSW_DS_SORTED (1 << (RCSW_MODFLAGS_START + 0))

/**
 * \brief Maintain the relative ordering between elements as they are inserted,
 * but not sort the elements.
 *
 * Applies to:
 *
 * - \ref darray
 */
#define RCSW_DS_ORDERED (1 << (RCSW_MODFLAGS_START + 1))

/**
 * \brief Indicate that the \ref hashmap should perform linear probing if the
 * bucket that an item hashes into is currently full.
 *
 * Results in greater hashmap utilization, but possibly longer
 * insert/remove/lookup times.
 */
#define RCSW_DS_HASHMAP_LINPROB (1 << (RCSW_MODFLAGS_START + 2))

/**
 * \brief Indicate that a \ref rbuffer should act as a FIFO (i.e., items are
 * never overrwritten/only added to ringbuffer when the ringbuffer is not
 * currently full.
 */
#define RCSW_DS_RBUFFER_AS_FIFO  (1 << (RCSW_MODFLAGS_START + 3))

/**
 * \brief Indicate that a \ref llist should NOT to allocate/deallocate a
 * datablock for a llist_node when the llist_node is allocated/deallocated. This
 * is useful when you are pointing to valid datablocks that are managed by
 * another data structure. An example use case would be an LRU list pointing to
 * datablocks within a cache.
 *
 * This flag has several side effects:
 *
 * 1. \ref llist_filter2() will not work as intended: the llist_nodes will be
 *    deallocated, but the datablocks for those llist_nodes will still be
 *    allocated, and (possibly) irretrievable.
 *
 * 2. \ref llist_filter() works the same as \ref llist_copy2() (a conditional
 *    copy)
 *
 */
#define RCSW_DS_LLIST_DB_DISOWN  (1 << (RCSW_MODFLAGS_START + 4))

/**
 * \brief Indicate that a \ref llist should NOT use compare function when
 * checking for equality/searching the linked list. Instead, the pointers for
 * llist_node->data are compared. This is useful when you have a pair of linked
 * lists as free/alloc lists which don't point to actual data, but only carve up
 * a chunk of memory.
 *
 * This flag implies \ref RCSW_DS_LLIST_DB_DISOWN.
 */
#define RCSW_DS_LLIST_DB_PTR  (1 << (RCSW_MODFLAGS_START + 5))

/**
 * \brief Indicate that a \ref bstree should function as a red-black tree and
 * rebalance itself after insertions and deletions.
 */
#define RCSW_DS_BSTREE_RB  (1 << (RCSW_MODFLAGS_START + 6))

/**
 * \brief Indicate that a \ref bstree should function as an interval tree.
 * tree.
 *
 * It has no effect unless the \ref RCSW_DS_BSTREE_RB flag is also passed. You
 * must also specify the correct element size for an interval during
 * initialization (this is not done automatically).
 */
#define RCSW_DS_BSTREE_INT  (1 << (RCSW_MODFLAGS_START + 7))

/**
 * \brief Indicate that a \ref bstree should function as an Order
 * Statistics Tree.
 *
 * It has no effect unless the \ref RCSW_DS_BSTREE_RB flag is also passed.
 */
#define RCSW_DS_BSTREE_OS  (1 << (RCSW_MODFLAGS_START + 8))

/**
 * \brief Indicate that a \ref binheap should function as a min heap. If you do
 * not pass this flag, \ref binheap will function as a max heap.
 */
#define RCSW_DS_BINHEAP_MIN  (1 << (RCSW_MODFLAGS_START + 9))

/**
 * \brief If you want to define additional flags for derived data structures,
 * start with this one to ensure no conflicts.
 */
#define RCSW_DS_EXTFLAGS_START 10

/*******************************************************************************
 * Structure Definitions
 ******************************************************************************/
#if !defined(DOXYGEN_DOCUMENTATION_BUILD)
/**
 * \brief The common set of parameters used by all data structures.
 */
#define RCSW_DECLARE_DS_PARAMS_COMMON  \
  int (*cmpe)(const void *const e1, const void *const e2); \
  int (*cmpkey)(const void *const e1, const void *const e2); \
  void (*printe)(const void *e); \
  uint8_t *meta; \
  uint8_t *elements; \
  size_t elt_size; \
  int max_elts; \
  uint32_t flags;
#endif

/*******************************************************************************
 * Inline Functions
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Calculate how large the chunk of memory for the metadata for a data
 * structure needs to be, given a max # of elements.
 *
 * Used in conjunction with \ref RCSW_NOALLOC_DATA and/or \ref
 * RCSW_NOALLOC_META for data structures requiring metadata.
 *
 * You really shouldn't use this function.
 *
 * \param max_elts Max # elements the structure will manage.
 *
 * \return Total # of bytes required.
 */
static inline size_t ds_meta_space(size_t max_elts) {
  return sizeof(struct allocm_entry) * max_elts;
}

/**
 * \brief Calculate how large the chunk of memory for some "element" that a data
 * structure will manage needs to be, given a max # of elements and element
 * size. "Element" can refer to the nodes needed by some data structures, or the
 * actual elements the data structure will manage.
 *
 * For data structures that do NOT require metadata (FIFOs, ringbuffers, etc.).
 *
 * You really shouldn't use this function--use the specific calculation
 * functions for each data structure found in their respective header
 * files. Used in conjunction with \ref RCSW_NOALLOC_DATA.
 *
 * \param max_elts Max # elements the structure will manage
 * \param elt_size Size of each element in bytes
 *
 * \return Total # of bytes required
 */
static inline size_t ds_elt_space_simple(size_t max_elts, size_t elt_size) {
  return max_elts * elt_size;
}

/**
 * \brief Calculate how large the chunk of memory for the elements that a data
 * structure will manage needs to be, given a max # of elements and element
 * size.
 *
 * For data structures that DO require metadata (trees, linked lists, etc.).
 *
 * You really shouldn't use this function--use the specific calculation
 * functions for each data structure found in their respective header
 * files. Used in conjunction with \ref RCSW_NOALLOC_DATA.
 *
 * \param max_elts Max # elements the structure will manage
 * \param elt_size Size of each element in bytes
 *
 * \return Total # of bytes required
 */
static inline size_t ds_elt_space_with_meta(size_t max_elts, size_t elt_size) {
  return ds_meta_space(max_elts) + ds_elt_space_simple(max_elts, elt_size);
}

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
/**
 * \brief Utility function to copy elt2 into elt1, overwriting.
 *
 * If the element is larger than a double, memcpy() is used. If it is < than the
 * size of a double, pointers are used.
 *
 * \param elt1 Destination.
 * \param elt2 Source.
 * \param elt_size Size of elements in bytes.
 *
 * \return \ref status_t
 */
status_t ds_elt_copy(void *elt1, const void *elt2, size_t elt_size);

/**
 * \brief Utility function to clear an element.
 *
 * If the element is larger than a double, memcpy() is used. If it is < than the
 * size of a double, pointers are used.
 *
 * \param elt Element to clear.
 * \param elt_size Size of element in bytes.
 *
 * \return \ref status_t
 */
status_t ds_elt_clear(void *elt, size_t elt_size);

/**
 * \brief Utility function to swap two elements.
 *
 * If the element is larger than double, a for() loop is used. Otherwise
 * pointers are used.
 *
 * \param elt1 Element #1.
 * \param elt2 Element #2.
 * \param elt_size Size of elements in bytes.
 *
 * \return \ref status_t
 */
status_t ds_elt_swap(void *elt1, void *elt2, size_t elt_size);

END_C_DECLS
