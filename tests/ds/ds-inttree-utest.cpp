/**
 * \file ds-inttree-utest.cpp
 *
 * Unit tests for inttree (interval tree built on top of RB-tree).
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
#include "rcsw/ds/inttree.h"
#include "rcsw/ds/rbtree.h"
#include "tests/ds/ds_bstree_test.hpp"
#include "tests/ds/ds_test.hpp"

/*******************************************************************************
 * Global Variables
 ******************************************************************************/
int n_elements; /* required by verify callbacks */

/*******************************************************************************
 * Test Helper Functions
 ******************************************************************************/
using inttree_test_t = void (*)(int len, struct bstree_config *config);

static void run_test(inttree_test_t test) {
  struct bstree_config config;
  memset(&config, 0, sizeof(bstree_config));
  config.elt_size = sizeof(struct interval_data);
  config.max_elts = TH_NUM_ITEMS;
  th::ds_init(&config);

  uint32_t flags[] = {
    RCSW_NONE,
    RCSW_ZALLOC,
    RCSW_NOALLOC_HANDLE,
    RCSW_NOALLOC_DATA,
    RCSW_NOALLOC_META,
  };

  /* Each flag in isolation */
  for (size_t i = 0; i < RCSW_ARRAY_ELTS(flags); ++i) {
    for (int m = 1; m <= TH_NUM_ITEMS; ++m) {
      config.flags = flags[i] | RCSW_DS_BSTREE_RB | RCSW_DS_BSTREE_INT;
      test(m, &config);
    }
  }

  /* Pairwise combinations */
  for (size_t i = 0; i < RCSW_ARRAY_ELTS(flags); ++i) {
    for (size_t j = i + 1; j < RCSW_ARRAY_ELTS(flags); ++j) {
      uint32_t applied = flags[i] | flags[j] | RCSW_DS_BSTREE_RB |
                         RCSW_DS_BSTREE_INT;
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
static void insert_test(int len, struct bstree_config *config) {
  struct bstree       *tree;
  struct bstree        mytree;
  struct interval_data arr[TH_NUM_ITEMS];

  if (config->flags & RCSW_NOALLOC_HANDLE) {
    CATCH_REQUIRE(nullptr == inttree_init(nullptr, config));
  }
  tree = inttree_init(&mytree, config);
  CATCH_REQUIRE(nullptr != tree);

  /* Empty-state invariant */
  CATCH_REQUIRE(bstree_size(tree) == 0);

  /* Build and verify auxiliary-field updates after each insertion */
  for (int i = 0; i < len; ++i) {
    struct interval_data e;
    e.low  = i;
    e.high = i + 10;

    CATCH_REQUIRE(inttree_insert(tree, &e) == OK);
    arr[i] = e;

    struct interval_data *e_ptr =
      (interval_data *)bstree_data_query(tree, &arr[i].low);
    CATCH_REQUIRE(nullptr != e_ptr);
    CATCH_REQUIRE(e_ptr->low == arr[i].low);
    CATCH_REQUIRE(e_ptr->high == arr[i].high);
  }

  /* Verify all intervals still queryable after full build */
  for (int i = 0; i < len; ++i) {
    struct interval_data *e_ptr =
      (interval_data *)bstree_data_query(tree, &arr[i].low);
    CATCH_REQUIRE(nullptr != e_ptr);
    CATCH_REQUIRE(e_ptr->low == arr[i].low);
    CATCH_REQUIRE(e_ptr->high == arr[i].high);
  }

  bstree_destroy(tree);

  CATCH_REQUIRE(th::leak_check_data(config) == 0);
  CATCH_REQUIRE(th::leak_check_nodes(config) == 0);
}

static void remove_test(int len, struct bstree_config *config) {
  struct bstree       *tree;
  struct bstree        mytree;
  struct interval_data arr[TH_NUM_ITEMS];

  tree = inttree_init(&mytree, config);
  CATCH_REQUIRE(nullptr != tree);

  for (int i = 0; i < len; ++i) {
    struct interval_data e;
    e.low  = i;
    e.high = i + 10;
    CATCH_REQUIRE(inttree_insert(tree, &e) == OK);
    arr[i] = e;
  }

  unsigned old_count;
  for (int i = 0; i < len; ++i) {
    CATCH_REQUIRE(bstree_data_query(tree, &arr[i].low) != nullptr);
    old_count = tree->current;
    CATCH_REQUIRE(bstree_remove(tree, &arr[i].low) == OK);
    CATCH_REQUIRE(bstree_data_query(tree, &arr[i].low) == nullptr);
    CATCH_REQUIRE(tree->current == old_count - 1);

    n_elements = tree->current;
    if (tree->current > 0) {
      CATCH_REQUIRE(
        bstree_traverse(tree,
                        reinterpret_cast<th::bst::bst_verify_cb>(
                          th::bst::verify_nodes_int),
                        ekTRAVERSE_INORDER) == OK);
    }
  }

  bstree_destroy(tree);

  CATCH_REQUIRE(th::leak_check_data(config) == 0);
  CATCH_REQUIRE(th::leak_check_nodes(config) == 0);
}

/**
 * \brief Build a tree with known non-overlapping and overlapping intervals,
 * then verify inttree_overlap_search() returns the correct result for each
 * query.
 */
static void overlap_test(int len, struct bstree_config *config) {
  struct bstree       *tree;
  struct bstree        mytree;
  struct interval_data insert_arr[TH_NUM_ITEMS];
  struct interval_data search_arr[TH_NUM_ITEMS];
  bool                 overlap_arr[TH_NUM_ITEMS];

  if (config->flags & RCSW_NOALLOC_HANDLE) {
    tree = inttree_init(&mytree, config);
  } else {
    tree = inttree_init(nullptr, config);
  }
  CATCH_REQUIRE(nullptr != tree);

  /* Build with spaced intervals: [0,0], [4,9], [8,20], ... */
  for (int i = 0; i < len; ++i) {
    struct interval_data e;
    e.low  = i * 4;
    e.high = e.low + i * 5;
    CATCH_REQUIRE(inttree_insert(tree, &e) == OK);
    insert_arr[i] = e;
  }

  /* Build query intervals and pre-compute expected overlaps */
  for (int i = 0; i < len; ++i) {
    search_arr[i].low  = rand() % 4;
    search_arr[i].high = search_arr[i].low + rand() % 5;
    overlap_arr[i]     = false;
    for (int j = 0; j < len; ++j) {
      if (search_arr[i].low < insert_arr[j].low &&
          search_arr[i].high >= insert_arr[j].low) {
        overlap_arr[i] = true;
      }
      if (search_arr[i].low >= insert_arr[j].low &&
          search_arr[i].low <= insert_arr[j].high) {
        overlap_arr[i] = true;
      }
    }
  }

  /* Verify overlap search matches pre-computed expectation */
  for (int i = 0; i < len; ++i) {
    struct interval_data *result = (interval_data *)inttree_overlap_search(
      tree, RCSW_INTTREE_ROOT(tree), &search_arr[i]);

    if (overlap_arr[i]) {
      CATCH_REQUIRE(result != nullptr);
    } else {
      CATCH_REQUIRE(result == nullptr);
    }
  }

  bstree_destroy(tree);
}

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("inttree Insert Test", "[ds][inttree]") {
  run_test(insert_test);
}

CATCH_TEST_CASE("inttree Remove Test", "[ds][inttree]") {
  run_test(remove_test);
}

CATCH_TEST_CASE("inttree Overlap Test", "[ds][inttree]") {
  run_test(overlap_test);
}
