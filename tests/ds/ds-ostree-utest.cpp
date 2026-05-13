/**
 * \file ds-ostree-utest.cpp
 *
 * Unit tests for ostree (order-statistics tree built on top of RB-tree).
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#define CATCH_CONFIG_PREFIX_ALL
#include <catch2/catch_test_macros.hpp>

#include "rcsw/ds/bstree_node.h"
#include "rcsw/ds/ostree.h"
#include "tests/ds/ds_bstree_test.hpp"
#include "tests/ds/ds_test.hpp"

/*******************************************************************************
 * Global Variables
 ******************************************************************************/
int n_elements; /* required by verify callbacks */

/*******************************************************************************
 * Test Helper Functions
 ******************************************************************************/
using ostree_test_t = void (*)(int len, struct bstree_config *config);

static void run_test(ostree_test_t test) {
  struct bstree_config config;
  memset(&config, 0, sizeof(bstree_config));
  /* ostree always uses element8; cmpkey compares by value1 */
  config.elt_size = sizeof(struct element8);
  config.max_elts = TH_NUM_ITEMS;
  config.cmpkey   = th::cmpe<element8>;
  th::ds_init(&config);

  uint32_t flags[] = {
    RCSW_NONE,
    RCSW_ZALLOC,
    RCSW_NOALLOC_HANDLE,
    RCSW_NOALLOC_DATA,
    RCSW_NOALLOC_META,
  };

  const uint32_t base = RCSW_DS_BSTREE_RB | RCSW_DS_BSTREE_OS;

  /* Each flag in isolation */
  for (size_t i = 0; i < RCSW_ARRAY_ELTS(flags); ++i) {
    for (int m = 1; m <= TH_NUM_ITEMS; ++m) {
      config.flags = base | flags[i]; /* assign, not |=, to avoid accumulation */
      test(m, &config);
    }
  }

  /* Pairwise combinations */
  for (size_t i = 0; i < RCSW_ARRAY_ELTS(flags); ++i) {
    for (size_t j = i + 1; j < RCSW_ARRAY_ELTS(flags); ++j) {
      uint32_t applied = base | flags[i] | flags[j];
      for (int m = 1; m <= TH_NUM_ITEMS; ++m) {
        config.flags = applied;
        test(m, &config);
      }
    }
  }

  th::ds_shutdown(&config);
}

/*******************************************************************************
 * Test Functions
 ******************************************************************************/
/**
 * \brief Verify ostree_select(): after inserting i elements the j-th smallest
 *        (0-indexed) should equal insert_arr[j].
 */
static void select_test(int len, struct bstree_config *config) {
  struct bstree  *tree;
  struct bstree   mytree;
  struct element8 insert_arr[TH_NUM_ITEMS];

  if (config->flags & RCSW_NOALLOC_HANDLE) {
    tree = ostree_init(&mytree, config);
  } else {
    tree = ostree_init(nullptr, config);
  }
  CATCH_REQUIRE(nullptr != tree);

  /* Build with monotonically increasing keys so rank == insertion index */
  for (int i = 0; i < len; ++i) {
    struct element8 e;
    e.value1 = i;
    e.value2 = 0;
    CATCH_REQUIRE(ostree_insert(tree, &e.value1, &e) == OK);
    insert_arr[i] = e;

    /* After each insert, all previously inserted elements must be selectable */
    for (int j = 0; j <= i; ++j) {
      struct ostree_node *node =
        ostree_select(tree, RCSW_OSTREE_ROOT(tree), j);
      CATCH_REQUIRE(nullptr != node);
      CATCH_REQUIRE(
        ((struct element8 *)node->data)->value1 == insert_arr[j].value1);
    }
  }

  /* Delete in insertion order, verifying statistics shrink correctly */
  for (int i = 0; i < len; ++i) {
    CATCH_REQUIRE(ostree_remove(tree, &insert_arr[i].value1) == OK);
    CATCH_REQUIRE(
      bstree_data_query(tree, &insert_arr[i].value1) == nullptr);

    /* Remaining elements must still be selectable at adjusted ranks */
    for (int j = i + 1; j < len; ++j) {
      int rank = j - i - 1;
      struct ostree_node *node =
        ostree_select(tree, RCSW_OSTREE_ROOT(tree), rank);
      CATCH_REQUIRE(nullptr != node);
      CATCH_REQUIRE(
        ((struct element8 *)node->data)->value1 == insert_arr[j].value1);
    }
  }

  bstree_destroy(tree);

  CATCH_REQUIRE(th::leak_check_data(config) == 0);
  CATCH_REQUIRE(th::leak_check_nodes(config) == 0);
}

/**
 * \brief Verify ostree_rank(): after inserting i elements the rank of element
 *        j should equal j (0-indexed).
 */
static void rank_test(int len, struct bstree_config *config) {
  struct bstree  *tree;
  struct bstree   mytree;
  struct element8 insert_arr[TH_NUM_ITEMS];

  tree = ostree_init(&mytree, config);
  CATCH_REQUIRE(nullptr != tree);

  for (int i = 0; i < len; ++i) {
    struct element8 e;
    e.value1 = i;
    e.value2 = 17;
    CATCH_REQUIRE(ostree_insert(tree, &e.value1, &e) == OK);
    insert_arr[i] = e;

    /* Verify rank of every inserted element after each insertion */
    for (int j = 0; j <= i; ++j) {
      struct ostree_node *node = ostree_node_query(
        tree, RCSW_OSTREE_ROOT(tree), &insert_arr[j]);
      CATCH_REQUIRE(nullptr != node);
      CATCH_REQUIRE(ostree_rank(tree, node) == j);
    }
  }

  /* Delete in insertion order; remaining ranks must shift down by 1 each time */
  for (int i = 0; i < len; ++i) {
    CATCH_REQUIRE(ostree_remove(tree, &insert_arr[i].value1) == OK);
    CATCH_REQUIRE(
      bstree_data_query(tree, &insert_arr[i].value1) == nullptr);

    for (int j = i + 1; j < len; ++j) {
      struct ostree_node *node = ostree_node_query(
        tree, RCSW_OSTREE_ROOT(tree), &insert_arr[j].value1);
      CATCH_REQUIRE(nullptr != node);
      CATCH_REQUIRE(ostree_rank(tree, node) == j - i - 1);
    }
  }

  ostree_destroy(tree);

  CATCH_REQUIRE(th::leak_check_data(config) == 0);
  CATCH_REQUIRE(th::leak_check_nodes(config) == 0);
}

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("ostree Select Test", "[ds][ostree]") {
  run_test(select_test);
}

CATCH_TEST_CASE("ostree Rank Test", "[ds][ostree]") {
  run_test(rank_test);
}
