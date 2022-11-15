/**
 * \file int_tree_node.h
 * \cond INTERNAL
 * \ingroup ds
 * \brief Functions related to interval tree nodes.
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

#ifndef INCLUDE_RCSW_DS_INT_TREE_NODE_H_
#define INCLUDE_RCSW_DS_INT_TREE_NODE_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/int_tree.h"

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Update the max high interval for a node during the fixup process after
 * an insertion/deletion.
 *
 * Do not call this function directly.
 *
 * \param node The node to update.
 */
void int_tree_node_update_max(struct int_tree_node* node);

/* \endcond */
END_C_DECLS

#endif /* INCLUDE_RCSW_DS_INT_TREE_NODE_H_ */
