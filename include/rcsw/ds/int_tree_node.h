/**
 * \file int_tree_node.h
 * \cond INTERNAL
 * \ingroup ds
 * \brief Functions related to interval tree nodes.
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

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

