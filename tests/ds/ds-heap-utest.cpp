/**
 * \file ds-heap-utest.cpp
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

/*******************************************************************************
 * Test Helper Functions
 ******************************************************************************/
template <typename T>
static void run_test(void (*test)(enum gen_elt_type,
                                  struct binheap_config *config),
                     enum gen_elt_type type) {
  RCSW_ER_INIT(TH_ZLOG_CONF);
  struct binheap_config config;
  memset(&config, 0, sizeof(binheap_config));
  config.flags    = 0;
  config.cmpe     = th::cmpe<T>;
  config.printe   = th::printe<T>;
  config.elt_size = sizeof(T);
  CATCH_REQUIRE(th::ds_init(&config) == OK);

  uint32_t flags[] = {RCSW_NONE,
                      RCSW_ZALLOC,
                      RCSW_NOALLOC_HANDLE,
                      RCSW_NOALLOC_DATA,
                      RCSW_DS_BINHEAP_MIN};
  uint32_t applied = 0;
  for (size_t i = 0; i < RCSW_ARRAY_ELTS(flags); ++i) {
    applied |= flags[i];
    for (size_t j = i + 1; j < RCSW_ARRAY_ELTS(flags); ++j) {
      applied |= flags[j];
      for (int m = 1; m < TH_NUM_ITEMS; ++m) {
        config.flags     = applied;
        config.max_elts  = m;
        config.init_size = 0;
        test(type, &config);
      } /* for(m..) */
      applied &= ~flags[j];
    } /* for(j..) */
  } /* for(i..) */

  th::ds_shutdown(&config);
  RCSW_ER_DEINIT();
} /* run_test() */

template <typename T>
static void verify_heap(struct binheap *heap) {
  size_t i = 0;

  while (++i <= binheap_size(heap)) {
    T *parent = (T *)darray_data_get(&heap->arr, i);

    if (RCSW_BINHEAP_LCHILD(i) <= binheap_size(heap)) {
      T *l_child = (T *)darray_data_get(&heap->arr, RCSW_BINHEAP_LCHILD(i));
      if (heap->flags & RCSW_DS_BINHEAP_MIN) {
        CATCH_REQUIRE(heap->arr.cmpe(parent, l_child) <= 0);
      } else {
        CATCH_REQUIRE(heap->arr.cmpe(parent, l_child) >= 0);
      }
    }
    if (RCSW_BINHEAP_RCHILD(i) <= binheap_size(heap)) {
      T *r_child = (T *)darray_data_get(&heap->arr, RCSW_BINHEAP_RCHILD(i));
      if (heap->flags & RCSW_DS_BINHEAP_MIN) {
        CATCH_REQUIRE(heap->arr.cmpe(parent, r_child) <= 0);
      } else {
        CATCH_REQUIRE(heap->arr.cmpe(parent, r_child) >= 0);
      }
    }
  } /* while() */
}

/*******************************************************************************
 * Test Functions
 ******************************************************************************/
template <typename T>
static void insert_test(enum gen_elt_type type, struct binheap_config *config) {
  struct binheap *heap = nullptr;
  struct binheap  myheap;

  if (config->flags & RCSW_NOALLOC_HANDLE) {
    CATCH_REQUIRE(nullptr == binheap_init(nullptr, config));
    heap = binheap_init(&myheap, config);
  } else {
    heap = binheap_init(nullptr, config);
  }

  CATCH_REQUIRE(nullptr != heap);

  th::element_generator<T> g(type, config->max_elts);

  for (size_t i = 0; i < config->max_elts; ++i) {
    T e = g.next();
    CATCH_REQUIRE(binheap_insert(heap, &e) == OK);
    CATCH_REQUIRE(binheap_insert(nullptr, nullptr) == ERROR);
  } /* for() */

  /* verify heap */
  CATCH_REQUIRE(binheap_size(heap) == (size_t)config->max_elts);
  verify_heap<T>(heap);

  binheap_destroy(heap);
} /* insert_test() */

template <typename T>
static void delete_test(gen_elt_type type, struct binheap_config *config) {
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
  } /* for() */

  CATCH_REQUIRE(binheap_size(heap) == (size_t)config->max_elts);

  size_t old_elts = binheap_size(heap);
  T      minmax{};

  if (heap->flags & RCSW_DS_BINHEAP_MIN) {
    minmax.value1 = INT_MIN;
  } else {
    minmax.value1 = INT_MAX;
  }

  while (!binheap_isempty(heap)) {
    size_t index = std::max<int>(rand() % binheap_size(heap), 1);
    CATCH_REQUIRE(OK == binheap_delete_key(heap, index, &minmax));
    CATCH_REQUIRE(ERROR == binheap_delete_key(nullptr, index, &minmax));
    CATCH_REQUIRE(binheap_size(heap) == old_elts - 1);
    verify_heap<T>(heap);
    old_elts = binheap_size(heap);
  } /* while() */

  binheap_destroy(heap);
} /* delete_test() */

template <typename T>
static void make_test(gen_elt_type type, struct binheap_config *config) {
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
  } /* for() */

  /* make heap and verify structure */
  CATCH_REQUIRE(OK == binheap_make(heap, arr, config->max_elts));
  verify_heap<T>(heap);

  binheap_destroy(heap);
} /* make_test() */

template <typename T>
static void structure_test(enum gen_elt_type      type,
                           struct binheap_config *config) {
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
  } /* for() */

  /* make heap and verify structure */
  CATCH_REQUIRE(OK == binheap_make(heap, arr, config->max_elts));
  verify_heap<T>(heap);

  for (size_t i = 0; i < config->max_elts; ++i) {
    T e;
    CATCH_REQUIRE(OK == binheap_extract(heap, &e));

    if (((ekINC_VALS == type) && (heap->flags & RCSW_DS_BINHEAP_MIN)) ||
        ((ekDEC_VALS == type) && !(heap->flags & RCSW_DS_BINHEAP_MIN))) {
      CATCH_REQUIRE(e.value1 == arr[i].value1);
    } else if (((ekDEC_VALS == type) && (heap->flags & RCSW_DS_BINHEAP_MIN)) ||
               ((ekINC_VALS == type) && !(heap->flags & RCSW_DS_BINHEAP_MIN))) {
      CATCH_REQUIRE(e.value1 == arr[config->max_elts - i - 1].value1);
    }
  } /* for(i..) */
  binheap_destroy(heap);
} /* make_test() */

template <typename T>
static void print_test(gen_elt_type type, struct binheap_config *config) {
  struct binheap *heap;

  config->flags &= ~RCSW_NOALLOC_HANDLE;
  binheap_print(nullptr);
  heap = binheap_init(nullptr, config);

  CATCH_REQUIRE(nullptr != heap);
  binheap_print(heap);

  T                        arr[TH_NUM_ITEMS];
  th::element_generator<T> g(type, config->max_elts);

  for (size_t i = 0; i < config->max_elts; ++i) {
    arr[i] = g.next();
  } /* for() */

  /* make heap and verify structure */
  CATCH_REQUIRE(OK == binheap_make(heap, arr, config->max_elts));

  binheap_print(heap);
  binheap_destroy(heap);
} /* print_test() */

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("insert() Test", "[ds][binheap]") {
  run_test<element8>(insert_test<element8>, ekINC_VALS);
  run_test<element8>(insert_test<element8>, ekDEC_VALS);
  run_test<element8>(insert_test<element8>, ekRAND_VALS);

  run_test<element4>(insert_test<element4>, ekINC_VALS);
  run_test<element4>(insert_test<element4>, ekDEC_VALS);
  run_test<element4>(insert_test<element4>, ekRAND_VALS);

  run_test<element2>(insert_test<element2>, ekINC_VALS);
  run_test<element2>(insert_test<element2>, ekDEC_VALS);
  run_test<element2>(insert_test<element2>, ekRAND_VALS);

  run_test<element1>(insert_test<element1>, ekINC_VALS);
  run_test<element1>(insert_test<element1>, ekDEC_VALS);
  run_test<element1>(insert_test<element1>, ekRAND_VALS);
}
CATCH_TEST_CASE("delete() Test", "[ds][binheap]") {
  run_test<element8>(delete_test<element8>, ekINC_VALS);
  run_test<element8>(delete_test<element8>, ekDEC_VALS);
  run_test<element8>(delete_test<element8>, ekRAND_VALS);
  /* don't run with element{1,2,4}--problems with integer representation */
}
CATCH_TEST_CASE("make() Test", "[ds][binheap]") {
  run_test<element8>(make_test<element8>, ekINC_VALS);
  run_test<element8>(make_test<element8>, ekDEC_VALS);
  run_test<element8>(make_test<element8>, ekRAND_VALS);

  run_test<element4>(make_test<element4>, ekINC_VALS);
  run_test<element4>(make_test<element4>, ekDEC_VALS);
  run_test<element4>(make_test<element4>, ekRAND_VALS);

  run_test<element2>(make_test<element2>, ekINC_VALS);
  run_test<element2>(make_test<element2>, ekDEC_VALS);
  run_test<element2>(make_test<element2>, ekRAND_VALS);

  run_test<element1>(make_test<element1>, ekINC_VALS);
  run_test<element1>(make_test<element1>, ekDEC_VALS);
  run_test<element1>(make_test<element1>, ekRAND_VALS);
}
CATCH_TEST_CASE("Structure Test", "[ds][binheap]") {
  run_test<element8>(structure_test<element8>, ekINC_VALS);
  run_test<element8>(structure_test<element8>, ekDEC_VALS);

  run_test<element4>(structure_test<element4>, ekINC_VALS);
  run_test<element4>(structure_test<element4>, ekDEC_VALS);

  run_test<element2>(structure_test<element2>, ekINC_VALS);
  run_test<element2>(structure_test<element2>, ekDEC_VALS);

  run_test<element1>(structure_test<element1>, ekINC_VALS);
  run_test<element1>(structure_test<element1>, ekDEC_VALS);
}
CATCH_TEST_CASE("Print Test", "[ds][binheap]") {
  run_test<element8>(print_test<element8>, ekINC_VALS);
}
