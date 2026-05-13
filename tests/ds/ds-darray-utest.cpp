/**
 * \file ds-darray-utest.cpp
 *
 * \copyright 2017 John Harwell, All rights reserved.
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#define CATCH_CONFIG_PREFIX_ALL
#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <vector>

#include "rcsw/ds/darray.h"
#include "tests/ds/ds_test.hpp"

/*******************************************************************************
 * Test Runner
 ******************************************************************************/
template <typename T>
static void run_test(void (*test)(int len, struct darray_config* config)) {
  struct darray_config config;
  memset(&config, 0, sizeof(darray_config));
  config.cmpe     = th::cmpe<T>;
  config.printe   = th::printe<T>;
  config.elt_size = sizeof(T);
  CATCH_REQUIRE(th::ds_init(&config) == OK);

  uint32_t flags[] = {
    RCSW_ZALLOC,
    RCSW_DS_SORTED,
    RCSW_DS_ORDERED,
    RCSW_NOALLOC_HANDLE,
    RCSW_NOALLOC_DATA,
  };
  run_test_flags(config, flags, RCSW_ARRAY_ELTS(flags), TH_NUM_ITEMS, test);
  th::ds_shutdown(&config);
}

/*******************************************************************************
 * Test Functions
 ******************************************************************************/
template <typename T>
static void addremove_test(int len, struct darray_config* config) {
  struct darray* arr;
  struct darray  myarr;

  /* verify NULL handle rejected when NOALLOC_HANDLE set */
  if (config->flags & RCSW_NOALLOC_HANDLE) {
    CATCH_REQUIRE(nullptr == darray_init(nullptr, config));
    arr = darray_init(&myarr, config);
  } else {
    arr = darray_init(nullptr, config);
  }
  CATCH_REQUIRE(nullptr != arr);

  /* empty state assertions */
  CATCH_REQUIRE(darray_isempty(arr));
  CATCH_REQUIRE(darray_size(arr) == 0);
  T dummy{};
  CATCH_REQUIRE(ERROR == darray_remove(arr, &dummy, 0));

  th::element_generator<T> g(th::gen_elt_type::ekINC_VALS, config->max_elts);
  std::vector<T>           inserted;

  for (int i = 0; i < len; i++) {
    T e = g.next();
    inserted.push_back(e);
    if (rand() % 2) {
      CATCH_REQUIRE(darray_insert(arr, &e, 0) == OK);
    } else {
      CATCH_REQUIRE(darray_insert(arr, &e, arr->current) == OK);
    }
    /* every inserted element must be findable */
    for (auto& el : inserted) {
      CATCH_REQUIRE(darray_idx_query(arr, &el) != -1);
    }
  }
  CATCH_REQUIRE(darray_size(arr) == (size_t)len);

  /* resize up */
  if (!(arr->flags & RCSW_NOALLOC_DATA)) {
    CATCH_REQUIRE(OK == darray_resize(arr, darray_size(arr) * 2));
  }

  /* remove all and verify each removal */
  for (int i = 0; i < len; i++) {
    T e;
    CATCH_REQUIRE(OK == darray_remove(arr, &e, 0));
    CATCH_REQUIRE(darray_idx_query(arr, &e) == -1);
  }
  CATCH_REQUIRE(darray_isempty(arr));
  darray_destroy(arr);
}

template <typename T>
static void sort_test(int len, struct darray_config* config) {
  struct darray* arr;
  struct darray  myarr;

  if (config->flags & RCSW_NOALLOC_HANDLE) {
    arr = darray_init(&myarr, config);
  } else {
    arr = darray_init(nullptr, config);
  }
  CATCH_REQUIRE(nullptr != arr);

  th::element_generator<T> g(th::gen_elt_type::ekRAND_VALS, config->max_elts);
  std::vector<T>           original;

  for (int i = 0; i < len; i++) {
    T e = g.next();
    original.push_back(e);
    CATCH_REQUIRE(darray_insert(arr, &e, arr->current) == OK);
  }

  darray_sort(arr, (rand() % 2) ? ekEXEC_ITER : ekEXEC_REC);

  /* verify sorted order AND permutation */
  std::vector<T> sorted;
  for (int i = 0; i < len; i++) {
    sorted.push_back(*reinterpret_cast<T*>(darray_data_get(arr, i)));
  }
  verify_sort_permutation(original, sorted.data(), len);

  darray_destroy(arr);
}

template <typename T>
static void copy_test(int len, struct darray_config* config) {
  struct darray* arr1;
  struct darray* arr2;
  struct darray  myarr;

  if (config->flags & RCSW_NOALLOC_HANDLE) {
    arr1 = darray_init(&myarr, config);
  } else {
    arr1 = darray_init(nullptr, config);
  }
  CATCH_REQUIRE(nullptr != arr1);

  th::element_generator<T> g(th::gen_elt_type::ekRAND_VALS, config->max_elts);
  std::vector<T>           inserted;
  for (int i = 0; i < len; i++) {
    T e = g.next();
    inserted.push_back(e);
    CATCH_REQUIRE(darray_insert(arr1, &e, arr1->current) == OK);
  }

  /* copy using heap allocation regardless of source config */
  arr2 = darray_copy(arr1, 0x0, nullptr);
  CATCH_REQUIRE(nullptr != arr2);

  /* both copies contain all elements */
  for (auto& e : inserted) {
    CATCH_REQUIRE(darray_idx_query(arr1, &e) != -1);
    CATCH_REQUIRE(darray_idx_query(arr2, &e) != -1);
  }

  /* copy preserves cmpe and printe */
  CATCH_REQUIRE(arr2->cmpe == arr1->cmpe);

  darray_destroy(arr1);
  darray_destroy(arr2);
}

template <typename T>
static void map_test(int len, struct darray_config* config) {
  struct darray* arr;
  struct darray  myarr;

  if (config->flags & RCSW_NOALLOC_HANDLE) {
    arr = darray_init(&myarr, config);
  } else {
    arr = darray_init(nullptr, config);
  }
  CATCH_REQUIRE(nullptr != arr);

  /* NULL callback rejected */
  CATCH_REQUIRE(ERROR == darray_map(arr, nullptr));

  th::element_generator<T> g(th::gen_elt_type::ekINC_VALS, config->max_elts);
  for (int i = 0; i < len; i++) {
    T e = g.next();
    CATCH_REQUIRE(darray_insert(arr, &e, arr->current) == OK);
  }

  /* map_func decrements value1 by 1 */
  CATCH_REQUIRE(darray_map(arr, th::map_func<T>) == OK);
  for (int i = 0; i < len; i++) {
    T e;
    CATCH_REQUIRE(darray_idx_serve(arr, &e, i) == OK);
    CATCH_REQUIRE(e.value1 == i - 1);
  }
  darray_destroy(arr);
}

template <typename T>
static void inject_test(int len, struct darray_config* config) {
  struct darray* arr;
  struct darray  myarr;

  if (config->flags & RCSW_NOALLOC_HANDLE) {
    arr = darray_init(&myarr, config);
  } else {
    arr = darray_init(nullptr, config);
  }
  CATCH_REQUIRE(nullptr != arr);

  /* NULL callback rejected */
  int dummy = 0;
  CATCH_REQUIRE(ERROR == darray_inject(arr, nullptr, &dummy));

  th::element_generator<T> g(th::gen_elt_type::ekINC_VALS, config->max_elts);
  int expected_sum = 0;
  for (int i = 0; i < len; i++) {
    T e = g.next();
    expected_sum += i;
    CATCH_REQUIRE(darray_insert(arr, &e, arr->current) == OK);
  }

  int total = 0;
  CATCH_REQUIRE(darray_inject(arr, th::inject_func<T>, &total) == OK);
  CATCH_REQUIRE(total == expected_sum);

  darray_destroy(arr);
}

template <typename T>
static void iter_test(int len, struct darray_config* config) {
  struct darray* arr;
  struct darray  myarr;

  if (config->flags & RCSW_NOALLOC_HANDLE) {
    arr = darray_init(&myarr, config);
  } else {
    arr = darray_init(nullptr, config);
  }
  CATCH_REQUIRE(nullptr != arr);

  th::element_generator<T> g(th::gen_elt_type::ekINC_VALS, config->max_elts);
  for (int i = 0; i < len; i++) {
    T e = g.next();
    CATCH_REQUIRE(darray_insert(arr, &e, arr->current) == OK);
  }

  T* e;
  struct ds_iterator iter;

  /* filtered forward: only even values */
  CATCH_REQUIRE(nullptr != darray_iter_init(&iter, arr, ekITER_FORWARD,
                                            th::iter_func_even<T>));
  while ((e = (T*)ds_iter_next(&iter)) != nullptr) {
    CATCH_REQUIRE(e->value1 % 2 == 0);
  }

  /* unfiltered forward: all values in order */
  CATCH_REQUIRE(nullptr != darray_iter_init(&iter, arr, ekITER_FORWARD,
                                            th::iter_func_all<T>));
  size_t count = 0;
  while ((e = (T*)ds_iter_next(&iter)) != nullptr) {
    CATCH_REQUIRE(e->value1 == (decltype(T::value1))count);
    count++;
  }
  CATCH_REQUIRE(count == darray_size(arr));

  /* unfiltered backward: values in reverse order */
  CATCH_REQUIRE(nullptr != darray_iter_init(&iter, arr, ekITER_BACKWARD,
                                            th::iter_func_all<T>));
  count = 0;
  while ((e = (T*)ds_iter_next(&iter)) != nullptr) {
    CATCH_REQUIRE(e->value1 == len - (decltype(T::value1))count - 1);
    count++;
  }
  CATCH_REQUIRE(count == darray_size(arr));

  /* two independent iterators over the same array */
  struct ds_iterator iter2;
  CATCH_REQUIRE(nullptr != darray_iter_init(&iter,  arr, ekITER_FORWARD,
                                            th::iter_func_all<T>));
  CATCH_REQUIRE(nullptr != darray_iter_init(&iter2, arr, ekITER_FORWARD,
                                            th::iter_func_all<T>));
  T* e1 = (T*)ds_iter_next(&iter);
  T* e2 = (T*)ds_iter_next(&iter2);
  CATCH_REQUIRE(e1 != nullptr);
  CATCH_REQUIRE(e2 != nullptr);
  /* both point at the same first element */
  CATCH_REQUIRE(th::cmpe<T>(e1, e2) == 0);

  darray_destroy(arr);
}

template <typename T>
static void filter_test(int len, struct darray_config* config) {
  struct darray* arr1;
  struct darray* arr2;
  struct darray  myarr;

  if (config->flags & RCSW_NOALLOC_HANDLE) {
    arr1 = darray_init(&myarr, config);
  } else {
    arr1 = darray_init(nullptr, config);
  }
  CATCH_REQUIRE(nullptr != arr1);

  th::element_generator<T> g(th::gen_elt_type::ekINC_VALS, config->max_elts);
  std::vector<T>           inserted;
  for (int i = 0; i < len; i++) {
    T e = g.next();
    inserted.push_back(e);
    CATCH_REQUIRE(darray_insert(arr1, &e, arr1->current) == OK);
  }

  if (config->flags & (RCSW_NOALLOC_DATA | RCSW_NOALLOC_HANDLE)) {
    arr2 = darray_filter(arr1, th::filter_func<T>, 0, nullptr);
    CATCH_REQUIRE(nullptr != arr2);
    for (auto& el : inserted) {
      if (th::filter_func<T>(&el)) {
        CATCH_REQUIRE(darray_idx_query(arr2, &el) != -1);
        CATCH_REQUIRE(darray_idx_query(arr1, &el) == -1);
      } else {
        CATCH_REQUIRE(darray_idx_query(arr2, &el) == -1);
        CATCH_REQUIRE(darray_idx_query(arr1, &el) != -1);
      }
    }
    darray_destroy(arr2);
  }
  darray_destroy(arr1);
}

template <typename T>
static void binarysearch_test(int len, struct darray_config* config) {
  struct darray* arr;
  struct darray  myarr;
  std::vector<T> inserted;

  if (config->flags & RCSW_NOALLOC_HANDLE) {
    arr = darray_init(&myarr, config);
  } else {
    arr = darray_init(nullptr, config);
  }
  CATCH_REQUIRE(nullptr != arr);

  th::element_generator<T> g(th::gen_elt_type::ekRAND_VALS, config->max_elts);
  for (int i = 0; i < len; i++) {
    T e = g.next();
    inserted.push_back(e);
    CATCH_REQUIRE(darray_insert(arr, &e, arr->current) == OK);
  }

  darray_sort(arr, ekEXEC_ITER);

  /* every inserted element must be found */
  for (auto& e : inserted) {
    CATCH_REQUIRE(darray_idx_query(arr, &e) != -1);
  }

  darray_destroy(arr);
}

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("darray Add/Remove Test", "[ds][darray]") {
  run_test<element8>(addremove_test<element8>);
  run_test<element4>(addremove_test<element4>);
  run_test<element2>(addremove_test<element2>);
  run_test<element1>(addremove_test<element1>);
}
CATCH_TEST_CASE("darray Sort Test", "[ds][darray]") {
  run_test<element8>(sort_test<element8>);
  run_test<element4>(sort_test<element4>);
  run_test<element2>(sort_test<element2>);
  run_test<element1>(sort_test<element1>);
}
CATCH_TEST_CASE("darray Copy Test", "[ds][darray]") {
  run_test<element8>(copy_test<element8>);
  run_test<element4>(copy_test<element4>);
  run_test<element2>(copy_test<element2>);
  run_test<element1>(copy_test<element1>);
}
CATCH_TEST_CASE("darray Map Test", "[ds][darray]") {
  run_test<element8>(map_test<element8>);
  run_test<element4>(map_test<element4>);
  run_test<element2>(map_test<element2>);
  run_test<element1>(map_test<element1>);
}
CATCH_TEST_CASE("darray Inject Test", "[ds][darray]") {
  run_test<element8>(inject_test<element8>);
  run_test<element4>(inject_test<element4>);
  run_test<element2>(inject_test<element2>);
  run_test<element1>(inject_test<element1>);
}
CATCH_TEST_CASE("darray Iterator Test", "[ds][darray]") {
  run_test<element8>(iter_test<element8>);
  run_test<element4>(iter_test<element4>);
  run_test<element2>(iter_test<element2>);
  run_test<element1>(iter_test<element1>);
}
CATCH_TEST_CASE("darray Filter Test", "[ds][darray]") {
  run_test<element8>(filter_test<element8>);
  run_test<element4>(filter_test<element4>);
  run_test<element2>(filter_test<element2>);
  run_test<element1>(filter_test<element1>);
}
CATCH_TEST_CASE("darray Binary Search Test", "[ds][darray]") {
  run_test<element8>(binarysearch_test<element8>);
  run_test<element4>(binarysearch_test<element4>);
  run_test<element2>(binarysearch_test<element2>);
  run_test<element1>(binarysearch_test<element1>);
}
CATCH_TEST_CASE("darray Print Test", "[ds][darray]") {
  /* Coverage test: verify print doesn't crash on NULL or populated arrays */
  struct darray_config config;
  memset(&config, 0, sizeof(darray_config));
  config.cmpe     = th::cmpe<element4>;
  config.printe   = th::printe<element4>;
  config.elt_size = sizeof(element4);
  config.max_elts = TH_NUM_ITEMS;
  CATCH_REQUIRE(th::ds_init(&config) == OK);
  struct darray* arr = darray_init(nullptr, &config);
  CATCH_REQUIRE(arr != nullptr);
  darray_print(nullptr);  /* must not crash */
  darray_print(arr);      /* empty */
  element4 e{};
  e.value1 = 1;
  darray_insert(arr, &e, 0);
  darray_print(arr);      /* one element */
  darray_destroy(arr);
  th::ds_shutdown(&config);
}
