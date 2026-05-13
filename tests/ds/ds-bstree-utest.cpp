/**
 * \file ds-bstree-utest.cpp
 *
 * Unit tests for BSTree (plain BST and RB variant).
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
#include "rcsw/utils/byteops.h"
#include "tests/ds/ds_bstree_test.hpp"
#include "tests/ds/ds_test.hpp"
#include "tests/element.hpp"

/*******************************************************************************
 * Global Variables
 ******************************************************************************/
int n_elements; /* global var required by verify callbacks */

/*******************************************************************************
 * Test Helper Functions
 ******************************************************************************/
static struct bstree_config make_config(uint32_t flags, uint32_t extra) {
  struct bstree_config c;
  memset(&c, 0, sizeof(c));
  c.cmpkey   = th_key_cmp;
  c.printe   = th::printe<element8>; /* overridden per-call when needed */
  c.elt_size = sizeof(element8);
  c.max_elts = TH_NUM_ITEMS;
  c.flags    = flags | extra;
  return c;
}

template <typename T>
static void run_test(uint32_t               extra_flags,
                     th::bst::test_t        test,
                     th::bst::bst_verify_cb verify_cb) {
  struct bstree_config config = make_config(0, extra_flags);
  config.elt_size             = sizeof(T);
  config.printe               = th::printe<T>;
  th::ds_init(&config);

  uint32_t flags[] = {
    RCSW_NONE,
    RCSW_ZALLOC,
    RCSW_NOALLOC_HANDLE,
    RCSW_NOALLOC_DATA,
    RCSW_NOALLOC_META,
  };

  /* Each flag alone */
  for (size_t i = 0; i < RCSW_ARRAY_ELTS(flags); ++i) {
    for (int j = 1; j <= TH_NUM_ITEMS; ++j) {
      config.flags = flags[i] | extra_flags;
      test(j, &config, verify_cb);
    }
  }

  /* Pairwise combinations */
  for (size_t i = 0; i < RCSW_ARRAY_ELTS(flags); ++i) {
    for (size_t j = i + 1; j < RCSW_ARRAY_ELTS(flags); ++j) {
      uint32_t applied = flags[i] | flags[j] | extra_flags;
      for (int m = 1; m <= TH_NUM_ITEMS; ++m) {
        config.flags = applied;
        test(m, &config, verify_cb);
      }
    }
  }

  th::ds_shutdown(&config);
}

template <typename T>
static void run_test_remove(uint32_t               extra_flags,
                            th::bst::rm_test_t     test,
                            th::bst::bst_verify_cb verify_cb) {
  struct bstree_config config = make_config(0, extra_flags);
  config.elt_size             = sizeof(T);
  config.printe               = th::printe<T>;
  th::ds_init(&config);

  uint32_t flags[] = {
    RCSW_NONE,
    RCSW_ZALLOC,
    RCSW_NOALLOC_HANDLE,
    RCSW_NOALLOC_DATA,
    RCSW_NOALLOC_META,
  };

  /* Each flag alone */
  for (size_t i = 0; i < RCSW_ARRAY_ELTS(flags); ++i) {
    for (int m = 1; m <= TH_NUM_ITEMS; ++m) {
      for (int k = 0; k < 2; ++k) {
        config.flags = flags[i] | extra_flags;
        test(m, k, &config, verify_cb);
      }
    }
  }

  /* Pairwise combinations */
  for (size_t i = 0; i < RCSW_ARRAY_ELTS(flags); ++i) {
    for (size_t j = i + 1; j < RCSW_ARRAY_ELTS(flags); ++j) {
      uint32_t applied = flags[i] | flags[j] | extra_flags;
      for (int m = 1; m <= TH_NUM_ITEMS; ++m) {
        for (int k = 0; k < 2; ++k) {
          config.flags = applied;
          test(m, k, &config, verify_cb);
        }
      }
    }
  }

  th::ds_shutdown(&config);
}

/*******************************************************************************
 * Test Functions
 ******************************************************************************/
template <typename T>
static void insert_test(int                    len,
                        struct bstree_config  *config,
                        th::bst::bst_verify_cb verify_cb) {
  struct bstree *tree;
  struct bstree  mytree;

  T   data_arr[TH_NUM_ITEMS];
  int key_arr[TH_NUM_ITEMS];

  /* RCSW_NOALLOC_HANDLE requires a non-NULL handle */
  if (config->flags & RCSW_NOALLOC_HANDLE) {
    CATCH_REQUIRE(nullptr == bstree_init(nullptr, config));
  }
  tree = bstree_init(&mytree, config);
  CATCH_REQUIRE(nullptr != tree);

  /* Empty-state invariants */
  CATCH_REQUIRE(bstree_size(tree) == 0);

                          th::element_generator<T> g(th::gen_elt_type::ekINC_VALS, config->max_elts);

  for (int i = 0; i < len; ++i) {
    int rand_key;
    utils_string_gen((char *)&rand_key, RCSW_BSTREE_NODE_KEYSIZE);
    T  e     = g.next();
    T *e_ptr = nullptr;

    CATCH_REQUIRE(bstree_insert(tree, &rand_key, &e) == OK);
    data_arr[i] = e;
    key_arr[i]  = rand_key;

    /* Query immediately after insert */
    e_ptr = (T *)bstree_data_query(tree, &key_arr[i]);
    CATCH_REQUIRE(nullptr != e_ptr);
    CATCH_REQUIRE(e_ptr->value1 == data_arr[i].value1);
  }

  CATCH_REQUIRE(bstree_size(tree) == (size_t)len);

  /* Verify all data still queryable after full build */
  for (int i = 0; i < len; ++i) {
    T *e_ptr = (T *)bstree_data_query(tree, &key_arr[i]);
    CATCH_REQUIRE(nullptr != e_ptr);
    CATCH_REQUIRE(e_ptr->value1 == data_arr[i].value1);
  }

  /* Structural verification via traversals */
  n_elements = tree->current;
  CATCH_REQUIRE(bstree_traverse(tree, verify_cb, ekTRAVERSE_PREORDER) == OK);
  CATCH_REQUIRE(bstree_traverse(tree, verify_cb, ekTRAVERSE_POSTORDER) == OK);

  bstree_destroy(tree);

  CATCH_REQUIRE(th::leak_check_data(config) == 0);
  CATCH_REQUIRE(th::leak_check_nodes(config) == 0);
}

template <typename T>
static void remove_test(int                    len,
                        int                    remove_type,
                        struct bstree_config  *config,
                        th::bst::bst_verify_cb verify_cb) {
  struct bstree *tree;
  struct bstree  mytree;

  T   data_arr[TH_NUM_ITEMS];
  int key_arr[TH_NUM_ITEMS];

  tree = bstree_init(&mytree, config);
  CATCH_REQUIRE(nullptr != tree);

                          th::element_generator<T> g(th::gen_elt_type::ekRAND_VALS, config->max_elts);

  for (int i = 0; i < len; ++i) {
    int rand_key;
    utils_string_gen((char *)&rand_key, RCSW_BSTREE_NODE_KEYSIZE);
    T e = g.next();
    CATCH_REQUIRE(bstree_insert(tree, &rand_key, &e) == OK);
    data_arr[i] = e;
    key_arr[i]  = rand_key;
  }

  unsigned old_count;
  for (int i = 0; i < len; ++i) {
    int idx = (remove_type == 0) ? i : len - i - 1;

    CATCH_REQUIRE(bstree_data_query(tree, &key_arr[idx]) != nullptr);
    old_count = tree->current;
    CATCH_REQUIRE(bstree_remove(tree, &key_arr[idx]) == OK);
    CATCH_REQUIRE(bstree_data_query(tree, &key_arr[idx]) == nullptr);
    CATCH_REQUIRE(tree->current == old_count - 1);

    /* Verify ordering after each removal (skip the last step: traversing a
     * 0-element tree is a no-op but harmless) */
    n_elements = tree->current;
    if (tree->current > 0) {
      CATCH_REQUIRE(
        bstree_traverse(tree, verify_cb, ekTRAVERSE_INORDER) == OK);
    }
  }

  CATCH_REQUIRE(tree->current == 0);
  bstree_destroy(tree);

  CATCH_REQUIRE(th::leak_check_data(config) == 0);
  CATCH_REQUIRE(th::leak_check_nodes(config) == 0);
}

template <typename T>
static void print_test(int                    len,
                       struct bstree_config  *config,
                       th::bst::bst_verify_cb) {
  int key_arr[TH_NUM_ITEMS];

  /* NULL handle must not crash */
  bstree_print(nullptr);

  /* Force heap allocation so we exercise the print path on a real tree */
  config->flags &= ~RCSW_NOALLOC_HANDLE;
  struct bstree *tree = bstree_init(nullptr, config);
  CATCH_REQUIRE(nullptr != tree);

  bstree_print(tree); /* empty */

  th::element_generator<T> g(th::gen_elt_type::ekINC_VALS, config->max_elts);
  for (int i = 0; i < len; ++i) {
    int rand_key;
    utils_string_gen((char *)&rand_key, RCSW_BSTREE_NODE_KEYSIZE);
    T e = g.next();
    CATCH_REQUIRE(bstree_insert(tree, &rand_key, &e) == OK);
    key_arr[i] = rand_key;
  }

  bstree_print(tree); /* populated */
  bstree_destroy(tree);
}

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("bstree Insert Test", "[ds][bstree]") {
  run_test<element1>(0, insert_test<element1>, th::bst::verify_nodes_bst);
  run_test<element2>(0, insert_test<element2>, th::bst::verify_nodes_bst);
  run_test<element4>(0, insert_test<element4>, th::bst::verify_nodes_bst);
  run_test<element8>(0, insert_test<element8>, th::bst::verify_nodes_bst);
}

CATCH_TEST_CASE("bstree Remove Test", "[ds][bstree]") {
  run_test_remove<element1>(0, remove_test<element1>, th::bst::verify_nodes_bst);
  run_test_remove<element2>(0, remove_test<element2>, th::bst::verify_nodes_bst);
  run_test_remove<element4>(0, remove_test<element4>, th::bst::verify_nodes_bst);
  run_test_remove<element8>(0, remove_test<element8>, th::bst::verify_nodes_bst);
}

CATCH_TEST_CASE("bstree Print Test", "[ds][bstree]") {
  run_test<element8>(0, print_test<element8>, th::bst::verify_nodes_bst);
}

/* Red-Black Tree tests */
CATCH_TEST_CASE("rbtree Insert Test", "[ds][rbtree]") {
  run_test<element1>(RCSW_DS_BSTREE_RB, insert_test<element1>, th::bst::verify_nodes_rb);
  run_test<element2>(RCSW_DS_BSTREE_RB, insert_test<element2>, th::bst::verify_nodes_rb);
  run_test<element4>(RCSW_DS_BSTREE_RB, insert_test<element4>, th::bst::verify_nodes_rb);
  run_test<element8>(RCSW_DS_BSTREE_RB, insert_test<element8>, th::bst::verify_nodes_rb);
}

CATCH_TEST_CASE("rbtree Remove Test", "[ds][rbtree]") {
  run_test_remove<element1>(RCSW_DS_BSTREE_RB, remove_test<element1>, th::bst::verify_nodes_rb);
  run_test_remove<element2>(RCSW_DS_BSTREE_RB, remove_test<element2>, th::bst::verify_nodes_rb);
  run_test_remove<element4>(RCSW_DS_BSTREE_RB, remove_test<element4>, th::bst::verify_nodes_rb);
  run_test_remove<element8>(RCSW_DS_BSTREE_RB, remove_test<element8>, th::bst::verify_nodes_rb);
}
