/**
 * \file bstree-test.cpp
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_PREFIX_ALL
#include <catch.hpp>

#include "rcsw/ds/ostree.h"
#include "rcsw/ds/bstree_node.h"
#include "rcsw/utils/utils.h"

#include "tests/ds_test.h"
#include "tests/ds_test.hpp"
#include "tests/ds_bstree_test.hpp"

/*******************************************************************************
 * Namespaces/Decls
 ******************************************************************************/
using ostree_test_t = void(*)(int len,
                              struct bstree_params *params);

/*******************************************************************************
 * Global Variables
 ******************************************************************************/
int n_elements; /* global var for # elements in RBTREE */

/*******************************************************************************
 * Test Helper Functions
 ******************************************************************************/
static void run_test(ostree_test_t test) {
  struct bstree_params params;
  memset(&params, 0, sizeof(bstree_params));
  params.flags = RCSW_DS_BSTREE_RB | RCSW_DS_BSTREE_OS;
  params.elt_size = sizeof(struct element8);
  params.max_elts = TH_NUM_ITEMS;
  params.cmpkey = th::cmpe<element8>;
  th::ds_init(&params);

  uint32_t flags[] = {
    RCSW_NONE,
    RCSW_ZALLOC,
    RCSW_NOALLOC_HANDLE,
    RCSW_NOALLOC_DATA,
    RCSW_NOALLOC_META,
  };
  uint32_t applied = 0;
  for (size_t i = 0; i < RCSW_ARRAY_ELTS(flags); ++i) {
    applied |= flags[i];
    for (size_t j = i + 1; j < RCSW_ARRAY_ELTS(flags); ++j) {
      applied |= flags[j];

      for (int m = 1; m <= TH_NUM_ITEMS; ++m) {
        params.flags |= applied;
        test(m, &params);
      } /* for(m..) */

      applied &= ~flags[j];
    } /* for(j..) */
  } /* for(i..) */

  th::ds_shutdown(&params);
} /* run_test() */

/*******************************************************************************
 * Test Functions
 ******************************************************************************/
/**
 * \brief Test rank for OStrees.
 */
static void ostree_select_test(int len, struct bstree_params *params) {
  struct bstree* tree;
  struct bstree mytree;
  struct element8 insert_arr[TH_NUM_ITEMS];

  if (params->flags & RCSW_NOALLOC_HANDLE) {
    tree = ostree_init(&mytree, params);
  } else {
    tree = ostree_init(NULL, params);
  }
  CATCH_REQUIRE(NULL != tree);

  /*
   * Build tree. Don't bother checking if insertion works correctly--that is
   * tested elsewhere.
   */
  for (int i = 0; i < len; ++i) {
    struct element8 e;
    e.value1 = i;
    CATCH_REQUIRE(OK == ostree_insert(tree, &e.value1, &e));
    insert_arr[i] = e;

    /* verify statistics as we build */
    for (int j = 0; j <= i; ++j) {
      struct ostree_node *node = ostree_select(tree, RCSW_OSTREE_ROOT(tree), j);
      CATCH_REQUIRE(NULL != node);
      CATCH_REQUIRE(((struct element8*)node->data)->value1 ==
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
      struct ostree_node *node = ostree_select(tree,
                                               RCSW_OSTREE_ROOT(tree),
                                               j - i - 1);
      CATCH_REQUIRE(NULL != node);
      CATCH_REQUIRE(((struct element8*)node->data)->value1 ==
                    insert_arr[j].value1);
    } /* for(j..) */
  } /* for(i..) */

  bstree_destroy(tree);
} /* ostree_select_test() */

/**
 * \brief Test select for OStrees.
 */
static void ostree_rank_test(int len, struct bstree_params *params) {
  struct bstree* tree;
  struct bstree mytree;
  struct element8 insert_arr[TH_NUM_ITEMS];

  tree = ostree_init(&mytree, params);
  CATCH_REQUIRE(NULL != tree);

  /*
   * Build tree. Don't bother checking if insertion works correctly--that is
   * tested elsewhere
   */
  for (int i = 0; i < len; ++i) {
    struct element8 e;
    e.value1 = i;
    e.value2 = 17;
    CATCH_REQUIRE(OK == ostree_insert(tree, &e.value1, &e));
    insert_arr[i] = e;

    /* verify statistics as we build */
    for (int j = 0; j <= i; ++j) {
      struct ostree_node * node = ostree_node_query(tree,
                                                    RCSW_OSTREE_ROOT(tree),
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
    for (int j = i + 1; j < len; ++j) {
      struct ostree_node* node = ostree_node_query(tree,
                                                   RCSW_OSTREE_ROOT(tree),
                                                   &insert_arr[j].value1);
      CATCH_REQUIRE(NULL != node);
      CATCH_REQUIRE(j - i - 1 == ostree_rank(tree, node));
    } /* for(j..) */
  } /* for(i..) */

  ostree_destroy(tree);
} /* ostree_rank_test() */

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("Select Test", "[ds][ostree]") {
  run_test(ostree_select_test);
}

CATCH_TEST_CASE("Rank Test", "[ds][ostree]") {
  run_test(ostree_rank_test);
}
