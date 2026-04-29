/**
 * \file alg-search-utest.cpp
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

#include <algorithm>
#include <vector>

#include "rcsw/algorithm/search.h"
#include "rcsw/algorithm/sort.h"
#include "tests/element.hpp"
#include "tests/test.h"

/*******************************************************************************
 * Test Helpers
 ******************************************************************************/
/**
 * Sort the dataset and verify that every element can be found by both
 * bsearch_iter and bsearch_rec, and that the returned index actually points
 * to a matching element.
 */
template <typename T>
static void test_bsearch_present(size_t n_elts) {
  th::element_set<T> data(n_elts);
  data.data_gen();
  qsort_rec(data.elts.data(), 0, (int)n_elts - 1, sizeof(T), th::cmpe<T>);

  for (size_t i = 0; i < n_elts; ++i) {
    /* iterative */
    int idx_iter = bsearch_iter(data.elts.data(),
                                &data.elts[i],
                                th::cmpe<T>,
                                sizeof(T),
                                0,
                                (int)n_elts - 1);
    CATCH_REQUIRE(idx_iter != -1);
    CATCH_REQUIRE(th::cmpe<T>(data.elts.data() + idx_iter, &data.elts[i]) ==
                  0);

    /* recursive */
    int idx_rec = bsearch_rec(data.elts.data(),
                              &data.elts[i],
                              th::cmpe<T>,
                              sizeof(T),
                              0,
                              (int)n_elts - 1);
    CATCH_REQUIRE(idx_rec != -1);
    CATCH_REQUIRE(th::cmpe<T>(data.elts.data() + idx_rec, &data.elts[i]) == 0);
  }
}

/**
 * Verify that searching for an element known not to be in the array
 * returns -1 for both variants.
 */
template <typename T>
static void test_bsearch_absent(size_t n_elts) {
  th::element_set<T> data(n_elts);
  data.data_gen();
  qsort_rec(data.elts.data(), 0, (int)n_elts - 1, sizeof(T), th::cmpe<T>);

  /* construct an element with a value that cannot appear in the sorted set */
  T absent{};
  absent.value1 = std::numeric_limits<decltype(T::value1)>::max();

  /* only run the absent test if absent is not already in the set */
  bool found = false;
  for (size_t i = 0; i < n_elts; ++i) {
    if (th::cmpe<T>(&data.elts[i], &absent) == 0) {
      found = true;
      break;
    }
  }
  if (found) {
    return;
  }

  CATCH_REQUIRE(-1 == bsearch_iter(data.elts.data(),
                                   &absent,
                                   th::cmpe<T>,
                                   sizeof(T),
                                   0,
                                   (int)n_elts - 1));
  CATCH_REQUIRE(-1 == bsearch_rec(data.elts.data(),
                                  &absent,
                                  th::cmpe<T>,
                                  sizeof(T),
                                  0,
                                  (int)n_elts - 1));
}

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("Binary Search - Element Present", "[alg][search]") {
  for (size_t i = 1; i < TH_NUM_ITEMS; ++i) {
    test_bsearch_present<element8>(i);
    test_bsearch_present<element4>(i);
    test_bsearch_present<element2>(i);
    test_bsearch_present<element1>(i);
  }
}

CATCH_TEST_CASE("Binary Search - Element Absent", "[alg][search]") {
  for (size_t i = 1; i < TH_NUM_ITEMS; ++i) {
    test_bsearch_absent<element8>(i);
    test_bsearch_absent<element4>(i);
    test_bsearch_absent<element2>(i);
    test_bsearch_absent<element1>(i);
  }
}

CATCH_TEST_CASE("Binary Search - Single Element Found", "[alg][search]") {
  element4 arr[1]{};
  arr[0].value1 = 7;
  CATCH_REQUIRE(0 == bsearch_iter(arr, arr, th::cmpe<element4>,
                                  sizeof(element4), 0, 0));
  CATCH_REQUIRE(0 == bsearch_rec(arr, arr, th::cmpe<element4>,
                                 sizeof(element4), 0, 0));
}

CATCH_TEST_CASE("Binary Search - Single Element Not Found", "[alg][search]") {
  element4 arr[1]{};
  arr[0].value1 = 7;
  element4 target{};
  target.value1 = 99;
  CATCH_REQUIRE(-1 == bsearch_iter(arr, &target, th::cmpe<element4>,
                                   sizeof(element4), 0, 0));
  CATCH_REQUIRE(-1 == bsearch_rec(arr, &target, th::cmpe<element4>,
                                  sizeof(element4), 0, 0));
}

CATCH_TEST_CASE("Binary Search - Empty Array", "[alg][search]") {
  element4 arr[1]{};
  element4 target{};
  target.value1 = 1;
  /* high < low: nothing to search */
  CATCH_REQUIRE(-1 == bsearch_iter(arr, &target, th::cmpe<element4>,
                                   sizeof(element4), 0, -1));
  CATCH_REQUIRE(-1 == bsearch_rec(arr, &target, th::cmpe<element4>,
                                  sizeof(element4), 0, -1));
}

CATCH_TEST_CASE("Binary Search - NULL Input", "[alg][search]") {
  element4 e{};
  CATCH_REQUIRE(-1 == bsearch_iter(nullptr, &e, th::cmpe<element4>,
                                   sizeof(element4), 0, 0));
  CATCH_REQUIRE(-1 == bsearch_rec(nullptr, &e, th::cmpe<element4>,
                                  sizeof(element4), 0, 0));
  CATCH_REQUIRE(-1 == bsearch_iter(&e, nullptr, th::cmpe<element4>,
                                   sizeof(element4), 0, 0));
}

CATCH_TEST_CASE("Binary Search - All Equal Elements", "[alg][search]") {
  /*
   * When all elements are equal, binary search must still find one of them
   * (any valid index is acceptable) and must not return -1.
   */
  const size_t n = 8;
  element4     arr[n]{};
  for (size_t i = 0; i < n; ++i) {
    arr[i].value1 = 5;
  }
  element4 target{};
  target.value1 = 5;

  int idx_iter =
    bsearch_iter(arr, &target, th::cmpe<element4>, sizeof(element4), 0, n - 1);
  CATCH_REQUIRE(idx_iter != -1);
  CATCH_REQUIRE(arr[idx_iter].value1 == 5);

  int idx_rec =
    bsearch_rec(arr, &target, th::cmpe<element4>, sizeof(element4), 0, n - 1);
  CATCH_REQUIRE(idx_rec != -1);
  CATCH_REQUIRE(arr[idx_rec].value1 == 5);
}

CATCH_TEST_CASE("Binary Search - First and Last Element", "[alg][search]") {
  const size_t       n = 16;
  th::element_set<element4> data(n);
  data.data_gen();
  qsort_rec(data.elts.data(), 0, n - 1, sizeof(element4), th::cmpe<element4>);

  /* first element */
  CATCH_REQUIRE(0 == bsearch_iter(data.elts.data(), &data.elts[0],
                                  th::cmpe<element4>, sizeof(element4), 0, n - 1));
  /* last element */
  int idx = bsearch_iter(data.elts.data(), &data.elts[n - 1],
                         th::cmpe<element4>, sizeof(element4), 0, n - 1);
  CATCH_REQUIRE(idx != -1);
  CATCH_REQUIRE(th::cmpe<element4>(data.elts.data() + idx,
                                   &data.elts[n - 1]) == 0);
}
