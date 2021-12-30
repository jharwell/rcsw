/**
 * @file bstree-test.cpp
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
extern "C" {
#include "rcsw/ds/rbtree.h"
#include "rcsw/ds/int_tree.h"
#include "rcsw/ds/ostree.h"
#include "rcsw/ds/bstree_node.h"
#include "rcsw/common/dbg.h"
#include "rcsw/utils/utils.h"
#include "tests/ds_test.h"
}

#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_PREFIX_ALL
#include <catch.hpp>

/******************************************************************************
 * Constant definitions
 *****************************************************************************/
#define NUM_TESTS       2
#define MODULE_ID M_TESTING

/*******************************************************************************
 * Global Variables
 ******************************************************************************/
int n_elements; /* global var for # elements in RBTREE */

/*******************************************************************************
 * Test Functions Forward Declarations
 ******************************************************************************/
/**
 * @brief Test inserting nodes in a binary search tree and verifying
 *
 * This routine creates 3 BSTREEs of sorted, reverse-sorted, and random
 * data of the specified size and verifies the integrity of the BSTREE
 * after each insertion.
 */
static void insert_test(int len, struct ds_params *params,
                        int (*verify_cb)(const struct bstree* const tree,
                                         struct bstree_node * const node));
/**
 * @brief Test removing elements from BSTREEs of different sizes
 *
 * This routine creates 3 BSTREEs of sorted, reverse-sorted, and random
 * data of the specified size and tests removing elements from them in both the
 * order they were inserted, and reverse- insert order.
 */
static void remove_test(int len, int remove_type, struct ds_params *params,
                        int (*verify_cb)(const struct bstree* const tree,
                                         struct bstree_node * const node));

/**
 * @brief Test inserting nodes in an interval tree and verifying
 *
 */
static void int_tree_insert_test(int len, struct ds_params *params);

/**
 * @brief Test removeing nodes in an interval tree and verifying
 */
static void int_tree_remove_test(int len, struct ds_params *params);

/**
 * @brief Test overlap search for interval trees
 */
static void int_tree_overlap_test(int len, struct ds_params *params);

/**
 * @brief Test rank for OStrees.
 */
static void ostree_rank_test(int len, struct ds_params *params);

/**
 * @brief Test select for OStrees.
 */
static void ostree_select_test(int len, struct ds_params *params);


/*******************************************************************************
 * Test Helper Functions Forward Declarations
 ******************************************************************************/
/**
 * @brief Verify parent-child relationships in a BSTREE (RB)
 *
 * @return 0 if OK, nonzero otherwise
 */
static int verify_nodes_rb(const struct bstree* const tree,
                           struct bstree_node * const node);
/**
 * @brief Verify parent-child relationships in a BSTREE
 *
 * @return 0 if OK, nonzero otherwise
 */
static int verify_nodes_bst(const struct bstree* const tree,
                            struct bstree_node * const node);
/**
 * @brief Verify parent-child relationships in an interval tree.
 *
 * @return 0 if OK, nonzero otherwise
 */
static int verify_nodes_int(const struct bstree* const tree,
                            struct int_tree_node * const node);


static void test_runner1(uint32_t extra_flags,
                  void (*test)(int len,
                               struct ds_params *params,
                               int (*verify_cb)(const struct bstree* const tree,
                                                struct bstree_node * const node)),
                  int (*verify_cb)(const struct bstree* const tree,
                                   struct bstree_node * const node));
static void test_runner2(uint32_t extra_flags,
                  void (*test)(int len,
                               int remove_type,
                               struct ds_params *params,
                               int (*verify_cb)(const struct bstree* const tree,
                                                struct bstree_node * const node)),
                  int (*verify_cb)(const struct bstree* const tree,
                                   struct bstree_node * const node));
void test_runner3(void (*test)(int len,
                               struct ds_params *params));

static void test_runner4(void (*test)(int len,
                                      struct ds_params *params));

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
/* Binary Search Tree tests */
CATCH_TEST_CASE("BSTree Insert Test", "[BSTree]") {
  test_runner1(0, insert_test, verify_nodes_bst);
}
CATCH_TEST_CASE("BSTree Remove Test", "[BSTree]") {
  test_runner2(0, remove_test, verify_nodes_bst);
}

/* Internal Tree tests */
CATCH_TEST_CASE("Interval Tree Insert Test", "[IntTree]") {
  test_runner3(int_tree_insert_test);
}
CATCH_TEST_CASE("Interval Tree Remove Test", "[IntTree]") {
  test_runner3(int_tree_remove_test);
}
CATCH_TEST_CASE("Interval Tree Overlap Test", "[IntTree]") {
  test_runner3(int_tree_overlap_test);
}

/* Order Statistics Tree tests */
CATCH_TEST_CASE("OSTree Select Test", "[OSTree]") {
  test_runner4(ostree_select_test);
}
CATCH_TEST_CASE("OSTree Rank Test", "[OSTree]") {
  test_runner4(ostree_rank_test);
}

/* Red-black Tree tests */
CATCH_TEST_CASE("RBTree Insert Test", "[RBTree]") {
  test_runner1(DS_BSTREE_REDBLACK, insert_test, verify_nodes_rb);
}
CATCH_TEST_CASE("RBTree Remove Test", "[RBTree]") {
  test_runner2(DS_BSTREE_REDBLACK, remove_test, verify_nodes_rb);
}

/*******************************************************************************
 * Test Functions
 ******************************************************************************/
void insert_test(int len, struct ds_params *params,
                 int (*verify_cb)(const struct bstree* const tree,
                                  struct bstree_node * const node)) {
  struct bstree* tree;
  struct bstree mytree;

  struct element data_arr[len+1];
  int key_arr[len+1];

  tree = bstree_init(&mytree, params);
  CATCH_REQUIRE(NULL != tree);

  /* Build tree with random keys and verify insertions */
  for (int i = 0; i < len; ++i) {
    int rand_key;
    string_gen((char*)&rand_key, BSTREE_NODE_KEYSIZE);
    struct element e = {.value1 = i, .value2 = 13};
    struct element *e_ptr;
    CATCH_REQUIRE(OK == bstree_insert(tree, &rand_key, &e));
    data_arr[i] = e;
    key_arr[i] = rand_key;
    e_ptr = (element*)bstree_data_query(tree, &key_arr[i]);
    CATCH_REQUIRE(NULL != e_ptr);
    CATCH_REQUIRE(e_ptr->value1 == data_arr[i].value1);
  } /* for() */

  /* verify node count */
  CATCH_REQUIRE(tree->current == len);

  /* verify all data in the trees */
  for (int i = 0; i < len; ++i) {
    struct element * e_ptr;
    e_ptr = (element*)bstree_data_query(tree, &key_arr[i]);
    CATCH_REQUIRE(e_ptr != NULL);
    CATCH_REQUIRE(e_ptr->value1 == data_arr[i].value1);
  } /* for() */

  /* verify BSTREE structure */
  n_elements = tree->current;
  CATCH_REQUIRE(bstree_traverse(tree, verify_cb,
                                BSTREE_TRAVERSE_INORDER) == OK);

  bstree_destroy(tree);

  /* verify all DS_APP_DOMAIN data deallocated */
  CATCH_REQUIRE(th_leak_check_data(params) == 0);
  CATCH_REQUIRE(th_leak_check_nodes(params) == 0);
} /* insert_test() */

void remove_test(int len, int remove_type, struct ds_params *params,
                 int (*verify_cb)(const struct bstree* const tree,
                                  struct bstree_node * const node)) {
  struct bstree* tree;
  struct bstree mytree;

  struct bstree_node arr1[len+1];
  struct element data_arr[len+1];
  int key_arr[len+1];

  tree = bstree_init(&mytree, params);
  CATCH_REQUIRE(NULL != tree);

  /*
   * Build tree with random keys (don't need to verify insertions--that is done
   * elsewhere)
   */
  for (int i = 1; i <= len; ++i) {
    int rand_key;
    string_gen((char*)&rand_key, BSTREE_NODE_KEYSIZE);
    struct element e = {.value1 = i, .value2 = 13};
    CATCH_REQUIRE(OK == bstree_insert(tree, &rand_key, &e));
    data_arr[i] = e;
    key_arr[i] = rand_key;
  } /* for() */

  /* test removing random elements until the tree is empty */
  unsigned old_count;
  for (int i = 1; i <= len; ++i) {
    int remove_index = (remove_type == 0)?i:len + 1 -i;

    /* tree 1 */
    CATCH_REQUIRE(bstree_data_query(tree, &key_arr[remove_index]) != NULL);
    old_count = tree->current;
    CATCH_REQUIRE(bstree_remove(tree, &key_arr[remove_index]) == OK);
    CATCH_REQUIRE(bstree_data_query(tree, &key_arr[remove_index]) == NULL);
    CATCH_REQUIRE(tree->current == old_count -1);

    /* Verify relative ordering of the nodes in the trees after
     * removal. Need the len - 1 so that I don't try to traverse a tree when
     * there is only a single node in it.
     */
    if (i >= len -1) {
      continue;
    }
    n_elements = tree->current;
    CATCH_REQUIRE(bstree_traverse(tree, verify_cb,
                                  BSTREE_TRAVERSE_INORDER) == OK);
  } /* for() */

  /* check the trees are now empty */
  CATCH_REQUIRE(tree->current == 0);

  bstree_destroy(tree);

  /* verify all APP_DOMAIN data deallocated */
  CATCH_REQUIRE(th_leak_check_data(params) == 0);
  CATCH_REQUIRE(th_leak_check_nodes(params) == 0);
} /* remove_test() */

void int_tree_insert_test(int len, struct ds_params *params) {
  struct bstree* tree;
  struct bstree mytree;
  struct interval_data arr1[len+1];

  tree = int_tree_init(&mytree, params);
  CATCH_REQUIRE(NULL != tree);

  /*
   * Build tree with random intervals. Verify insertions, so that I know the
   * auxiliary field is being updated properly through rotations.
   */
  for (int i = 0; i < len; ++i) {
    struct interval_data e;
    struct interval_data * e_ptr;
    e.low = i;
    e.high = i + 10;

    CATCH_REQUIRE(OK == int_tree_insert(tree, &e));
    arr1[i] = e;
    e_ptr = (interval_data*)bstree_data_query(tree, &arr1[i].low);
    CATCH_REQUIRE(NULL != e_ptr);
    CATCH_REQUIRE(e_ptr->low == arr1[i].low);
    CATCH_REQUIRE(e_ptr->high == arr1[i].high);
  }
  /* verify all data in tree */
  for (int i = 0; i < len; ++i) {
    struct interval_data * e_ptr;
    e_ptr = (interval_data*)bstree_data_query(tree, &arr1[i].low);
    CATCH_REQUIRE(e_ptr != NULL);
    CATCH_REQUIRE(e_ptr->low == arr1[i].low);
    CATCH_REQUIRE(e_ptr->high == arr1[i].high);
  } /* for() */

  bstree_destroy(tree);
} /* int_tree_insert_test() */

void int_tree_remove_test(int len, struct ds_params *params) {
  struct bstree* tree;
  struct bstree mytree;
  struct interval_data arr1[len+1];

  tree = int_tree_init(&mytree, params);
  CATCH_REQUIRE(NULL != tree);

  /*
   * Build tree with random intervals (no need to verify insertions--done
   * elsewhere)
   */
  for (int i = 0; i < len; ++i) {
    struct interval_data e;
    struct interval_data * e_ptr;
    e.low = i;
    e.high = i + 10;
    /* tree 1 */
    CATCH_REQUIRE(OK == int_tree_insert(tree, &e));
    arr1[i] = e;
  } /* for() */

  unsigned old_count;
  for (int i = 0; i < len; ++i) {
    int remove_index = i;

    CATCH_REQUIRE(bstree_data_query(tree, &arr1[remove_index].low) != NULL);
    old_count = tree->current;
    CATCH_REQUIRE(bstree_remove(tree, &arr1[remove_index].low) == OK);
    CATCH_REQUIRE(bstree_data_query(tree, &arr1[remove_index].low) == NULL);
    CATCH_REQUIRE(tree->current == old_count -1);

    n_elements = tree->current;
    CATCH_REQUIRE(bstree_traverse(tree,
                                  (int(*)(const bstree*, bstree_node*))verify_nodes_int,
                                  BSTREE_TRAVERSE_INORDER) == OK);
  } /* for() */

  bstree_destroy(tree);
} /* int_tree_remove_test() */

void int_tree_overlap_test(int len, struct ds_params *params) {
  struct bstree* tree;
  struct bstree mytree;
  struct interval_data insert_arr[len];
  struct interval_data search_arr[len];
  bool overlap_arr[len];

  if (params->flags & DS_APP_DOMAIN_HANDLE) {
    tree = int_tree_init(&mytree, params);
  } else {
    tree = int_tree_init(NULL, params);
  }
  CATCH_REQUIRE(NULL != tree);

  /*
   * Build tree with specific intervals (no need to verify insertions--done
   * elsewhere)
   */
  for (int i = 0; i < len; ++i) {
    struct interval_data e;
    struct interval_data * e_ptr;
    e.low = i*4;
    e.high = e.low + i*5;
    CATCH_REQUIRE(OK == int_tree_insert(tree, &e));
    insert_arr[i] = e;
  } /* for() */

  /*
   * Build query intervals and overlap arrays
   */
  for (int i = 0; i < len; ++i) {
    search_arr[i].low = rand() % 4;
    search_arr[i].high = search_arr[i].low + rand() % 5;
    overlap_arr[i] = false;
    for (int j = 0; j < len; ++j) {
      if (search_arr[i].low < insert_arr[j].low && search_arr[i].high >= insert_arr[j].low) {
        overlap_arr[i] = true;
      }
      if (search_arr[i].low >= insert_arr[j].low && search_arr[i].low <= insert_arr[j].high) {
        overlap_arr[i] = true;
      }
    } /* for(j..) */
  } /* for() */

  /*
   * Verify interval overlapping
   */
  for (int i = 0; i < len; ++i) {
    if (overlap_arr[i]) {
      CATCH_REQUIRE(NULL != int_tree_overlap_search(tree,
                                                    (struct int_tree_node*)tree->root,
                                                    search_arr + i));
    } else {
      CATCH_REQUIRE(NULL == int_tree_overlap_search(tree,
                                                    (struct int_tree_node*)tree->root,
                                                    search_arr + i));
    }
  } /* for() */
  bstree_destroy(tree);
} /* int_tree_overlap_test() */

void ostree_select_test(int len, struct ds_params *params) {
  struct bstree* tree;
  struct bstree mytree;
  struct element insert_arr[len];

  if (params->flags & DS_APP_DOMAIN_HANDLE) {
    tree = ostree_init(&mytree, params);
  } else {
    tree = ostree_init(NULL, params);
  }
  CATCH_REQUIRE(NULL != tree);

  /*
   * Build tree. Don't bother checking if insertion works correctly--that is
   * tested elsewhere
   */
  for (int i = 0; i < len; ++i) {
    struct element e;
    e.value1 = i;
    CATCH_REQUIRE(OK == ostree_insert(tree, &e.value1, &e));
    insert_arr[i] = e;

    /* verify statistics as we build */
    for (int j = 0; j <= i; ++j) {
      struct ostree_node *node = ostree_select(tree, OSTREE_ROOT(tree), j);
      CATCH_REQUIRE(NULL != node);
      CATCH_REQUIRE(((struct element*)node->data)->value1 ==
                    insert_arr[j].value1);
    } /* for(j..) */
  } /* for(i..) */


  /*
   * Destroy tree. Don't bother checking if deletions work correctly--that is
   * tested elsewhere.
   */
  for (int i = 0; i < len; ++i) {
    CATCH_REQUIRE(ostree_remove(tree, &insert_arr[i].value1) == OK);
    CATCH_REQUIRE(NULL == bstree_data_query(tree, &insert_arr[i].value1));

    /* verify statistics as we delete */
    for (int j = i+1; j < len; ++j) {
      struct ostree_node *node = ostree_select(tree, OSTREE_ROOT(tree),
                                               j - i - 1);
      CATCH_REQUIRE(NULL != node);
      CATCH_REQUIRE(((struct element*)node->data)->value1 ==
                    insert_arr[j].value1);
    } /* for(j..) */
  } /* for(i..) */

  bstree_destroy(tree);
} /* ostree_select_test() */

void ostree_rank_test(int len, struct ds_params *params) {
  struct bstree* tree;
  struct bstree mytree;
  struct element insert_arr[len];

  tree = ostree_init(&mytree, params);
  CATCH_REQUIRE(NULL != tree);

  /*
   * Build tree. Don't bother checking if insertion works correctly--that is
   * tested elsewhere
   */
  for (int i = 0; i < len; ++i) {
    struct element e;
    e.value1 = i;
    e.value2 = 17;
    CATCH_REQUIRE(OK == ostree_insert(tree, &e.value1, &e));
    insert_arr[i] = e;

    /* verify statistics as we build */
    for (int j = 0; j <= i; ++j) {
      struct ostree_node * node = (struct ostree_node*)bstree_node_query(tree,
                                                                         OSTREE_ROOT(tree),
                                                                         &insert_arr[j]);
      CATCH_REQUIRE(NULL != node);
      CATCH_REQUIRE(j == ostree_rank(tree, node));
    } /* for(j..) */
  } /* for(i..) */


  /*
   * Destroy tree. Don't bother checking if deletions work correctly--that is
   * tested elsewhere.
   */
  for (int i = 0; i < len; ++i) {
    CATCH_REQUIRE(ostree_remove(tree, &insert_arr[i].value1) == OK);
    CATCH_REQUIRE(NULL == bstree_data_query(tree, &insert_arr[i].value1));

    /* verify statistics as we delete */
    for (int j = i+1; j < len; ++j) {
      struct ostree_node* node = (struct ostree_node*)bstree_node_query(tree,
                                                                        OSTREE_ROOT(tree),
                                                                        &insert_arr[j].value1);
      CATCH_REQUIRE(NULL != node);
      CATCH_REQUIRE(j - i - 1 == ostree_rank(tree, node));
    } /* for(j..) */
  } /* for(i..) */

  bstree_destroy(tree);
} /* ostree_rank_test() */

/*******************************************************************************
 * Test Helper Functions
 ******************************************************************************/
int verify_nodes_rb(const struct bstree* const tree,
                    struct bstree_node * const node) {
  const uint8_t * node_key = node->key;
  uint8_t *left_key;
  uint8_t *right_key;

  /*
   * Verify root and nil nodes are black (RBTree property #1)
   */
  struct bstree_node* nil = tree->nil;
  int height;
  CATCH_REQUIRE(tree->root->red == 0);
  CATCH_REQUIRE(tree->nil->red == 0);
  CATCH_REQUIRE(tree->root->parent == nil);

  /*
   * Verify children are < current node if a left child, and > current node
   * if a right child
   */
  if (node->left != nil && node->right != nil) {
    left_key = node->left->key;
    right_key = node->right->key;
    CATCH_REQUIRE(th_key_cmp(left_key, node_key) <= 0);
    CATCH_REQUIRE(th_key_cmp(right_key, node_key) > 0);
  } else if (node->left != nil) {
    left_key = node->left->key;
    CATCH_REQUIRE(th_key_cmp(left_key, node_key) <= 0);
  } else if (node->right != nil) {
    right_key = node->right->key;
    CATCH_REQUIRE(th_key_cmp(right_key, node_key) > 0);
  }
  /*
   * Verify that black height of L/R subtrees are equal (RBTree property #2)
   */
  CATCH_REQUIRE(rbtree_node_black_height(node->left) ==
                rbtree_node_black_height(node->right));

  /*
   * Verify if the node is red, both children are black (RBTree property #3)
   */
  CATCH_REQUIRE(!(node->red == 1 && node->left->red == 1));
  CATCH_REQUIRE(!(node->red == 1 && node->right->red == 1));
  return 0;
} /* verify_nodes_rb() */

int verify_nodes_bst(const struct bstree* const tree,
                     struct bstree_node * const node) {
  const uint8_t * node_key = node->key;
  uint8_t* left_key;
  uint8_t* right_key;
  struct bstree_node* nil = tree->nil;

  /*
   * Verify children are < current node if a left child, and > current node
   * if a right child
   */
  if (node->left != nil && node->right != nil) {
    left_key = node->left->key;
    right_key = node->right->key;
    CATCH_REQUIRE(th_key_cmp(left_key, node_key) <= 0);
    CATCH_REQUIRE(th_key_cmp(right_key, node_key) > 0);
  } else if (node->left != nil) {
    left_key = node->left->key;
    CATCH_REQUIRE(th_key_cmp(left_key, node_key) <= 0);
  } else if (node->right != nil) {
    right_key = node->right->key;
    CATCH_REQUIRE(th_key_cmp(right_key, node_key) > 0);
  }
  return 0;
} /* verify_nodes_bst() */

int verify_nodes_int(const struct bstree* const tree,
                     struct int_tree_node * const node) {
  const uint8_t * node_key = node->key;
  uint8_t* left_key;
  uint8_t* right_key;
  struct bstree_node* nil = tree->nil;

  /*
   * Verify auxiliary field
   */

  if (node != (struct int_tree_node*)nil) {
    if (node->right != (struct int_tree_node*)nil) {
      CATCH_REQUIRE(node->max_high >= node->left->max_high);
    }
    if (node->right != (struct int_tree_node*)nil) {
      CATCH_REQUIRE(node->max_high <= node->right->max_high);
    }
    CATCH_REQUIRE(node->max_high >= ((struct interval_data*)node->data)->high);
  }
  return 0;
} /* verify_nodes_os() */

static void test_runner1(uint32_t extra_flags,
                  void (*test)(int len,
                               struct ds_params *params,
                               int (*verify_cb)(const struct bstree* const tree,
                                                struct bstree_node * const node)),
                  int (*verify_cb)(const struct bstree* const tree,
                                   struct bstree_node * const node)) {
  struct ds_params params;
  params.tag = DS_BSTREE;
  params.flags = 0;
  params.cmpe = th_key_cmp;
  params.printe = th_printe;
  params.el_size = sizeof(struct element);
  params.max_elts = NUM_ITEMS;
  th_ds_init(&params);

  dbg_insmod(M_DS_BSTREE, "BSTree");
  /* dbg_mod_lvl_set(M_DS_BSTREE,DBG_V); */

  for (int j = 1; j <= NUM_ITEMS; ++j) {
    DBGN("Testing with %d items\n", j);
    for (int i = 0; i <= 0x100; ++i) {
      params.flags = i | extra_flags;
      test(j, &params, verify_cb);
    } /* for(i..) */
  } /* for(j..) */
  th_ds_shutdown(&params);
} /* test_runner1() */

static void test_runner2(uint32_t extra_flags,
                  void (*test)(int len,
                               int remove_type,
                               struct ds_params *params,
                               int (*verify_cb)(const struct bstree* const tree,
                                                struct bstree_node * const node)),
                  int (*verify_cb)(const struct bstree* const tree,
                                   struct bstree_node * const node)) {
  struct ds_params params;
  params.tag = DS_BSTREE;
  params.flags = 0;
  params.cmpe = th_key_cmp;
  params.printe = th_printe;
  params.el_size = sizeof(struct element);
  params.max_elts = NUM_ITEMS;

  th_ds_init(&params);

  for (int j = 1; j <= NUM_ITEMS; ++j) {
    DBGN("Testing with %d items\n", j);
    for (int i = 0; i <= 0x100; ++i) {
      for (int k = 0; k < 2; ++k) {
        params.flags = i | extra_flags;
        test(j, k, &params, verify_cb);
      } /* for(k..) */
    } /* for(i..) */
  } /* for(j..) */
  th_ds_shutdown(&params);
} /* test_runner2() */

void test_runner3(void (*test)(int len,
                               struct ds_params *params)) {
  struct ds_params params;
  params.tag = DS_BSTREE;
  params.flags = 0;
  params.el_size = sizeof(struct interval_data);
  params.max_elts = NUM_ITEMS;
  params.cmpe = int_tree_cmp_key;
  th_ds_init(&params);

  dbg_insmod(M_DS_BSTREE, "BSTree");
  for (int j = 1; j <= NUM_ITEMS; ++j) {
    DBGN("Testing with %d items\n", j);
    for (int i = 0; i <= 0x100; ++i) {
      params.flags = i | DS_BSTREE_REDBLACK | DS_BSTREE_INTERVAL;
      test(j, &params);
    } /* for(i..) */
  } /* for(j..) */
  th_ds_shutdown(&params);
} /* test_runner3() */

static void test_runner4(void (*test)(int len,
                               struct ds_params *params)) {
  struct ds_params params;
  params.tag = DS_BSTREE;
  params.flags = 0;
  params.el_size = sizeof(struct element);
  params.max_elts = NUM_ITEMS;
  params.cmpe = th_cmpe;

  dbg_insmod(M_DS_BSTREE, "BSTree");

  th_ds_init(&params);
  for (int j = 1; j <= NUM_ITEMS; ++j) {
    DBGN("Testing with %d items\n", j);
    for (int i = 0; i <= 0x100; ++i) {
      params.flags = i | DS_BSTREE_REDBLACK | DS_BSTREE_OS;
      test(j, &params);
    } /* for(i..) */
  } /* for(j..) */
  th_ds_shutdown(&params);
} /* test_runner4() */
