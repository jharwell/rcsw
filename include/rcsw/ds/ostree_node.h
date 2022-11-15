/**
 * \file ostree_node.h
 * \cond INTERNAL
 * \ingroup ds
 * \brief Functions related to ostree nodes
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

#ifndef INCLUDE_RCSW_DS_OSTREE_NODE_H_
#define INCLUDE_RCSW_DS_OSTREE_NODE_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/ostree.h"

/*******************************************************************************
 * Constant Definitions
 ******************************************************************************/
/**
 * Whether the last operation was an insert or delete (fixup procedure is
 * different)
 */
enum ostree_fixup_type {
    OSTREE_FIXUP_INSERT,
    OSTREE_FIXUP_DELETE,
};

/*******************************************************************************
 * Forward Declarations
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Fixup count field for all nodes above target node after an
 * insertion/deletion.
 *
 * Do not call this function directly.
 *
 * \param tree The ostree handle.
 * \param node The node to fixup.
 * \param type The type of fixup to perform.
 */
void ostree_count_fixup(const struct bstree* tree,
                        struct ostree_node * node,
                        enum ostree_fixup_type type);
/**
 * \brief Update the subtree size for a node.
 *
 * Do not call this function directly.
 *
 * \param node The node to update.
 */
void ostree_node_update_count(struct ostree_node* node);

END_C_DECLS

/* \endcond */

#endif /* INCLUDE_RCSW_DS_OSTREE_NODE_H_ */
