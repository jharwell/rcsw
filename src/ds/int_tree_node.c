/**
 * \file int_tree_node.c
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/int_tree_node.h"

/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS

void int_tree_node_update_max(struct int_tree_node* const node) {
  node->max_high = RCSW_MAX3(node->left->max_high,
                        node->right->max_high,
                        ((struct interval_data*)node->data)->high);
} /* int_tree_node_update_max() */

END_C_DECLS
