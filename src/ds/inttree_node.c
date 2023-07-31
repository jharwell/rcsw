/**
 * \file inttree_node.c
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/inttree_node.h"

/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS

void inttree_node_update_max(struct inttree_node* const node) {
  struct interval_data* data = (struct interval_data*)node->data;
  node->max_high = RCSW_MAX3(node->left->max_high,
                             node->right->max_high,
                             data->high);
} /* inttree_node_update_max() */

END_C_DECLS
