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
#define CATCH_CONFIG_PREFIX_ALL
#include <catch2/catch_test_macros.hpp>

#include "rcsw/ds/bstree.h"
#include "rcsw/ds/bstree_node.h"
#include "rcsw/ds/rbtree.h"
#include "tests/ds/ds_bstree_test.hpp"
#include "tests/ds/ds_test.hpp"
#include "rcsw/utils/byteops.h"

/*******************************************************************************
 * Global Variables
 ******************************************************************************/
int n_elements; /* global var for # elements in RBTREE */

/*******************************************************************************
 * Test Helper Functions
 ******************************************************************************/
template <typename T>
static void run_test(uint32_t               extra_flags,
                     th::bst::test_t        test,
                     th::bst::bst_verify_cb verify_cb) {
  struct bstree_config config;
  memset(&config, 0, sizeof(bstree_config));
  config.flags    = 0;
  config.cmpkey   = th_key_cmp;
  config.printe   = th::printe<T>;
  config.elt_size = sizeof(T);
  config.max_elts = TH_NUM_ITEMS;
  th::ds_init(&config);

  /* dbg_insmod(M_DS_BSTREE, "BSTree"); */
  /* dbg_mod_lvl_set(M_DS_BSTREE,DBG_V); */

  uint32_t flags[] = {
    RCSW_NONE,
    RCSW_NOALLOC_HANDLE,
    RCSW_NOALLOC_DATA,
    RCSW_NOALLOC_META,
  };
  for (int j = 1; j <= TH_NUM_ITEMS; ++j) {
    /* DBGN("Testing with %d items\n", j); */
    for (size_t i = 0; i < RCSW_ARRAY_ELTS(flags); ++i) {
      config.flags = flags[i] | extra_flags;
      test(j, &config, verify_cb);
    } /* for(i..) */
  } /* for(j..) */
  th::ds_shutdown(&config);
} /* run_test() */

template <typename T>
static void run_test_remove(uint32_t               extra_flags,
                            th::bst::rm_test_t     test,
                            th::bst::bst_verify_cb verify_cb) {
  struct bstree_config config;
  memset(&config, 0, sizeof(bstree_config));
  config.flags    = 0;
  config.cmpkey   = th_key_cmp;
  config.printe   = th::printe<T>;
  config.elt_size = sizeof(T);
  config.max_elts = TH_NUM_ITEMS;
  th::ds_init(&config);

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
        for (int k = 0; k < 2; ++k) {
          config.flags = applied | extra_flags;
          test(m, k, &config, verify_cb);
        } /* for(k..) */
      } /* for(m..) */
      applied &= ~flags[j];
    } /* for(j..) */
  } /* for(i..) */

  th::ds_shutdown(&config);
} /* run_test_remove() */

/*******************************************************************************
 * Test Functions
 ******************************************************************************/
/**
 * \brief Test inserting nodes in a binary search tree and verifying
 *
 * This routine creates 3 BSTREEs of sorted, reverse-sorted, and random
 * data of the specified size and verifies the integrity of the BSTREE
 * after each insertion.
 */
template <typename T>
static void insert_test(int                    len,
                        struct bstree_config  *config,
                        th::bst::bst_verify_cb verify_cb) {
  struct bstree *tree;
  struct bstree  mytree;

  T   data_arr[TH_NUM_ITEMS];
  int key_arr[TH_NUM_ITEMS];

  if (config->flags & RCSW_NOALLOC_HANDLE) {
    CATCH_REQUIRE(nullptr == bstree_init(nullptr, config));
  }
  tree = bstree_init(&mytree, config);
  CATCH_REQUIRE(nullptr != tree);

  /* Build tree with random keys and verify insertions */
  th::element_generator<T> g(gen_elt_type::ekINC_VALS, config->max_elts);

  for (int i = 0; i < len; ++i) {
    int rand_key;
    utils_string_gen((char *)&rand_key, RCSW_BSTREE_NODE_KEYSIZE);
    T  e = g.next();
    T *e_ptr;
    CATCH_REQUIRE(OK == bstree_insert(tree, &rand_key, &e));
    data_arr[i] = e;
    key_arr[i]  = rand_key;
    e_ptr       = (T *)bstree_data_query(tree, &key_arr[i]);
    CATCH_REQUIRE(nullptr != e_ptr);
    CATCH_REQUIRE(e_ptr->value1 == data_arr[i].value1);
  } /* for() */

  /* verify node count */
  CATCH_REQUIRE(bstree_size(tree) == (size_t)len);

  /* verify all data in the trees */
  for (int i = 0; i < len; ++i) {
    T *e_ptr;
    e_ptr = (T *)bstree_data_query(tree, &key_arr[i]);
    CATCH_REQUIRE(e_ptr != nullptr);
    CATCH_REQUIRE(e_ptr->value1 == data_arr[i].value1);
  } /* for() */

  /* verify BSTREE structure */
  n_elements = tree->current;
  CATCH_REQUIRE(bstree_traverse(tree, verify_cb, ekTRAVERSE_PREORDER) == OK);
  CATCH_REQUIRE(bstree_traverse(tree, verify_cb, ekTRAVERSE_POSTORDER) == OK);

  bstree_destroy(tree);

  /* verify all DS_APP_DOMAIN data deallocated */
  CATCH_REQUIRE(th::leak_check_data(config) == 0);
  CATCH_REQUIRE(th::leak_check_nodes(config) == 0);
} /* insert_test() */

/**
 * \brief Test printing the tree.
 */
template <typename T>
static void print_test(int                   len,
                       struct bstree_config *config,
                       th::bst::bst_verify_cb) {
  struct bstree *tree;

  T   data_arr[TH_NUM_ITEMS];
  int key_arr[TH_NUM_ITEMS];

  bstree_print(nullptr);
  config->flags &= ~RCSW_NOALLOC_HANDLE;
  tree = bstree_init(nullptr, config);
  CATCH_REQUIRE(nullptr != tree);
  bstree_print(tree);

  /* Build tree with random keys and verify insertions */
  th::element_generator<T> g(gen_elt_type::ekINC_VALS, config->max_elts);

  for (int i = 0; i < len; ++i) {
    int rand_key;
    utils_string_gen((char *)&rand_key, RCSW_BSTREE_NODE_KEYSIZE);
    T  e = g.next();
    T *e_ptr;
    CATCH_REQUIRE(OK == bstree_insert(tree, &rand_key, &e));
    data_arr[i] = e;
    key_arr[i]  = rand_key;
    e_ptr       = (T *)bstree_data_query(tree, &key_arr[i]);
    CATCH_REQUIRE(nullptr != e_ptr);
    CATCH_REQUIRE(e_ptr->value1 == data_arr[i].value1);
  } /* for() */

  bstree_print(tree);
  bstree_destroy(tree);
} /* print_test() */

/**
 * \brief Test removing elements from BSTREEs of different sizes
 *
 * This routine creates 3 BSTREEs of sorted, reverse-sorted, and random
 * data of the specified size and tests removing elements from them in both the
 * order they were inserted, and reverse- insert order.
 */
template <typename T>
static void remove_test(int                    len,
                        int                    remove_type,
                        struct bstree_config  *config,
                        th::bst::bst_verify_cb verify_cb) {
  struct bstree *tree;
  struct bstree  mytree;

  struct bstree_node arr1[TH_NUM_ITEMS];
  T                  data_arr[TH_NUM_ITEMS];
  int                key_arr[TH_NUM_ITEMS];

  tree = bstree_init(&mytree, config);
  CATCH_REQUIRE((nullptr != tree));

  /*
   * Build tree with random keys (don't need to verify insertions--that is done
   * elsewhere)
   */
  th::element_generator<T> g(gen_elt_type::ekRAND_VALS, config->max_elts);
  for (int i = 0; i < len; ++i) {
    int rand_key;
    utils_string_gen((char *)&rand_key, RCSW_BSTREE_NODE_KEYSIZE);
    T e = g.next();
    CATCH_REQUIRE(OK == bstree_insert(tree, &rand_key, &e));
    data_arr[i] = e;
    key_arr[i]  = rand_key;
  } /* for() */

  /* test removing random elements until the tree is empty */
  unsigned old_count;
  for (int i = 0; i < len; ++i) {
    int remove_index = (remove_type == 0) ? i : len - i - 1;

    /* tree 1 */
    CATCH_REQUIRE(bstree_data_query(tree, &key_arr[remove_index]) != nullptr);
    old_count = tree->current;
    CATCH_REQUIRE(bstree_remove(tree, &key_arr[remove_index]) == OK);
    CATCH_REQUIRE(bstree_data_query(tree, &key_arr[remove_index]) == nullptr);
    CATCH_REQUIRE(tree->current == old_count - 1);

    /* Verify relative ordering of the nodes in the trees after
     * removal. Need the len - 1 so that I don't try to traverse a tree when
     * there is only a single node in it.
     */
    if (i >= len - 1) {
      continue;
    }
    n_elements = tree->current;
    CATCH_REQUIRE(bstree_traverse(tree, verify_cb, ekTRAVERSE_INORDER) == OK);
  } /* for() */

  /* check the trees are now empty */
  CATCH_REQUIRE(tree->current == 0);

  bstree_destroy(tree);

  /* verify all APP_DOMAIN data deallocated */
  CATCH_REQUIRE(th::leak_check_data(config) == 0);
  CATCH_REQUIRE(th::leak_check_nodes(config) == 0);
} /* remove_test() */

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
/* Binary Search Tree tests */
CATCH_TEST_CASE("Insert Test", "[ds][bstree]") {
  run_test<element1>(0, insert_test<element1>, th::bst::verify_nodes_bst);
  run_test<element2>(0, insert_test<element2>, th::bst::verify_nodes_bst);
  run_test<element4>(0, insert_test<element4>, th::bst::verify_nodes_bst);
  run_test<element8>(0, insert_test<element8>, th::bst::verify_nodes_bst);
}

CATCH_TEST_CASE("Remove Test", "[ds][bstree]") {
  run_test_remove<element1>(0, remove_test<element1>, th::bst::verify_nodes_bst);
  run_test_remove<element2>(0, remove_test<element2>, th::bst::verify_nodes_bst);
  run_test_remove<element4>(0, remove_test<element4>, th::bst::verify_nodes_bst);
  run_test_remove<element8>(0, remove_test<element8>, th::bst::verify_nodes_bst);
}

CATCH_TEST_CASE("Print Test", "[ds][bstree]") {
  run_test<element8>(RCSW_DS_BSTREE_RB,
                     print_test<element8>,
                     th::bst::verify_nodes_bst);
}

/* Red-black Tree tests */
CATCH_TEST_CASE("RB Insert Test", "[ds][rbtree]") {
  run_test<element1>(RCSW_DS_BSTREE_RB,
                     insert_test<element1>,
                     th::bst::verify_nodes_rb);
  run_test<element2>(RCSW_DS_BSTREE_RB,
                     insert_test<element2>,
                     th::bst::verify_nodes_rb);
  run_test<element4>(RCSW_DS_BSTREE_RB,
                     insert_test<element4>,
                     th::bst::verify_nodes_rb);
  run_test<element8>(RCSW_DS_BSTREE_RB,
                     insert_test<element8>,
                     th::bst::verify_nodes_rb);
}
