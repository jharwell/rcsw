/**
 * \file bstree.h
 * \ingroup ds
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

/*******************************************************************************
 * Constant Definitions
 ******************************************************************************/
/**
 * \brief The size of the node keys in a \ref bstree.
 */
#define RCSW_BSTREE_NODE_KEYSIZE sizeof(int32_t)

/**
 * \brief The different traversal types the tree supports.
 */
enum bstree_traversal_type {
  ekTRAVERSE_PREORDER,
  ekTRAVERSE_INORDER,
  ekTRAVERSE_POSTORDER,
};

/*******************************************************************************
 * Macros
 ******************************************************************************/
/**
 * Convenience macro for getting a reference to the root node in the tree.
 */
#define RCSW_BSTREE_ROOT(tree) ((tree)->root->left)

/*******************************************************************************
 * Structure Definitions
 ******************************************************************************/
/**
 * \brief Parameters for \ref bstree.
 */
struct bstree_params {
  /**
   * For comparing keys associated with elements. Cannot be NULL.
   */
  int (*cmpkey)(const void *const e1, const void *const e2);

  /**
   * For printing an element. Can be NULL.
   */
  void (*printe)(const void *e);

  /**
   * Pointer to application-allocated space for storing the \ref bstree
   * nodes. Ignored unless \ref RCSW_NOALLOC_META is passed.
   */
  dptr_t *meta;

  /**
   * Pointer to application-allocated space for storing data managed by the \ref
   * bstree. Ignored unless \ref RCSW_NOALLOC_DATA is passed.
   */
  dptr_t *elements;

  /**
   * Size of elements in bytes.
   */
  size_t elt_size;

  /**
   * Maximum number of elements allowed. -1 = no upper limit.
   */
  int max_elts;

  /**
   * Configuration flags. See \ref bstree.flags for valid flags.
   */
  uint32_t flags;
};

/**
 * \brief The base node for all binary search trees.
 *
 * For all nodes from trees derived from bstree, the node MUST:
 *
 * (1) Have all the fields present below and have them declared in the same
 * order as the "parent" structs.
 *
 * (2) Have the same size as the "parent" struct entries.
 *
 * This is so that I can take the derived class struct, cast it to a
 * bstree/bstree_node, and pass it to a function in the bstree class, and have
 * everything work as desired.
 *
 * Must be packed and aligned to the same size as \ref dptr_t so that casts from
 * \ref bstree_node.data are safe on all targets.
 */
struct RCSW_ATTR(packed, aligned (sizeof(dptr_t))) bstree_node {
  uint8_t key[RCSW_BSTREE_NODE_KEYSIZE];
  dptr_t *data;
  struct bstree_node *left;
  struct bstree_node *right;
  struct bstree_node *parent;

  /**
   * If the tree is used as a red-black tree, then this field is used to
   * indicate the color of the node. It is undefined for regular BSTs. If
   * red is false then the node is black.
   */
  bool_t red;

  /* extra padding added for compatibility with ostree_node and inttree_node */
  int32_t tmp;
};

/**
 * \brief Container for all memory (space) used by a \ref bstree.
 */
struct bstree_space_mgmt {
  /**
   * Space for the data elements. Used if \ref RCSW_NOALLOC_DATA passed in \ref
   * bstree_params.flags.
   */
  dptr_t*             datablocks;

  /**
   * Space for the allocation map for datablocks. Used if \ref RCSW_NOALLOC_DATA
   * passed in \ref bstree_params.flags.
   */
  struct allocm_entry* db_map;

  /**
   * Space for \ref bstree_node objects. Used if \ref RCSW_NOALLOC_META
   * passed in \ref bstree_params.flags.
   */
  struct bstree_node*  nodes;

  /**
   * Space for the allocation map for \ref bstree_node objects. Used if \ref
   * RCSW_NOALLOC_META passed in \ref bstree_params.flags.
   */
  struct allocm_entry* node_map;
};

/**
 * \brief Binary Search Tree using  approach from Introduction to Algorithms.
 */
struct bstree {
  /**
   * For comparing the keys from two different elements. Cannot be NULL.
   */
  int (*cmpkey)(const void *const a, const void *const b);

  /**
   * For printing an element. Can be NULL.
   */
  void (*printe)(const void *const e);

  /**
   * Management of all node and element space for the tree.
   */
  struct bstree_space_mgmt space;

  /**
   * Number of nodes/elements currently in the tree.
   */
  size_t current;

  /**
   * Current depth as a traversal progresses.
   */
  size_t depth;

  /**
   * Configuration flags. Valid flags are:
   *
   * - \ref RCSW_NOALLOC_HANDLE
   * - \ref RCSW_NOALLOC_DATA
   * - \ref RCSW_NOALLOC_META
   * - \ref RCSW_DS_BSTREE_INT
   * - \ref RCSW_DS_BSTREE_OS
   * - \ref RCSW_DS_BSTREE_RB
   * - \ref RCSW_ZALLOC
   *
   * All other flags are ignored.
   */
  uint32_t flags;

  /**
   * Max # of elements for tree. -1 indicates no limit.
   */

  int max_elts;

  /**
   * Size of each element in bytes.
   */
  size_t elt_size;

  /**
   * Root of the tree (sentinel). root->left should always point to the node
   * which is the root of the tree. Use this so that there are fewer special
   * cases/less checking for NULL pointers.
   */
  struct bstree_node *root;

  /**
   * Sentinel. Points to a node which should always be black (for red-black
   * trees) but has aribtrary children and parent. Use this so that there are no
   * fewer special cases in the code/less checking for NULL pointers.
   */
  struct bstree_node *nil;
};

/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS

/*******************************************************************************
 * RCSW Private Functions
 ******************************************************************************/
/**
 * \brief Insert a new data into a BST
 *
 * This should NEVER be called by an application--for internal use only.
 *
 * \param tree The BST handle
 * \param key The key of the data to insert
 * \param data The data to insert
 * \param node_size The size of the nodes in the tree, in bytes
 *
 * \return \ref status_t
 */
RCSW_LOCAL status_t bstree_insert_internal(struct bstree * tree,
                                void * key, void * data,
                                size_t node_size);
/**
 * \brief Initialize a binary search (or related) tree, which may have different
 * node sizes.
 *
 * This should NEVER be called by an application--for internal use only.
 *
 * \param tree_in The BST handle to be filled. Must be non-NULL if \ref
 *                RCSW_NOALLOC_HANDLE passed in \ref bstree_params.flags.
 *
 * \param params Initialization parameters
 *
 * \param node_size The size of the nodes in the tree, in bytes
 *
 * \return The initialized tree, or NULL if an error occurred
 */
RCSW_LOCAL struct bstree *bstree_init_internal(struct bstree *tree_in,
                                    const struct bstree_params * params,
                                    size_t node_size) RCSW_CHECK_RET;


/*******************************************************************************
 * API Functions
 ******************************************************************************/
/**
 * \brief Determine if a \ref bstree is currently full.
 *
 * \param bst The BST handle
 *
 * \return \ref bool_t
 */
static inline bool_t bstree_isfull(const struct bstree* const bst) {
  RCSW_FPC_NV(false, NULL != bst);
  return (bst->current == (size_t)bst->max_elts);
}

/**
 * \brief Determine if a \ref bstree is currently empty.
 *
 * \param bst The BST handle
 *
 * \return \ref bool_t
 */
static inline bool_t bstree_isempty(const struct bstree* const bst) {
  RCSW_FPC_NV(false, NULL != bst);
  return (bst->current == 0);
}

/**
 * \brief Determine # elements currently a \ref bstree.
 *
 * \param bst The BST handle
 *
 * \return # elements in bst, or 0 on ERROR
 */

static inline size_t bstree_size(const struct bstree* const bst) {
  RCSW_FPC_NV(0, NULL != bst);
  return bst->current;
}

/**
 * \brief Calculate the # of bytes that the BST will require if \ref
 * RCSW_NOALLOC_DATA is passed to manage a specified # of elements of a
 * specified size.
 *
 * The +2 is for the root and nil nodes.
 *
 * \param max_elts # of desired elements the BST will hold
 * \param elt_size size of elements in bytes
 *
 * \return The total # of bytes the application would need to allocate
 */
static inline size_t bstree_element_space(size_t max_elts, size_t elt_size) {
  return ds_elt_space_with_meta(max_elts+2, elt_size);
}

/**
 * \brief Calculate the space needed for the nodes in a \ref bstree.
 *
 * Used in conjunction with \ref RCSW_NOALLOC_META. The +2 is for the root and
 * nil nodes.
 *
 * \param max_elts Max # of elements the BST will hold.
 *
 * \return The # of bytes required.
 */
static inline size_t bstree_meta_space(size_t max_elts) {
  return ds_meta_space(max_elts+2) +
      ds_elt_space_simple(max_elts+2, sizeof(struct bstree_node));
}

/**
 * \brief Insert an item into a \ref bstree
 *
 * \param tree The BST handle

 * \param key The key for the data to insert
 *
 * \param data The data to insert
 */
RCSW_API status_t bstree_insert(struct bstree* tree,
                                void* const key,
                                void* const data);

/**
 * \brief Initialize a \ref bstree.
 *
 * \param tree_in The BST handle
 *
 * \param params Initialization parameters
 */
RCSW_API struct bstree* bstree_init(struct bstree* tree_in,
                                    const struct bstree_params* const params);

/**
 * \brief Destroy a binary search tree
 *
 * Any further use of the tree after calling this function is undefined.
 *
 * \param tree The BST to destroy
 *
 */
RCSW_API void bstree_destroy(struct bstree *tree);


/**
 * \brief Remove the node in a BST that contains data that matches the given key
 *
 * \param tree The BST handle
 * \param key Key to match in the search for the node to delete
 *
 * \return \ref status_t
 */
RCSW_API status_t bstree_remove(struct bstree * tree, const void * key);

/**
 * \brief Delete a node from the tree
 *
 * The algorithm from this function is from _Introduction_To_Algorithms_
 *
 * \param tree The BST handle
 * \param victim The node to delete
 * \param elt To be filled with the data of the deleted node if non-NULL
 *
 * \return \ref status_t
 */
RCSW_API status_t bstree_delete(struct bstree* tree, struct bstree_node* victim,
                       void * elt);

/**
 * \brief Query a BST for a specific node, starting the search at the specified
 * search root
 *
 * \param tree The BST handle
 * \param search_root The root of the search (not necessarily the root of the
 * tree)
 * \param key The key to match
 *
 * \return The node with the matching key, or NULL if not found
 */
RCSW_API struct bstree_node *bstree_node_query(const struct bstree* tree,
                                      struct bstree_node* search_root,
                                      const void* key);

/**
 * \brief Get the data associated with a key
 *
 * This function returns the data in the first node for which the compare
 * function returned 0 for.
 *
 * \param tree The BST handle
 * \param key The key to search for
 *
 * \return The data associated with the key, or NULL if no match for key was
 * found
 */
RCSW_API void *bstree_data_query(const struct bstree * tree, const void * key);

/**
 * \brief Traverse a binary search tree and operate on each node's data.
 *
 * Can traverse in a pre-order, post-order, or in-order way.
 *
 * If the callback returns nonzero on a given node, then the traversal is
 * aborted.
 *
 * \param tree The BST handle
 *
 * \param cb Callback called on each node in the tree, and passed the BST
 *           handle. Cannot be used to modify the BST handle, but CAN modify the
 *           node.
 *
 * \param type The type of traversal to perform.
 *
 * \return Return code of last callback that was non-zero, or 0 if callback
 * succeeded on all nodes
 */
RCSW_API int bstree_traverse(struct bstree * tree,
                    int (*cb)(const struct bstree* tree,
                              struct bstree_node * node),
                    enum bstree_traversal_type type);

/**
 * \brief Print a BSTREE
 *
 * \param tree The BST handle
 */
RCSW_API void bstree_print(struct bstree * tree);



END_C_DECLS
