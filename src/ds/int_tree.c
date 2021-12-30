/**
 * @file int_tree.c
 *
 * @copyright 2017 John Harwell, All rights reserved.
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

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/int_tree.h"
#include <limits.h>
#include "rcsw/common/dbg.h"
#include "rcsw/common/fpc.h"
#include "rcsw/ds/bstree_node.h"
#include "rcsw/ds/int_tree_node.h"
#include "rcsw/utils/utils.h"

/*******************************************************************************
 * Forward Declarations
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * @brief Compare two intervals to see if they overlap
 *
 * @param a Interval #1
 * @param b Interval #2
 *
 * @return TRUE if intervals overlap, FALSE otherwise
 */
static bool_t int_tree_cmp_overlap(const void* a, const void* b);

/*******************************************************************************
 * API Functions
 ******************************************************************************/
int int_tree_cmp_key(const void* a, const void* b) {
  const int* a_p = a;
  const int* b_p = b;
  return (*a_p - *b_p);
} /* int_tree_cmp_key() */

void int_tree_init_helper(const struct bstree* tree) {
  struct interval_data nil_data = {.low = INT_MIN, .high = INT_MIN};
  ds_elt_copy(tree->nil->data, &nil_data, tree->el_size);
  ds_elt_copy(tree->root->data, &nil_data, tree->el_size);
  ((struct int_tree_node*)tree->nil)->max_high = INT_MIN;
  ((struct int_tree_node*)tree->root)->max_high = INT_MIN;
} /* int_tree_init_helper() */

struct int_tree_node* int_tree_overlap_search(
    const struct bstree* tree,
    struct int_tree_node* root,
    const struct interval_data* interval) {
  RCSW_FPC_NV(NULL, NULL != tree, NULL != root, NULL != interval);

  /*
   * Not found in an empty subtree (obviously)
   */
  if (root == (struct int_tree_node*)tree->nil) {
    return NULL;
  }
  /*
   * If found, return the first overlapping interval. There may be more!!
   */
  if (int_tree_cmp_overlap(root->data, interval)) {
    return root;
  }

  /*
   * If the left child of the current root exists and its max is >= the low of
   * the given interval, then the interval MIGHT overlap with an interval
   * somewhere in the left subtree.
   */
  if (root->left != (struct int_tree_node*)tree->nil &&
      root->left->max_high >= interval->low) {
    return int_tree_overlap_search(tree, root->left, interval);
  } else { /* otherwise, any overlap can only reside in the right subtree */
    return int_tree_overlap_search(tree, root->right, interval);
  }
} /* int_tree_overlap_search() */

void int_tree_high_fixup(const struct bstree* tree, struct int_tree_node* node) {
  node->max_high = ((struct interval_data*)node->data)->high;
  while (node != (struct int_tree_node*)tree->root) {
    int_tree_node_update_max(node);
    node = node->parent;
  } /* while() */
} /* int_tree_high_fixup() */

/*******************************************************************************
 * Static Functions
 ******************************************************************************/
static bool_t int_tree_cmp_overlap(const void* a, const void* b) {
  const struct interval_data* a_p = a;
  const struct interval_data* b_p = b;
  if (a_p->low <= b_p->low) {
    return (bool_t)(b_p->low <= a_p->high);
  } else {
    return (bool_t)(a_p->low <= b_p->high);
  }
} /* int_tree_cmp_overlap() */

END_C_DECLS
