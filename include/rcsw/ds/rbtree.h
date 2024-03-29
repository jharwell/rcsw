/**
 * \file rbtree.h
 * \ingroup ds
 * \brief Implementation of Red-Black Tree.
 *
 * This was really hard to get to work.
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/bstree.h"

/*******************************************************************************
 * RCSW Private Functions
 ******************************************************************************/
BEGIN_C_DECLS

/**
 *  \brief Peform rotations and change colors to restore red-black properties
 *  after a node is deleted.
 *
 *  The algorithm is from _Introduction_To_Algorithms_
 *
 * \param tree The rbtree handle.
 * \param node The parent (or successor) of the node that was deleted.
 *
 */
RCSW_API void rbtree_delete_fixup(struct bstree* tree, struct bstree_node* node);

/**
 * \brief  Fix up tree structure after an insertion
 *
 * \param tree The rbtree handle.
 * \param node The parent the node that was inserted.
 */
RCSW_API void rbtree_insert_fixup(struct bstree* tree,
                                  struct bstree_node * node);

/*******************************************************************************
 * API Functions
 ******************************************************************************/
/**
 * \brief Get the black height of a node in an RB Tree.
 *
 * \param node The root of the tree to get the height of.
 *
 * \return The height or 0 if the node is NULL.
 *
 **/
RCSW_API int rbtree_node_black_height(const struct bstree_node *node) RCSW_PURE;

END_C_DECLS
