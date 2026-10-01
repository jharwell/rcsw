/**
 * \file
 *
 * \copyright 2017 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * \ingroup ds
 *
 * \brief Interval tree.
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/al/types.h"
#include "rcsw/core/compilers.h"
#include "rcsw/core/core.h"
#include "rcsw/ds/bstree.h"

/*******************************************************************************
 * Macros
 ******************************************************************************/
/**
 * Convenience macro for getting a reference to the root node in the tree
 */
#define RCSW_INTTREE_ROOT(tree) ((struct inttree_node*)RCSW_BSTREE_ROOT(tree))

/*******************************************************************************
 * Types
 ******************************************************************************/
/**
 * A simple representation of an interval for use in an interval tree.
 *
 * Must be packed and aligned to the same size as \ref dptr_t so that casts from
 * \ref inttree_node.data are safe on all targets.
 */
struct RCSW_ATTR(packed, aligned(sizeof(dptr_t))) interval_data {
  int32_t high;
  int32_t low;
};

/**
 * \brief A node in an interval tree.
 *
 * Note that the first fields are identical to the ones in the bstree_node; this
 * is necessary for the casting to "up" the inheritance tree to work.
 *
  Must be packed and aligned to the same size as \ref dptr_t so that casts from
 * \ref inttree_node.data are safe on all targets.
 */
struct RCSW_ATTR(packed, aligned(sizeof(dptr_t))) inttree_node {
  uint8_t              key[RCSW_BSTREE_NODE_KEYSIZE];
  dptr_t*              data;
  struct inttree_node* left;
  struct inttree_node* right;
  struct inttree_node* parent;
  bool_t               red;

  /**
   * This field is the largest HIGH value of the subtree rooted at this node.
   */
  int32_t max_high;
};

/*******************************************************************************
 * Private API
 ******************************************************************************/
/**
 * \brief Initialize the interval tree specific bits of a BST
 *
 * \param tree The partially constructed interval tree.
 */
RCSW_LOCAL void inttree_init_helper(const struct bstree* tree);

/**
 * \brief Fixup high field for all nodes above target node after an
 * insertion/deletion
 *
 * \param tree The interval tree handle.
 * \param node The leaf node to propagate fixes up the tree for.
 */

RCSW_LOCAL void inttree_high_fixup(const struct bstree* tree,
                                   struct inttree_node* node);

/*******************************************************************************
 * Public API
 *
 * There are other bstree functions that you can use besides these; however,
 * given that you are using an Interval tree, these are really the only
 * operations you should be doing (besides insert/delete). I don't wrap the
 * whole bstree API here for that reason.
 *
 ******************************************************************************/
BEGIN_C_DECLS
/**
 * \brief Determine if an interval tree is full.
 *
 * \param tree The interval tree handle.
 */
static inline bool_t inttree_isfull(const struct bstree* const tree) {
  return bstree_isfull(tree);
}

/**
 * \brief Determine if an interval tree is empty.
 *
 * \param tree The interval tree handle.
 */
static inline bool_t inttree_isempty(const struct bstree* const tree) {
  return bstree_isempty(tree);
}

/**
 * \brief Get # elements currently in an interval tree.
 *
 * \param tree The interval tree handle.
 *
 * \return The # of elements, or 0 on error.
 */
static inline size_t inttree_size(const struct bstree* const tree) {
  return bstree_size(tree);
}

/**
 * \brief Calculate the # of bytes that the interval tree will require if \ref
 * RCSW_NOALLOC_DATA is passed to manage a specified # of elements of a
 * specified size.
 *
 * \param max_elts # of desired elements the tree will hold
 *
 * \return The total # of bytes the application would need to allocate
 */
static inline size_t inttree_element_space(size_t max_elts) {
  return bstree_element_space(max_elts, sizeof(struct interval_data));
}

/**
 * \brief Calculate the space needed for the nodes in the interval tree, given a
 * max # of elements
 *
 * Used in conjunction with \ref RCSW_NOALLOC_META.
 *
 * \param max_elts # of desired elements the tree will hold
 *
 * \return The # of bytes required
 */
static inline size_t inttree_meta_space(size_t max_elts) {
  return bstree_meta_space(max_elts);
}

/**
 * \copydoc bstree_delete()
 */
static inline status_t inttree_delete(struct bstree*       tree,
                                      struct inttree_node* victim,
                                      void*                elt) {
  return bstree_delete(tree, (struct bstree_node*)victim, elt);
}

/**
 * \copydoc bstree_remove()
 */
static inline status_t inttree_remove(struct bstree* tree, const void* key) {
  return bstree_remove(tree, key);
}

/**
 * \brief Insert an interval into an interval tree.
 *
 * \param tree The interval tree handle.
 * \param interval The interval to insert.
 *
 * \return \ref status_t
 */
RCSW_API status_t inttree_insert(struct bstree*        tree,
                                 struct interval_data* interval);

/**
 * \brief Initialize an interval tree.
 *
 * \p params->elt_size must be \c sizeof(struct interval_data); it is not set
 * automatically.
 *
 * \param tree_in Caller storage for the handle, used only if \ref
 *                RCSW_NOALLOC_HANDLE is passed; ignored (may be NULL)
 *                otherwise. See \rcswdoc{concepts/memory-model}.
 * \param params Initialization parameters.
 *
 * \return The initialized tree, or NULL if an error occurred.
 */
RCSW_API struct bstree* inttree_init(struct bstree*        tree_in,
                                     struct bstree_config* params);

/**
 * \brief Determine if the given interval overlaps any in the interval tree.
 *
 * To search the entire tree, pass \ref RCSW_INTTREE_ROOT(tree) as \p root.
 * (\c tree->root is a sentinel, not the root node.)
 *
 * \param tree The interval tree handle
 * \param root Root of tree to start searching at
 * \param interval The interval to test for overlaps with.
 *
 * \return The first overlapping interval encountered, or NULL if none was found
 * or an error occurred.
 */
RCSW_API struct inttree_node* inttree_overlap_search(
  const struct bstree*        tree,
  struct inttree_node*        root,
  const struct interval_data* interval);

END_C_DECLS
