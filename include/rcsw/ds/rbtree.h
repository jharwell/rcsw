/**
 * \file rbtree.h
 * \ingroup ds
 * \brief Implementation of Red-Black Tree.
 *
 * This was really hard to get to work.
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * This file is part of RCSW.
 *
 * RCSW is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * RCSW is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * RCSW.  If not, see <http://www.gnu.org/licenses/
 */

#ifndef INCLUDE_RCSW_DS_RBTREE_H_
#define INCLUDE_RCSW_DS_RBTREE_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/bstree.h"

/*******************************************************************************
 * Forward Declarations
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
void rbtree_delete_fixup(struct bstree* tree, struct bstree_node* node);

/**
 * \brief  Fix up tree structure after an insertion
 *
 * \param tree The rbtree handle.
 * \param node The parent the node that was inserted.
 */
void rbtree_insert_fixup(struct bstree* tree, struct bstree_node * node);

/**
 * \brief Get the black height of a node in an RB Tree.
 *
 * \param node The root of the tree to get the height of.
 *
 * \return The height or 0 if the node is NULL.
 *
 **/
int rbtree_node_black_height(const struct bstree_node *node) RCSW_PURE;

END_C_DECLS

#endif /* INCLUDE_RCSW_DS_RBTREE_H_ */
