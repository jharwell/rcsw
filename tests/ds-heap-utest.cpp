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
#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_PREFIX_ALL
#include <catch.hpp>

#include "rcsw/ds/binheap.h"
#include "rcsw/utils/utils.h"
#include "rcsw/utils/mem.h"
#include "tests/ds_test.h"
#include "tests/ds_test.hpp"

/*******************************************************************************
 * Test Helper Functions
 ******************************************************************************/
template<typename T>
static void run_test(void (*test)(enum gen_elt_type, struct binheap_params *params),
                     enum gen_elt_type type) {
  /* dbg_init(); */
  /* dbg_insmod(M_TESTING, "Testing"); */
  /* dbg_insmod(M_DS_BINHEAP, "BINHEAP"); */
  /* dbg_insmod(M_DS_DARRAY, "DARRAY"); */
  /* dbg_mod_lvl_set(M_DS_BINHEAP, DBG_V); */
  /* dbg_mod_lvl_set(M_DS_DARRAY, DBG_V); */
  struct binheap_params params;
  params.flags = 0;
  params.cmpe = th_cmpe<T>;
  params.printe = th_printe<T>;
  params.elt_size = sizeof(T);
  CATCH_REQUIRE(th_ds_init(&params) == OK);

  uint32_t flags[] = {
    RCSW_NOALLOC_HANDLE,
    RCSW_NOALLOC_DATA,
    RCSW_DS_BINHEAP_MIN
  };
  for (int j = 1; j < TH_NUM_ITEMS; ++j) {
    for (size_t i = 0; i < RCSW_ARRAY_ELTS(flags); ++i) {
      params.flags = flags[i];
      params.max_elts = j;
      params.init_size = 0;
      test(type, &params);
    } /* for(i..) */
  } /* for(j..) */
  th_ds_shutdown(&params);
} /* run_test() */

template<typename T>
static void verify_heap(struct binheap* heap) {
  size_t i = 0;

  while (++i <= binheap_n_elts(heap)) {
    T* parent = (T*)darray_data_get(&heap->arr, i);


    if (RCSW_BINHEAP_LCHILD(i) <= binheap_n_elts(heap)) {
      T* l_child = (T*)darray_data_get(&heap->arr,
                                       RCSW_BINHEAP_LCHILD(i));
      if (heap->flags & RCSW_DS_BINHEAP_MIN) {
        CATCH_REQUIRE(heap->arr.cmpe(parent, l_child) <= 0);
      } else {
        CATCH_REQUIRE(heap->arr.cmpe(parent, l_child) >= 0);
      }
    }
    if (RCSW_BINHEAP_RCHILD(i) <= binheap_n_elts(heap)) {
      T* r_child = (T*)darray_data_get(&heap->arr,
                                                   RCSW_BINHEAP_RCHILD(i));
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
template<typename T>
static void insert_test(enum gen_elt_type type, struct binheap_params * params) {
  struct binheap *heap = nullptr;
  struct binheap myheap;


  if (params->flags & RCSW_NOALLOC_HANDLE) {
    CATCH_REQUIRE(nullptr == binheap_init(nullptr, params));
    heap = binheap_init(&myheap, params);
  } else {
    heap = binheap_init(nullptr, params);
  }

  CATCH_REQUIRE(nullptr != heap);

  element_generator<T> g(type, params->max_elts);

  for (int i = 0; i < params->max_elts; ++i) {
    T e = g.next();
    CATCH_REQUIRE(binheap_insert(heap, &e) == OK);
    CATCH_REQUIRE(binheap_insert(nullptr, nullptr) == ERROR);
  } /* for() */

  /* verify heap */
  CATCH_REQUIRE(binheap_n_elts(heap) == (size_t)params->max_elts);
  verify_heap<T>(heap);

  binheap_destroy(heap);
} /* insert_test() */

template<typename T>
static void delete_test(gen_elt_type type, struct binheap_params * params) {
  struct binheap *heap;
  struct binheap myheap;

  if (params->flags & RCSW_NOALLOC_HANDLE) {
    heap = binheap_init(&myheap, params);
  } else {
    heap = binheap_init(nullptr, params);
  }

  CATCH_REQUIRE(nullptr != heap);

  element_generator<T> g(type, params->max_elts);

  for (int i = 0; i < params->max_elts; ++i) {
    T e = g.next();
    CATCH_REQUIRE(binheap_insert(heap, &e) == OK);
  } /* for() */

  CATCH_REQUIRE(binheap_n_elts(heap) == (size_t)params->max_elts);

  size_t old_elts = binheap_n_elts(heap);
  T minmax;

  if (heap->flags & RCSW_DS_BINHEAP_MIN) {
    minmax.value1 = INT_MIN;
  } else {
    minmax.value1 = INT_MAX;
  }

  while (!binheap_isempty(heap)) {
    size_t index = std::max<int>(rand() % binheap_n_elts(heap),
                                                 1);
    CATCH_REQUIRE(OK == binheap_delete_key(heap, index, &minmax));
    CATCH_REQUIRE(ERROR == binheap_delete_key(nullptr, index, &minmax));
    CATCH_REQUIRE(binheap_n_elts(heap) == old_elts - 1);
    verify_heap<T>(heap);
    old_elts = binheap_n_elts(heap);
  } /* while() */

  binheap_destroy(heap);
} /* delete_test() */

template<typename T>
static void make_test(gen_elt_type type, struct binheap_params * params) {
  struct binheap *heap;
  struct binheap myheap;

  if (params->flags & RCSW_NOALLOC_HANDLE) {
    heap = binheap_init(&myheap, params);
  } else {
    heap = binheap_init(nullptr, params);
  }

  CATCH_REQUIRE(nullptr != heap);
  T arr[TH_NUM_ITEMS];


  element_generator<T> g(type, params->max_elts);
  for (int i = 0; i < params->max_elts; ++i) {
    arr[i] = g.next();
  } /* for() */

  /* make heap and verify structure */
  CATCH_REQUIRE(OK == binheap_make(heap, arr, params->max_elts));
  verify_heap<T>(heap);

  binheap_destroy(heap);
} /* make_test() */

template<typename T>
static void structure_test(enum gen_elt_type type, struct binheap_params * params) {
  struct binheap *heap;
  struct binheap myheap;

  if (params->flags & RCSW_NOALLOC_HANDLE) {
    heap = binheap_init(&myheap, params);
  } else {
    heap = binheap_init(nullptr, params);
  }

  CATCH_REQUIRE(nullptr != heap);
  T arr[TH_NUM_ITEMS];

  element_generator<T> g(type, params->max_elts);
  for (int i = 0; i < params->max_elts; ++i) {
    arr[i] = g.next();
  } /* for() */

  /* make heap and verify structure */
  CATCH_REQUIRE(OK == binheap_make(heap, arr, params->max_elts));
  verify_heap<T>(heap);

  for (int i = 0; i < params->max_elts; ++i) {
    T e;
    CATCH_REQUIRE(OK == binheap_extract(heap, &e));

    if (((ekINC_VALS == type) && (heap->flags & RCSW_DS_BINHEAP_MIN)) ||
        ((ekDEC_VALS == type) && !(heap->flags & RCSW_DS_BINHEAP_MIN))) {
      CATCH_REQUIRE(e.value1 == arr[i].value1);
    } else if (((ekDEC_VALS == type) && (heap->flags & RCSW_DS_BINHEAP_MIN)) ||
               ((ekINC_VALS == type) && !(heap->flags & RCSW_DS_BINHEAP_MIN))) {
      CATCH_REQUIRE(e.value1 == arr[params->max_elts - i - 1].value1);
    }
  } /* for(i..) */
  binheap_destroy(heap);
} /* make_test() */

template<typename T>
static void print_test(gen_elt_type type, struct binheap_params * params) {
  struct binheap *heap;

  params->flags &= ~RCSW_NOALLOC_HANDLE;
  binheap_print(nullptr);
  heap = binheap_init(nullptr, params);

  CATCH_REQUIRE(nullptr != heap);
  binheap_print(heap);

  T arr[TH_NUM_ITEMS];
  element_generator<T> g(type, params->max_elts);

  for (int i = 0; i < params->max_elts; ++i) {
    arr[i] = g.next();
  } /* for() */

  /* make heap and verify structure */
  CATCH_REQUIRE(OK == binheap_make(heap, arr, params->max_elts));

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
