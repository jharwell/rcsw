/**
 * \file inttree.c
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/inttree.h"

#include <limits.h>

#include "rcsw/er/client.h"
#include "rcsw/common/fpc.h"
#include "rcsw/ds/bstree_node.h"
#include "rcsw/ds/inttree_node.h"
#include "rcsw/utils/utils.h"

/*******************************************************************************
 * Static Functions
 ******************************************************************************/
BEGIN_C_DECLS
/**
 * \brief Compare two intervals to see if they overlap
 *
 * \param a Interval #1
 * \param b Interval #2
 *
 * \return TRUE if intervals overlap, FALSE otherwise
 */
static bool_t inttree_cmp_overlap(const void* a, const void* b) {
  const struct interval_data* a_p = a;
  const struct interval_data* b_p = b;
  if (a_p->low <= b_p->low) {
    return (bool_t)(b_p->low <= a_p->high);
  } else {
    return (bool_t)(a_p->low <= b_p->high);
  }
} /* inttree_cmp_overlap() */

/*******************************************************************************
 * API Functions
 ******************************************************************************/
int inttree_cmp_key(const void* a, const void* b) {
  const int32_t* a_p = a;
  const int32_t* b_p = b;
  return (*a_p - *b_p);
} /* inttree_cmp_key() */

void inttree_init_helper(const struct bstree* tree) {
  struct interval_data nil_data = { .low = INT_MIN, .high = INT_MIN };
  ds_elt_copy(tree->nil->data, &nil_data, tree->elt_size);
  ds_elt_copy(tree->root->data, &nil_data, tree->elt_size);
  ((struct inttree_node*)tree->nil)->max_high = INT_MIN;
  ((struct inttree_node*)tree->root)->max_high = INT_MIN;
} /* inttree_init_helper() */

struct inttree_node*
inttree_overlap_search(const struct bstree* tree,
                       struct inttree_node* root,
                       const struct interval_data* interval) {
  RCSW_FPC_NV(NULL, NULL != tree, NULL != root, NULL != interval);

  /*
   * Not found in an empty subtree (obviously)
   */
  if (root == (struct inttree_node*)tree->nil) {
    return NULL;
  }
  /*
   * If found, return the first overlapping interval. There may be more!!
   */
  if (inttree_cmp_overlap(root->data, interval)) {
    return root;
  }

  /*
   * If the left child of the current root exists and its max is >= the low of
   * the given interval, then the interval MIGHT overlap with an interval
   * somewhere in the left subtree.
   */
  if (root->left != (struct inttree_node*)tree->nil &&
      root->left->max_high >= interval->low) {
    return inttree_overlap_search(tree, root->left, interval);
  } else { /* otherwise, any overlap can only reside in the right subtree */
    return inttree_overlap_search(tree, root->right, interval);
  }
} /* inttree_overlap_search() */

void inttree_high_fixup(const struct bstree* tree, struct inttree_node* node) {
  node->max_high = ((struct interval_data*)node->data)->high;
  while (node != (struct inttree_node*)tree->root) {
    inttree_node_update_max(node);
    node = node->parent;
  } /* while() */
} /* inttree_high_fixup() */

END_C_DECLS
