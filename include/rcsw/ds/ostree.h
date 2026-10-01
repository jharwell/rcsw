/**
 * \file
 *
 * \copyright 2017 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * \ingroup ds
 *
 * \brief Order statistics tree.
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/al/types.h"
#include "rcsw/core/compilers.h"
#include "rcsw/core/core.h"
#include "rcsw/ds/bstree.h"
#include "rcsw/ds/ds.h"

/*******************************************************************************
 * Macros
 ******************************************************************************/
/**
 * \copydoc RCSW_BSTREE_ROOT()
 */
#define RCSW_OSTREE_ROOT(tree) ((struct ostree_node*)RCSW_BSTREE_ROOT(tree))

/*******************************************************************************
 * Types
 ******************************************************************************/
/**
 * \brief A node in an order-statistic tree, derived from a \ref bstree_node
 * (share common fields).
 *
 * \note With later GCC versions, if you don't have the casting right for
 * functions which are shared between bstree and ostree, things won't work
 * because of differing alignments/packing under optimizations.
 *
 * Must be packed and aligned to the same size as \ref dptr_t so that casts from
 * \ref ostree_node.data are safe on all targets.
 */
struct RCSW_ATTR(packed, aligned(sizeof(dptr_t))) ostree_node {
  uint8_t             key[RCSW_BSTREE_NODE_KEYSIZE];
  dptr_t*             data;
  struct ostree_node* left;
  struct ostree_node* right;
  struct ostree_node* parent;
  bool_t              red;

  /**
   * Size of subtree anchored at node (including node)
   */
  int32_t count;
};

/*******************************************************************************
 * Private API
 ******************************************************************************/
BEGIN_C_DECLS
/**
 * \brief Initialize the order-statistic tree specific bits of a BST.
 *
 * \param tree The ostree handle.
 */
RCSW_LOCAL void ostree_init_helper(struct bstree* tree);

/*******************************************************************************
 * Public API
 *
 * There are other bstree functions that you can use besides these; however,
 * given that you are using an Order Statistics tree, these are really the only
 * operations you should be doing (besides insert/delete). I don't wrap the
 * bstree API here for that reason.
 *
 ******************************************************************************/

/**
 * \copydoc bstree_element_space()
 */
static inline size_t ostree_element_space(size_t max_elts, size_t elt_size) {
  return bstree_element_space(max_elts, elt_size);
}

/**
 * \brief Calculate the space needed for the nodes in an order-statistic tree.
 *
 * Used in conjunction with \ref RCSW_NOALLOC_META. The +2 is for the root and
 * nil nodes.
 *
 * \param max_elts Max # of elements the tree will hold.
 *
 * \return The # of bytes required.
 */
static inline size_t ostree_meta_space(size_t max_elts) {
  return ds_meta_space(max_elts + 2) +
         ds_elt_space_simple(max_elts + 2, sizeof(struct ostree_node));
}

/**
 * \copydoc bstree_delete()
 */
static inline status_t ostree_delete(struct bstree*      tree,
                                     struct ostree_node* victim,
                                     void*               elt) {
  return bstree_delete(tree, (struct bstree_node*)victim, elt);
}

/**
 * \copydoc bstree_remove()
 */
static inline status_t ostree_remove(struct bstree* tree, const void* key) {
  return bstree_remove(tree, key);
}

/**
 * \copydoc bstree_destroy()
 */
static inline void ostree_destroy(struct bstree* tree) {
  return bstree_destroy(tree);
}

/**
 * \copydoc bstree_node_query()
 */
static inline struct ostree_node* ostree_node_query(
  const struct bstree* const tree,
  struct ostree_node*        search_root,
  const void* const          key) {
  return (struct ostree_node*)bstree_node_query(tree,
                                                (struct bstree_node*)search_root,
                                                key);
}

/**
 * \brief Select the ith smallest element in the order-statistic tree.
 *
 * \param tree The ostree handle.
 *
 * \param node_in The node to start searching at (should probably be the root).
 *
 * \param i The element rank to select.
 *
 * \return The ith smallest element, or NULL if no such element or an error
 * occurred.
 */
RCSW_API struct ostree_node* ostree_select(const struct bstree* tree,
                                           struct ostree_node*  node_in,
                                           int                  i);

/**
 * \brief Get the rank of an element within an order-statistic tree.
 *
 * \param tree The OStree handle.
 * \param node The node to get the rank of.
 *
 * \return The rank, or -1 on ERROR.
 */
RCSW_API int ostree_rank(const struct bstree*      tree,
                         const struct ostree_node* node);

/**
 * \brief Initialize an order statistics tree.
 *
 * \param tree_in Caller storage for the handle, used only if \ref
 *                RCSW_NOALLOC_HANDLE is passed; ignored (may be NULL)
 *                otherwise. See \rcswdoc{concepts/memory-model}.
 * \param params Initialization parameters.
 *
 * \return The initialized tree, or NULL if an error occurred.
 */
RCSW_API struct bstree* ostree_init(struct bstree*              tree_in,
                                    const struct bstree_config* params);

/**
 * \brief Insert an element into an order-statistic tree.
 *
 * \param tree The ostree handle.
 * \param key The key for the data to insert.
 * \param data The data to insert.
 *
 * \return \ref status_t
 */
RCSW_API status_t ostree_insert(struct bstree* tree, void* key, void* data);

END_C_DECLS
