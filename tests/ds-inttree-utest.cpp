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

#include "rcsw/ds/rbtree.h"
#include "rcsw/ds/inttree.h"
#include "rcsw/ds/ostree.h"
#include "rcsw/ds/bstree_node.h"
#include "rcsw/utils/utils.h"

#include "tests/ds_test.h"
#include "tests/ds_test.hpp"
#include "tests/ds_bstree_test.hpp"

/*******************************************************************************
 * Namespaces/Decls
 ******************************************************************************/
using inttree_test_t = void(*)(int len,
                               struct bstree_params *params);

/*******************************************************************************
 * Global Variables
 ******************************************************************************/
int n_elements; /* global var for # elements in RBTREE */

/*******************************************************************************
 * Test Helper Functions
 ******************************************************************************/
static void run_test(inttree_test_t test) {
  struct bstree_params params;
  memset(&params, 0, sizeof(bstree_params));
  params.flags = 0;
  params.elt_size = sizeof(struct interval_data);
  params.max_elts = TH_NUM_ITEMS;
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
        params.flags = applied | RCSW_DS_BSTREE_RB | RCSW_DS_BSTREE_INT;
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
 * \brief Test inserting nodes in an interval tree and verifying
 */
static void inttree_insert_test(int len, struct bstree_params *params) {
  struct bstree* tree;
  struct bstree mytree;
  struct interval_data arr1[TH_NUM_ITEMS];

  tree = inttree_init(&mytree, params);
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

    CATCH_REQUIRE(OK == inttree_insert(tree, &e));
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
} /* inttree_insert_test() */

/**
 * \brief Test removeing nodes in an interval tree and verifying
 */
static void inttree_remove_test(int len, struct bstree_params *params) {
  struct bstree* tree;
  struct bstree mytree;
  struct interval_data arr1[TH_NUM_ITEMS];

  tree = inttree_init(&mytree, params);
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
    CATCH_REQUIRE(OK == inttree_insert(tree, &e));
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
                                  (th::bst::bst_verify_cb)th::bst::verify_nodes_int,
                                  ekTRAVERSE_INORDER) == OK);
  } /* for() */

  bstree_destroy(tree);
} /* inttree_remove_test() */

/**
 * \brief Test overlap search for interval trees
 */
static void inttree_overlap_test(int len, struct bstree_params *params) {
  struct bstree* tree;
  struct bstree mytree;
  struct interval_data insert_arr[TH_NUM_ITEMS];
  struct interval_data search_arr[TH_NUM_ITEMS];
  bool overlap_arr[TH_NUM_ITEMS];

  if (params->flags & RCSW_NOALLOC_HANDLE) {
    tree = inttree_init(&mytree, params);
  } else {
    tree = inttree_init(NULL, params);
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
    CATCH_REQUIRE(OK == inttree_insert(tree, &e));
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
      CATCH_REQUIRE(NULL != inttree_overlap_search(tree,
                                                   RCSW_INTTREE_ROOT(tree),
                                                   search_arr + i));
    } else {
      CATCH_REQUIRE(NULL == inttree_overlap_search(tree,
                                                   RCSW_INTTREE_ROOT(tree),
                                                   search_arr + i));
    }
  } /* for() */
  bstree_destroy(tree);
} /* inttree_overlap_test() */

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
/* Internal Tree tests */
CATCH_TEST_CASE("Insert Test", "[ds][inttree]") {
  run_test(inttree_insert_test);
}
CATCH_TEST_CASE("Remove Test", "[ds][inttree]") {
  run_test(inttree_remove_test);
}
CATCH_TEST_CASE("Overlap Test", "[ds][inttree]") {
  run_test(inttree_overlap_test);
}
