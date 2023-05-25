/**
 * \file ds.h
 * \brief Common definitions for all data structures.
 *
 * Initialization parameters, common constant/type definitions, and run-time
 * configuration flags.
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/common/common.h"

/*******************************************************************************
 * Constant Definitions
 ******************************************************************************/
/**
 * Tags for initialization parameter union. Not all data structures under DS
 * use the same initialization structure.
 */
enum ds_tag {
  DS_DARRAY,
  DS_LLIST,
  DS_HASHMAP,
  DS_BSTREE,
  DS_RBUFFER,
  DS_BIN_HEAP,
  DS_FIFO,
  DS_STATIC_MATRIX,
  DS_DYNAMIC_MATRIX,
  DS_ADJ_MATRIX
};

/**
 * Keep the data structure sorted after insertions/deletions. Also implies
 * maintaining the relative ordering of elements.
 *
 * Applies to darray, llist, hashmap
 */
#define DS_KEEP_SORTED 0x1

/**
 * \brief Maintain the relative ordering between elements as they are inserted,
 * but not sort the elements.
 *
 * Applies to darray.
 */
#define DS_MAINTAIN_ORDER 0x2

/**
 * \brief Declare that space for the data structure handle is
 * provided by the application.
 *
 * The _in parameter is ignored if you do not pass this flag (even if it is
 * non-NULL). Applies to all data structures.
 */
#define DS_APP_DOMAIN_HANDLE 0x4

/**
 * \brief Declare that the space for datablocks/data the data structure will
 * manage is provided by the application.
 *
 * Passing this flag causes the memory pointed to by the elements field of
 * ds_params to be used for storing the datablocks, instead of malloc()ing for
 * the space. Applies to all data structures.
 *
 * If this flag is passed to the following data structure init() functions,the
 * maximum number of elements for the data structure MUST be capped/set
 * (a -1 will cause an error):
 *
 * darray, llist, bstree
 *
 * The application is responsible for calculating/specifying an appropriate cap
 * value.
 *
 * The amount of space that must be allocated by the application for the data
 * elements can be calculated by using the appropriate sizing functions. Some
 * data structures use the generic ds_calc_element_space() function, and some
 * data structures require a more specialized macro which can be found in their
 * respective header file.
 */
#define DS_APP_DOMAIN_DATA 0x8

/**
 * \brief Declare that space for the nodes/metadata of the data
 * structure will use is provided by the application.
 *
 * Passing this flag causes the memory pointed to by the nodes field of
 * ds_params to be used for storing the data struct nodes/metadata, instead of
 * malloc()ing for the space.
 *
 * Applies to:
 * darray, llist, bstree, hashmap
 *
 * If this flag is passed to the following data structure init() functions,the
 * maximum number of elements for the data structure MUST be capped (a -1 will
 * cause an error):
 *
 * darray, llist, bstree
 *
 * The application is responsible for calculating/specifying an appropriate cap
 * value.
 *
 * The amount of space that must be allocated by the application for the nodes
 * can be calculated by the XX_node_space() function found in the header file
 * for the data structure. (i.e. \ref bstree_node_space() for the binary search
 * tree).
 */
#define DS_APP_DOMAIN_NODES 0x10

/**
 * \brief Indicate that the hashmap should perform linear probing if the bucket that an item
 * hashes into is currently full.
 *
 * Results in greater hashmap utilization, but possibly longer
 * insert/remove/lookup times.
 */
#define DS_HASHMAP_LINEAR_PROBING 0x20

/**
 * \brief Indicate that a ringbuffer should act as a FIFO (i.e., items are never
 * overrwritten/only added to ringbuffer when the ringbuffer is not currently
 * full.
 */
#define DS_RBUFFER_AS_FIFO 0x40

/**
 * \brief Indicate that a linked list should NOT to allocate/deallocate a
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
 * 2. \ref llist_filter() works the same as \ref llist_copy2() (a conditional copy)
 *
 */
#define DS_LLIST_NO_DB 0x80

/**
 * \brief Indicate that a linked list should NOT use compare function when
 * checking for equality/searching the linked list. Instead, the pointers for
 * llist_node->data are compared. This is useful when you have a pair of linked
 * lists as free/alloc lists which don't point to actual data, but only carve up
 * a chunk of memory.
 *
 * This flag implies \ref DS_LLIST_NO_DB.
 */
#define DS_LLIST_PTR_CMP 0x100

/**
 * \brief Indicate that a binary search tree should function as a red-black tree
 * and rebalance itself after insertions and deletions.
 */
#define DS_BSTREE_REDBLACK 0x200

/**
 * \brief Indicate that a binary search tree should function as an interval tree.
 * tree.
 *
 * It has no effect unless the \ref DS_BSTREE_REDBLACK flag is also passed. You
 * must also specify the correct element size for an interval during
 * initialization (this is not done automatically).
 */
#define DS_BSTREE_INTERVAL 0x400

/**
 * \brief Indicate that a binary search tree should function as an Order Statistics
 * Tree.
 *
 * It has no effect unless the \ref DS_BSTREE_REDBLACK flag is also passed.
 */
#define DS_BSTREE_OS 0x800

/**
 * \brief Indicate that a heap should function as a min heap. If you do not pass
 * this flag, all heaps will function as max heaps.
 */
#define DS_RCSW_MIN_HEAP 0x1000

/**
 * \brief If you want to define additional flags for derived data structures,
 * start with this one to ensure no conflicts.
 */
#define DS_EXT_FLAGS 0x2000

/*******************************************************************************
 * Structure Definitions
 ******************************************************************************/
/**
 * \brief Dynamic array (darray) initialization parameters.
 */
struct da_params {
  /**
   * Initial size of the array (must be < max_elts). Ignored if \ref
   * DS_APP_DOMAIN_DATA is passed.
   */
  size_t init_size;
};

/**
 * \brief Hashmap initialization parameters.
 */
struct hm_params {
  /** Used by the hashmap darrays find elements. Must be non-NULL. */
  int (*key_cmp)(const void *a, const void *b);

  /** Hashing function to use. Must be non-NULL. */
  uint32_t (*hash)(const void *const key, size_t len);

  size_t bsize;  /// Initial size of hash buckets (buckets can grow)
  size_t n_buckets;  /// Fixed number of buckets for hashmap

  /**
   * # of inserts before automatically sorting. -1 = do not automatically
   * sort.
   */
  int sort_thresh;

  size_t keysize;  /// Size in bytes for hashnode keys
};

/**
 * \brief Binary heap initialization parameters.
 */
struct bhp_params {
  size_t init_size;  /// Initial size of heap.
};

/**
 * \brief Static matrix initialization parameters.
 *
 */
struct static_matrix_params {
  size_t n_rows;  /// # rows in matrix.
  size_t n_cols;  /// # columns in matrix.
};

/**
 * \brief Dynamic matrix initialization parameters.
 *
 */
struct dynamic_matrix_params {
  size_t n_rows;  /// # rows in matrix.
  size_t n_cols;  /// # columns in matrix.
  uint8_t* rows;  /// Ptr to space for vector-of-row-vectors.
};


/**
 * \brief Adjacency matrix initialization parameters.
 *
 */
struct adj_matrix_params {
  /**
   * Initial # of vertices for graph, for space allocation.
   */
  size_t n_vertices;  /// Max # of vertices graph will hold.
  bool_t is_directed;  /// Is the graph directed or undirected?
  /**
   * Are the graph edges weighted? If a graph is undirected it cannot be
   * weighted.
   */
  bool_t is_weighted;
};

/**
 * \brief Adjacency list initialization parameters.
 *
 */
struct adj_list_params {
  /**
   * Initial # of vertices for graph, for space allocation.
   */
  size_t init_vertices;
  size_t max_vertices;  /// Max # of vertices graph will hold.
  int max_vertex_edges;   /// Max # of edges a vertex can have. -1 = no limit.
  bool_t is_directed;  /// Is the graph directed or undirected?
  bool_t is_weighted;  /// Are the graph edges weighted?
};

/**
 * A single initialization parameters structure for all general purpose data
 * structures. Some data structures require additional parameters which are
 * captured in the union.
 */
struct ds_params {
  union {
    struct da_params da;
    struct hm_params hm;
    struct bhp_params bhp;
    struct adj_matrix_params adjm;
    struct static_matrix_params smat;
    struct dynamic_matrix_params dmat;
  } type;

  /**
   * For comparing elements.
   *
   * Can be NULL for dynamic array, linked list, ringbuffer, fifo, hashmap.
   *
   * Cannot be NULL for binary search tree (and data structures derived from
   * binary search tree), binary heap.
   */
  int (*cmpe)(const void *const e1, const void *const e2);

  /**
   * For printing an element. Can be NULL for any data structure; only used
   * for diagnostic purposes.
   */
  void (*printe)(const void *e);

  /**
   * Pointer to space the application has allocated for the data structure to
   * reside in. This is NOT the same as space for the data that the data
   * structure is taking care of. For example, if a linked list is used, then
   * this is a pointer to a block of memory that the linked list data
   * structure will use to store its nodes in, instead of malloc()ing for
   * them. Ignored unless \ref DS_APP_DOMAIN_NODES is passed.
   *
   * Used by linked list, binary search tree (and derived structures), hashmap.
   */
  uint8_t *nodes;

  /**
   * Pointer to space the application has allocated for storing the actual
   * data that the data structure will be managing. This is NOT the same
   * as the space for the data structure itself. Ignored unless \ref
   * DS_APP_DOMAIN_DATA is passed.
   *
   * Used by all data structures.
   */
  uint8_t *elements;

  /**
   * Key for which member of union is valid. For data structures that do not
   * require additional parameters, and have no entry in the union above, it
   * serves as a sanity check for programmers to make sure the data structure
   * they are initializing is the one they intended.
   */
  enum ds_tag tag;
  size_t el_size;   /// size of elements in bytes.

  /**
   * Maximum # of elements allowed. -1 = no upper limit.
   *
   * Used by all data structures except hashmap.
   */
  int max_elts;
  uint32_t flags;  /// Initialization flags
};

/**
 * \brief Data structure iterator.
 *
 * Used to provide a uniform interface for iterating through data
 * structures. Not implemented on all data structures in library.
 */
struct ds_iterator {
  struct darray *arr;
  struct rbuffer *rb;
  struct llist *list;
  struct llist_node *curr;
  size_t index;
  enum ds_tag tag;

  /**
   * Classification function. Used to determine which elements are returned
   * during iteration.
   */
  bool_t (*classify)(void *e);
};

/*******************************************************************************
 * Inline Functions
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Calculate how large the chunk of memory for the metadata for a data
 * structure needs to be, given a max # of elements.
 *
 * Used in conjunction with \ref DS_APP_DOMAIN_DATA and/or \ref DS_APP_DOMAIN_NODES for
 * data structures requiring metadata.
 *
 * \param max_elts Max # elements the structure will manage.
 *
 * \return Total # of bytes required.
 */
static inline size_t ds_calc_meta_space(size_t max_elts) {
  return sizeof(int) * max_elts;
}

/**
 * \brief Calculate how large the chunk of memory for the elements that a data
 * structure will manage needs to be, given a max # of elements and element
 * size.
 *
 * For data structures that do NOT require metadata (FIFOs, ringbuffers, etc.).
 * You really shouldn't use this function--use the specific calculation
 * functions for each data structure found in their respective header
 * files. Used in conjunction with \ref DS_APP_DOMAIN_DATA.
 *
 * \param max_elts Max # elements the structure will manage
 * \param el_size Size of each element in bytes
 *
 * \return Total # of bytes required
 */
static inline size_t ds_calc_element_space1(size_t max_elts, size_t el_size) {
  return max_elts * el_size;
}

/**
 * \brief Calculate how large the chunk of memory for the elements that a data
 * structure will manage needs to be, given a max # of elements and element
 * size.
 *
 * For data structures that DO require metadata (trees, linked lists, etc.).
 * You really shouldn't use this function--use the specific calculation
 * functions for each data structure found in their respective header
 * files. Used in conjunction with \ref DS_APP_DOMAIN_DATA.
 *
 * \param max_elts Max # elements the structure will manage
 * \param el_size Size of each element in bytes
 *
 * \return Total # of bytes required
 */
static inline size_t ds_calc_element_space2(size_t max_elts, size_t el_size) {
  return ds_calc_meta_space(max_elts) +
      ds_calc_element_space1(max_elts, el_size);
}

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
/**
 * \brief - Get the next element that matches the iteration conditions
 *
 * \param iter The iterator
 *
 * \return The next element, or NULL if no more
 */
void *ds_iter_next(struct ds_iterator *iter);

/**
 * \brief Search a metadata area for a free datablock/node
 *
 * \param mem_p Pointer to the start of elements/nodes metadata block
 * \param ent_size Size of the entities managed by the metadata section
 * \param max_elts Max # of elements for the data structure (i.e. the size of
 * the metadata area)
 * \param index Index to start probing at. Filled with the found index
 *
 * \return Pointer to free entity, or NULL if none found
 **/
void *ds_meta_probe(uint8_t *mem_p, size_t ent_size, size_t max_elts,
                    size_t *index);
/**
 * \brief Initialize an iterator
 *
 * Initialize an iterator to iterate of some/all of the elements of a
 * data structure. Not implemented for all data structures yet...
 *
 * \param tag What type of data structure to initialize for
 * \param ds The data structure to iterate over
 * \param f The function to determine if an element will be returned by the iterator
 * or not. If NULL, all elements will be returned in order.
 *
 * \return The initialized iterator, or NULL if an ERROR occurred
 */
struct ds_iterator * ds_iter_init(enum ds_tag tag, void *ds,
                                  bool_t (*f)(void *e));

/**
 * \brief Utility function to copy elt2 into elt1, overwriting.
 *
 * If the element is larger than a double, memcpy() is used. If it is < than the
 * size of a double, pointers are used.
 *
 * \param elt1 Destination.
 * \param elt2 Source.
 * \param el_size Size of elements in bytes.
 *
 * \return \ref status_t
 */
status_t ds_elt_copy(void *elt1, const void *elt2, size_t el_size);

/**
 * \brief Utility function to clear an element.
 *
 * If the element is larger than a double, memcpy() is used. If it is < than the
 * size of a double, pointers are used.
 *
 * \param elt Element to clear.
 * \param el_size Size of element in bytes.
 *
 * \return \ref status_t
 */
status_t ds_elt_clear(void *elt, size_t el_size);

/**
 * \brief Utility function to check if an element is 0.
 *
 * If the element is larger than double, a for() loop is used. Otherwise
 * pointers are used.
 *
 * \param elt Element to check.
 * \param el_size Size of element in bytes.
 *
 * \return \ref bool_t
 */
bool_t ds_elt_zchk(void *elt, size_t el_size);

/**
 * \brief Utility function to swap two elements.
 *
 * If the element is larger than double, a for() loop is used. Otherwise
 * pointers are used.
 *
 * \param elt1 Element #1.
 * \param elt2 Element #2.
 * \param el_size Size of elements in bytes.
 *
 * \return \ref status_t
 */
status_t ds_elt_swap(void *elt1, void *elt2, size_t el_size);

END_C_DECLS
