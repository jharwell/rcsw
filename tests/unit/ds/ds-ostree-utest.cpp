/**
 * \file
 *
 * \copyright 2017 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * Unit tests for ostree (order-statistics tree built on top of RB-tree).
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#define CATCH_CONFIG_PREFIX_ALL
#include <catch2/catch_test_macros.hpp>

#include "rcsw/ds/bstree_node.h"
#include "rcsw/ds/ostree.h"
#include "tests/unit/ds/ds_bstree_test.hpp"
#include "tests/unit/ds/ds_test.hpp"

/*******************************************************************************
 * Test Harness
 ******************************************************************************/
static int g_n_elements; /* required by verify callbacks */
using ostree_test_t = void (*)(int len, struct bstree_config* config);

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

/**
 * \brief Verify ostree_select(): after inserting i elements the j-th smallest
 *        (0-indexed) should equal insert_arr[j].
 */
static void select_test(int len, struct bstree_config* config) {
  struct bstree*  tree;
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
      struct ostree_node* node = ostree_select(tree, RCSW_OSTREE_ROOT(tree), j);
      CATCH_REQUIRE(nullptr != node);
      CATCH_REQUIRE(((struct element8*)node->data)->value1 ==
                    insert_arr[j].value1);
    }
  }

  /* Delete in insertion order, verifying statistics shrink correctly */
  for (int i = 0; i < len; ++i) {
    CATCH_REQUIRE(ostree_remove(tree, &insert_arr[i].value1) == OK);
    CATCH_REQUIRE(bstree_data_query(tree, &insert_arr[i].value1) == nullptr);

    /* Remaining elements must still be selectable at adjusted ranks */
    for (int j = i + 1; j < len; ++j) {
      int                 rank = j - i - 1;
      struct ostree_node* node =
        ostree_select(tree, RCSW_OSTREE_ROOT(tree), rank);
      CATCH_REQUIRE(nullptr != node);
      CATCH_REQUIRE(((struct element8*)node->data)->value1 ==
                    insert_arr[j].value1);
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
static void rank_test(int len, struct bstree_config* config) {
  struct bstree*  tree;
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
      struct ostree_node* node =
        ostree_node_query(tree, RCSW_OSTREE_ROOT(tree), &insert_arr[j]);
      CATCH_REQUIRE(nullptr != node);
      CATCH_REQUIRE(ostree_rank(tree, node) == j);
    }
  }

  /* Delete in insertion order; remaining ranks must shift down by 1 each time */
  for (int i = 0; i < len; ++i) {
    CATCH_REQUIRE(ostree_remove(tree, &insert_arr[i].value1) == OK);
    CATCH_REQUIRE(bstree_data_query(tree, &insert_arr[i].value1) == nullptr);

    for (int j = i + 1; j < len; ++j) {
      struct ostree_node* node =
        ostree_node_query(tree, RCSW_OSTREE_ROOT(tree), &insert_arr[j].value1);
      CATCH_REQUIRE(nullptr != node);
      CATCH_REQUIRE(ostree_rank(tree, node) == j - i - 1);
    }
  }

  ostree_destroy(tree);

  CATCH_REQUIRE(th::leak_check_data(config) == 0);
  CATCH_REQUIRE(th::leak_check_nodes(config) == 0);
}
struct bstree* make_ostree() {
  struct bstree_config c;
  memset(&c, 0, sizeof(c));
  c.cmpkey   = th::cmp_int;
  c.elt_size = sizeof(int32_t);
  c.max_elts = -1;
  c.flags    = RCSW_DS_BSTREE_RB | RCSW_DS_BSTREE_OS;
  return ostree_init(nullptr, &c);
}

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("ostree Select Test", "[ds][ostree]") { run_test(select_test); }

CATCH_TEST_CASE("ostree Rank Test", "[ds][ostree]") { run_test(rank_test); }

CATCH_TEST_CASE(
  "ostree subtree counts, select and rank stay correct across "
  "removes",
  "[ds][ostree]") {
  struct bstree*       t = make_ostree();
  std::mt19937         rng(4321);
  std::vector<int32_t> keys;
  for (int32_t i = 0; i < 64; ++i) {
    keys.push_back(i);
  }
  std::shuffle(keys.begin(), keys.end(), rng);
  for (int32_t k : keys) {
    CATCH_REQUIRE(OK == ostree_insert(t, &k, &k));
  }

  std::vector<int32_t> present = keys;
  std::shuffle(keys.begin(), keys.end(), rng);
  for (size_t i = 0; i < keys.size() / 2; ++i) {
    CATCH_REQUIRE(OK == ostree_remove(t, &keys[i]));
    present.erase(std::find(present.begin(), present.end(), keys[i]));
    std::sort(present.begin(), present.end());

    int32_t n;
    CATCH_REQUIRE(th::bst::count_ok(t, RCSW_OSTREE_ROOT(t), &n));
    CATCH_REQUIRE(static_cast<size_t>(n) == present.size());
    for (size_t r = 0; r < present.size(); ++r) {
      struct ostree_node* node =
        ostree_select(t, RCSW_OSTREE_ROOT(t), static_cast<int>(r));
      CATCH_REQUIRE(nullptr != node);
      int32_t key;
      memcpy(&key, node->key, sizeof(key));
      CATCH_REQUIRE(key == present[r]);
      CATCH_REQUIRE(ostree_rank(t, node) == static_cast<int>(r));
    }
  }
  bstree_destroy(t);
}

CATCH_TEST_CASE("ostree_init needs no flags from the caller", "[ds][ostree]") {
  struct bstree_config c;
  memset(&c, 0, sizeof(c));
  c.cmpkey   = th::cmp_int;
  c.elt_size = sizeof(int32_t);
  c.max_elts = -1;
  c.flags    = RCSW_NONE;
  struct bstree* t = ostree_init(nullptr, &c);
  CATCH_REQUIRE(nullptr != t);
  CATCH_REQUIRE(RCSW_NONE == c.flags); /* the caller's config is untouched */
  CATCH_REQUIRE(t->cfg.flags & RCSW_DS_BSTREE_OS);
  CATCH_REQUIRE(t->cfg.flags & RCSW_DS_BSTREE_RB);

  for (int32_t i = 0; i < 50; ++i) {
    int32_t k = ((i * 17) % 50) * 3; /* 0, 3, ..., 147 in scrambled order */
    CATCH_REQUIRE(OK == ostree_insert(t, &k, &k));
  }
  for (int32_t r = 0; r < 50; ++r) {
    struct ostree_node* n = ostree_select(t, RCSW_OSTREE_ROOT(t), r);
    CATCH_REQUIRE(nullptr != n);
    CATCH_REQUIRE(r * 3 == *reinterpret_cast<int32_t*>(n->key));
    CATCH_REQUIRE(r == ostree_rank(t, n));
  }
  ostree_destroy(t);
}

CATCH_TEST_CASE("ostree nodes in caller-provided storage are aligned",
                "[ds][ostree][noalloc]") {
  constexpr size_t     kN = 20;
  std::vector<dptr_t>  meta(ostree_meta_space(kN) / sizeof(dptr_t));
  std::vector<dptr_t>  elements(ostree_element_space(kN, sizeof(int32_t)) /
                               sizeof(dptr_t));
  struct bstree_config c;
  memset(&c, 0, sizeof(c));
  c.cmpkey   = th::cmp_int;
  c.elt_size = sizeof(int32_t);
  c.max_elts = kN;
  c.meta     = meta.data();
  c.elements = elements.data();
  c.flags    = RCSW_NOALLOC_META | RCSW_NOALLOC_DATA;
  struct bstree* t = ostree_init(nullptr, &c);
  CATCH_REQUIRE(nullptr != t);

  for (int32_t i = 0; i < static_cast<int32_t>(kN); ++i) {
    int32_t v = (i * 7) % static_cast<int32_t>(kN);
    CATCH_REQUIRE(OK == ostree_insert(t, &v, &v));
  }
  for (int32_t r = 0; r < static_cast<int32_t>(kN); ++r) {
    struct ostree_node* n = ostree_select(t, RCSW_OSTREE_ROOT(t), r);
    CATCH_REQUIRE(nullptr != n);
    CATCH_REQUIRE(r == *reinterpret_cast<int32_t*>(n->data));
    /* the pointer members are naturally aligned in every node */
    CATCH_REQUIRE(0 == reinterpret_cast<uintptr_t>(&n->left) % alignof(void*));
    CATCH_REQUIRE(0 == reinterpret_cast<uintptr_t>(&n->data) % alignof(void*));
  }
  ostree_destroy(t);
}
