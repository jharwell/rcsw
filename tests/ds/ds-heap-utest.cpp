/**
 * \file ds-heap-utest.cpp
 *
 * Unit tests for binheap (max-heap and min-heap variants).
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <limits.h>
#define CATCH_CONFIG_PREFIX_ALL
#include <catch2/catch_test_macros.hpp>

#include "rcsw/ds/binheap.h"
#include "rcsw/utils/mem.h"
#include "tests/ds/ds_test.hpp"
#include "tests/element.hpp"

/*******************************************************************************
 * Test Helper Functions
 ******************************************************************************/
template <typename T>
static void run_test(void (*test)(th::gen_elt_type, struct binheap_config *),
                     th::gen_elt_type type) {
  RCSW_ER_INIT(TH_ZLOG_CONF);

  struct binheap_config config;
  memset(&config, 0, sizeof(binheap_config));
  config.cmpe     = th::cmpe<T>;
  config.printe   = th::printe<T>;
  config.elt_size = sizeof(T);
  CATCH_REQUIRE(th::ds_init(&config) == OK);

  uint32_t flags[] = {
    RCSW_NONE,
    RCSW_ZALLOC,
    RCSW_NOALLOC_HANDLE,
    RCSW_NOALLOC_DATA,
    RCSW_DS_BINHEAP_MIN,
  };

  /* Each flag in isolation */
  for (size_t i = 0; i < RCSW_ARRAY_ELTS(flags); ++i) {
    for (int m = 1; m < TH_NUM_ITEMS; ++m) {
      config.flags     = flags[i];
      config.max_elts  = m;
      config.init_size = 0;
      test(type, &config);
    }
  }

  /* Pairwise combinations */
  for (size_t i = 0; i < RCSW_ARRAY_ELTS(flags); ++i) {
    for (size_t j = i + 1; j < RCSW_ARRAY_ELTS(flags); ++j) {
      uint32_t applied = flags[i] | flags[j];
      for (int m = 1; m < TH_NUM_ITEMS; ++m) {
        config.flags     = applied;
        config.max_elts  = m;
        config.init_size = 0;
        test(type, &config);
      }
    }
  }

  th::ds_shutdown(&config);
  RCSW_ER_DEINIT();
}

/**
 * \brief Verify the heap invariant across all nodes.
 *
 * For a max-heap: parent >= both children.
 * For a min-heap: parent <= both children.
 */
template <typename T>
static void verify_heap(struct binheap *heap) {
  size_t i = 0;
  while (++i <= binheap_size(heap)) {
    T *parent = (T *)darray_data_get(&heap->arr, i);

    if (RCSW_BINHEAP_LCHILD(i) <= binheap_size(heap)) {
      T *lc = (T *)darray_data_get(&heap->arr, RCSW_BINHEAP_LCHILD(i));
      if (heap->flags & RCSW_DS_BINHEAP_MIN) {
        CATCH_REQUIRE(heap->arr.cmpe(parent, lc) <= 0);
      } else {
        CATCH_REQUIRE(heap->arr.cmpe(parent, lc) >= 0);
      }
    }
    if (RCSW_BINHEAP_RCHILD(i) <= binheap_size(heap)) {
      T *rc = (T *)darray_data_get(&heap->arr, RCSW_BINHEAP_RCHILD(i));
      if (heap->flags & RCSW_DS_BINHEAP_MIN) {
        CATCH_REQUIRE(heap->arr.cmpe(parent, rc) <= 0);
      } else {
        CATCH_REQUIRE(heap->arr.cmpe(parent, rc) >= 0);
      }
    }
  }
}

/*******************************************************************************
 * Test Functions
 ******************************************************************************/
template <typename T>
static void insert_test(th::gen_elt_type type, struct binheap_config *config) {
  struct binheap *heap;
  struct binheap  myheap;

  /* RCSW_NOALLOC_HANDLE requires a non-NULL handle */
  if (config->flags & RCSW_NOALLOC_HANDLE) {
    CATCH_REQUIRE(nullptr == binheap_init(nullptr, config));
    heap = binheap_init(&myheap, config);
  } else {
    heap = binheap_init(nullptr, config);
  }
  CATCH_REQUIRE(nullptr != heap);

  /* Empty-state invariants */
  CATCH_REQUIRE(binheap_isempty(heap));
  CATCH_REQUIRE(binheap_size(heap) == 0);
  {
    T dummy;
    CATCH_REQUIRE(binheap_extract(heap, &dummy) == ERROR);
  }

  th::element_generator<T> g(type, config->max_elts);

  for (size_t i = 0; i < config->max_elts; ++i) {
    T e = g.next();
    CATCH_REQUIRE(binheap_insert(heap, &e) == OK);
    /* NULL sentinel checks */
    CATCH_REQUIRE(binheap_insert(nullptr, nullptr) == ERROR);
  }

  CATCH_REQUIRE(binheap_size(heap) == config->max_elts);
  verify_heap<T>(heap);

  /* Insert beyond capacity must fail */
  {
    T e = g.next();
    CATCH_REQUIRE(binheap_insert(heap, &e) == ERROR);
  }

  binheap_destroy(heap);
}

template <typename T>
static void delete_test(th::gen_elt_type type, struct binheap_config *config) {
  struct binheap *heap;
  struct binheap  myheap;

  if (config->flags & RCSW_NOALLOC_HANDLE) {
    heap = binheap_init(&myheap, config);
  } else {
    heap = binheap_init(nullptr, config);
  }
  CATCH_REQUIRE(nullptr != heap);

  th::element_generator<T> g(type, config->max_elts);
  for (size_t i = 0; i < config->max_elts; ++i) {
    T e = g.next();
    CATCH_REQUIRE(binheap_insert(heap, &e) == OK);
  }

  CATCH_REQUIRE(binheap_size(heap) == config->max_elts);

  T minmax{};
  minmax.value1 = (heap->flags & RCSW_DS_BINHEAP_MIN) ? INT_MIN : INT_MAX;

  size_t old_elts = binheap_size(heap);
  while (!binheap_isempty(heap)) {
    size_t index = std::max<int>(rand() % binheap_size(heap), 1);
    CATCH_REQUIRE(binheap_delete_key(heap, index, &minmax) == OK);
    CATCH_REQUIRE(binheap_delete_key(nullptr, index, &minmax) == ERROR);
    CATCH_REQUIRE(binheap_size(heap) == old_elts - 1);
    verify_heap<T>(heap);
    old_elts = binheap_size(heap);
  }

  binheap_destroy(heap);
}

template <typename T>
static void make_test(th::gen_elt_type type, struct binheap_config *config) {
  struct binheap *heap;
  struct binheap  myheap;

  if (config->flags & RCSW_NOALLOC_HANDLE) {
    heap = binheap_init(&myheap, config);
  } else {
    heap = binheap_init(nullptr, config);
  }
  CATCH_REQUIRE(nullptr != heap);

  T arr[TH_NUM_ITEMS];
  th::element_generator<T> g(type, config->max_elts);
  for (size_t i = 0; i < config->max_elts; ++i) {
    arr[i] = g.next();
  }

  CATCH_REQUIRE(binheap_make(heap, arr, config->max_elts) == OK);
  verify_heap<T>(heap);

  binheap_destroy(heap);
}

/**
 * \brief Verify that sequential extraction from a heap built from sorted data
 *        produces elements in the expected order.
 */
template <typename T>
static void structure_test(th::gen_elt_type type, struct binheap_config *config) {
  struct binheap *heap;
  struct binheap  myheap;

  if (config->flags & RCSW_NOALLOC_HANDLE) {
    heap = binheap_init(&myheap, config);
  } else {
    heap = binheap_init(nullptr, config);
  }
  CATCH_REQUIRE(nullptr != heap);

  T arr[TH_NUM_ITEMS];
  th::element_generator<T> g(type, config->max_elts);
  for (size_t i = 0; i < config->max_elts; ++i) {
    arr[i] = g.next();
  }

  CATCH_REQUIRE(binheap_make(heap, arr, config->max_elts) == OK);
  verify_heap<T>(heap);

  for (size_t i = 0; i < config->max_elts; ++i) {
    T e;
    CATCH_REQUIRE(binheap_extract(heap, &e) == OK);

    /* For INC_VALS min-heap or DEC_VALS max-heap: extract in original order */
    if (((type == th::gen_elt_type::ekINC_VALS) && (heap->flags & RCSW_DS_BINHEAP_MIN)) ||
        ((type == th::gen_elt_type::ekDEC_VALS) && !(heap->flags & RCSW_DS_BINHEAP_MIN))) {
      CATCH_REQUIRE(e.value1 == arr[i].value1);
    }
    /* For DEC_VALS min-heap or INC_VALS max-heap: extract in reverse order */
    else if (((type == th::gen_elt_type::ekDEC_VALS) && (heap->flags & RCSW_DS_BINHEAP_MIN)) ||
             ((type == th::gen_elt_type::ekINC_VALS) && !(heap->flags & RCSW_DS_BINHEAP_MIN))) {
      CATCH_REQUIRE(e.value1 == arr[config->max_elts - i - 1].value1);
    }
  }

  CATCH_REQUIRE(binheap_isempty(heap));
  binheap_destroy(heap);
}

template <typename T>
static void print_test(th::gen_elt_type type, struct binheap_config *config) {
  /* NULL handle must not crash */
  binheap_print(nullptr);

  config->flags &= ~RCSW_NOALLOC_HANDLE;
  struct binheap *heap = binheap_init(nullptr, config);
  CATCH_REQUIRE(nullptr != heap);

  binheap_print(heap); /* empty */

  T arr[TH_NUM_ITEMS];
  th::element_generator<T> g(type, config->max_elts);
  for (size_t i = 0; i < config->max_elts; ++i) {
    arr[i] = g.next();
  }

  CATCH_REQUIRE(binheap_make(heap, arr, config->max_elts) == OK);
  binheap_print(heap); /* populated */

  binheap_destroy(heap);
}

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("binheap Insert Test", "[ds][binheap]") {
  run_test<element8>(insert_test<element8>, th::gen_elt_type::ekINC_VALS);
  run_test<element8>(insert_test<element8>, th::gen_elt_type::ekDEC_VALS);
  run_test<element8>(insert_test<element8>, th::gen_elt_type::ekRAND_VALS);

  run_test<element4>(insert_test<element4>, th::gen_elt_type::ekINC_VALS);
  run_test<element4>(insert_test<element4>, th::gen_elt_type::ekDEC_VALS);
  run_test<element4>(insert_test<element4>, th::gen_elt_type::ekRAND_VALS);

  run_test<element2>(insert_test<element2>, th::gen_elt_type::ekINC_VALS);
  run_test<element2>(insert_test<element2>, th::gen_elt_type::ekDEC_VALS);
  run_test<element2>(insert_test<element2>, th::gen_elt_type::ekRAND_VALS);

  run_test<element1>(insert_test<element1>, th::gen_elt_type::ekINC_VALS);
  run_test<element1>(insert_test<element1>, th::gen_elt_type::ekDEC_VALS);
  run_test<element1>(insert_test<element1>, th::gen_elt_type::ekRAND_VALS);
}

CATCH_TEST_CASE("binheap Delete Test", "[ds][binheap]") {
  /*
   * Only run with element8: delete uses a sentinel value1 of INT_MIN/INT_MAX.
   * Smaller element types cannot represent those values correctly.
   */
  run_test<element8>(delete_test<element8>, th::gen_elt_type::ekINC_VALS);
  run_test<element8>(delete_test<element8>, th::gen_elt_type::ekDEC_VALS);
  run_test<element8>(delete_test<element8>, th::gen_elt_type::ekRAND_VALS);
}

CATCH_TEST_CASE("binheap Make Test", "[ds][binheap]") {
  run_test<element8>(make_test<element8>, th::gen_elt_type::ekINC_VALS);
  run_test<element8>(make_test<element8>, th::gen_elt_type::ekDEC_VALS);
  run_test<element8>(make_test<element8>, th::gen_elt_type::ekRAND_VALS);

  run_test<element4>(make_test<element4>, th::gen_elt_type::ekINC_VALS);
  run_test<element4>(make_test<element4>, th::gen_elt_type::ekDEC_VALS);
  run_test<element4>(make_test<element4>, th::gen_elt_type::ekRAND_VALS);

  run_test<element2>(make_test<element2>, th::gen_elt_type::ekINC_VALS);
  run_test<element2>(make_test<element2>, th::gen_elt_type::ekDEC_VALS);
  run_test<element2>(make_test<element2>, th::gen_elt_type::ekRAND_VALS);

  run_test<element1>(make_test<element1>, th::gen_elt_type::ekINC_VALS);
  run_test<element1>(make_test<element1>, th::gen_elt_type::ekDEC_VALS);
  run_test<element1>(make_test<element1>, th::gen_elt_type::ekRAND_VALS);
}

CATCH_TEST_CASE("binheap Structure Test", "[ds][binheap]") {
  run_test<element8>(structure_test<element8>, th::gen_elt_type::ekINC_VALS);
  run_test<element8>(structure_test<element8>, th::gen_elt_type::ekDEC_VALS);

  run_test<element4>(structure_test<element4>, th::gen_elt_type::ekINC_VALS);
  run_test<element4>(structure_test<element4>, th::gen_elt_type::ekDEC_VALS);

  run_test<element2>(structure_test<element2>, th::gen_elt_type::ekINC_VALS);
  run_test<element2>(structure_test<element2>, th::gen_elt_type::ekDEC_VALS);

  run_test<element1>(structure_test<element1>, th::gen_elt_type::ekINC_VALS);
  run_test<element1>(structure_test<element1>, th::gen_elt_type::ekDEC_VALS);
}

CATCH_TEST_CASE("binheap Print Test", "[ds][binheap]") {
  run_test<element8>(print_test<element8>, th::gen_elt_type::ekINC_VALS);
}
