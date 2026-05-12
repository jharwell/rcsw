/**
 * \file alg-sort-utest.cpp
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
#include <numeric>
#include <vector>

#include "rcsw/algorithm/sort.h"
#include "tests/ds_test.h"
#include "tests/ds_test.hpp"
#include "tests/element.hpp"

/*******************************************************************************
 * Test Helpers
 ******************************************************************************/
/**
 * Verify that an array is sorted in non-decreasing order according to cmpe,
 * and that it is a permutation of the original.
 */
template <typename T>
static void verify_sort(const std::vector<T>& original,
                        const T*              sorted,
                        size_t                n_elts) {
  /* sorted order */
  for (size_t i = 0; i + 1 < n_elts; ++i) {
    CATCH_REQUIRE(th::cmpe<T>(sorted + i, sorted + i + 1) <= 0);
  }
  /* same multiset as original */
  std::vector<T> orig_copy = original;
  std::vector<T> sort_copy(sorted, sorted + n_elts);
  std::sort(orig_copy.begin(),
            orig_copy.end(),
            [](const T& a, const T& b) {
              return th::cmpe<T>(&a, &b) < 0;
            });
  std::sort(sort_copy.begin(),
            sort_copy.end(),
            [](const T& a, const T& b) {
              return th::cmpe<T>(&a, &b) < 0;
            });
  CATCH_REQUIRE(orig_copy == sort_copy);
}

template <typename T>
static std::vector<T> make_sorted(size_t n_elts) {
  th::element_set<T> s(n_elts);
  s.data_gen();
  std::vector<T> v = s.elts;
  std::sort(v.begin(), v.end(), [](const T& a, const T& b) {
    return th::cmpe<T>(&a, &b) < 0;
  });
  return v;
}

template <typename T>
static std::vector<T> make_reverse_sorted(size_t n_elts) {
  auto v = make_sorted<T>(n_elts);
  std::reverse(v.begin(), v.end());
  return v;
}

template <typename T>
static std::vector<T> make_all_equal(size_t n_elts) {
  th::element_set<T> s(1);
  s.data_gen();
  return std::vector<T>(n_elts, s.elts[0]);
}

/*******************************************************************************
 * Insertion Sort
 ******************************************************************************/
template <typename T>
static void test_insertion_sort(size_t n_elts) {
  th::element_set<T> data(n_elts);
  data.data_gen();
  std::vector<T> original = data.elts;

  insertion_sort(data.elts.data(), n_elts, sizeof(T), th::cmpe<T>);
  verify_sort(original, data.elts.data(), n_elts);
}

template <typename T>
static void test_insertion_sort_already_sorted(size_t n_elts) {
  auto v = make_sorted<T>(n_elts);
  std::vector<T> original = v;
  insertion_sort(v.data(), n_elts, sizeof(T), th::cmpe<T>);
  verify_sort(original, v.data(), n_elts);
}

template <typename T>
static void test_insertion_sort_reverse_sorted(size_t n_elts) {
  auto v = make_reverse_sorted<T>(n_elts);
  std::vector<T> original = v;
  insertion_sort(v.data(), n_elts, sizeof(T), th::cmpe<T>);
  verify_sort(original, v.data(), n_elts);
}

template <typename T>
static void test_insertion_sort_all_equal(size_t n_elts) {
  auto v = make_all_equal<T>(n_elts);
  std::vector<T> original = v;
  insertion_sort(v.data(), n_elts, sizeof(T), th::cmpe<T>);
  verify_sort(original, v.data(), n_elts);
}

CATCH_TEST_CASE("Insertion Sort - Random Input", "[alg][sort][insertion]") {
  for (size_t i = 0; i < TH_NUM_ITEMS; ++i) {
    test_insertion_sort<element8>(i);
    test_insertion_sort<element4>(i);
    test_insertion_sort<element2>(i);
    test_insertion_sort<element1>(i);
  }
}

CATCH_TEST_CASE("Insertion Sort - Already Sorted", "[alg][sort][insertion]") {
  for (size_t i = 1; i < TH_NUM_ITEMS; ++i) {
    test_insertion_sort_already_sorted<element4>(i);
    test_insertion_sort_already_sorted<element1>(i);
  }
}

CATCH_TEST_CASE("Insertion Sort - Reverse Sorted", "[alg][sort][insertion]") {
  for (size_t i = 1; i < TH_NUM_ITEMS; ++i) {
    test_insertion_sort_reverse_sorted<element4>(i);
    test_insertion_sort_reverse_sorted<element1>(i);
  }
}

CATCH_TEST_CASE("Insertion Sort - All Equal", "[alg][sort][insertion]") {
  for (size_t i = 1; i < TH_NUM_ITEMS; ++i) {
    test_insertion_sort_all_equal<element4>(i);
    test_insertion_sort_all_equal<element1>(i);
  }
}

CATCH_TEST_CASE("Insertion Sort - NULL Input", "[alg][sort][insertion]") {
  /* Should not crash -- RCSW_FPC_V returns cleanly */
  insertion_sort(nullptr, 5, sizeof(element4), th::cmpe<element4>);
  insertion_sort(nullptr, 0, sizeof(element4), th::cmpe<element4>);
}

/*******************************************************************************
 * Radix Sort
 ******************************************************************************/
static void verify_radix_sort(size_t* arr, size_t n_elts) {
  for (size_t i = 0; i + 1 < n_elts; ++i) {
    CATCH_REQUIRE(arr[i] <= arr[i + 1]);
  }
}

static void test_radix_sort_base(size_t base, size_t n_elts) {
  std::vector<size_t> arr(n_elts);
  /* fill with pseudo-random values */
  for (size_t i = 0; i < n_elts; ++i) {
    arr[i] = (size_t)((i * 6364136223846793005ULL + 1442695040888963407ULL) &
                      0xFFFF);
  }
  std::vector<size_t> original = arr;
  std::vector<size_t> tmp(n_elts);

  radix_sort(arr.data(), tmp.data(), n_elts, base);
  verify_radix_sort(arr.data(), n_elts);

  /* permutation check */
  std::vector<size_t> orig_sorted = original;
  std::sort(orig_sorted.begin(), orig_sorted.end());
  CATCH_REQUIRE(arr == orig_sorted);
}

CATCH_TEST_CASE("Radix Sort - Base 10", "[alg][sort][radix]") {
  for (size_t i = 1; i < TH_NUM_ITEMS; ++i) {
    test_radix_sort_base(10, i);
  }
}

CATCH_TEST_CASE("Radix Sort - Base 16", "[alg][sort][radix]") {
  for (size_t i = 1; i < TH_NUM_ITEMS; ++i) {
    test_radix_sort_base(16, i);
  }
}

CATCH_TEST_CASE("Radix Sort - Already Sorted", "[alg][sort][radix]") {
  size_t arr[] = {1, 2, 3, 4, 5, 6, 7, 8};
  size_t tmp[8];
  radix_sort(arr, tmp, 8, 10);
  verify_radix_sort(arr, 8);
}

CATCH_TEST_CASE("Radix Sort - All Equal", "[alg][sort][radix]") {
  size_t arr[] = {42, 42, 42, 42, 42};
  size_t tmp[5];
  radix_sort(arr, tmp, 5, 10);
  for (int i = 0; i < 5; ++i) {
    CATCH_REQUIRE(arr[i] == 42);
  }
}

CATCH_TEST_CASE("Radix Sort - Single Element", "[alg][sort][radix]") {
  size_t arr[] = {99};
  size_t tmp[1];
  radix_sort(arr, tmp, 1, 10);
  CATCH_REQUIRE(arr[0] == 99);
}

/*******************************************************************************
 * Quicksort
 ******************************************************************************/
template <typename T>
static void test_qsort_random(size_t n_elts) {
  /* test both rec and iter on the same input */
  th::element_set<T> data(n_elts);
  data.data_gen();
  std::vector<T> original = data.elts;

  /* recursive */
  std::vector<T> rec = original;
  qsort_rec(rec.data(), 0, (int)n_elts - 1, sizeof(T), th::cmpe<T>);
  verify_sort(original, rec.data(), n_elts);

  /* iterative */
  std::vector<T> iter = original;
  qsort_iter(iter.data(), (int)n_elts - 1, sizeof(T), th::cmpe<T>);
  verify_sort(original, iter.data(), n_elts);

  /* both produce the same result */
  CATCH_REQUIRE(rec == iter);
}

template <typename T>
static void test_qsort_already_sorted(size_t n_elts) {
  auto v = make_sorted<T>(n_elts);
  std::vector<T> original = v;

  std::vector<T> rec = v;
  qsort_rec(rec.data(), 0, (int)n_elts - 1, sizeof(T), th::cmpe<T>);
  verify_sort(original, rec.data(), n_elts);

  std::vector<T> iter = v;
  qsort_iter(iter.data(), (int)n_elts - 1, sizeof(T), th::cmpe<T>);
  verify_sort(original, iter.data(), n_elts);
}

template <typename T>
static void test_qsort_reverse_sorted(size_t n_elts) {
  auto v = make_reverse_sorted<T>(n_elts);
  std::vector<T> original = v;

  std::vector<T> rec = v;
  qsort_rec(rec.data(), 0, (int)n_elts - 1, sizeof(T), th::cmpe<T>);
  verify_sort(original, rec.data(), n_elts);

  std::vector<T> iter = v;
  qsort_iter(iter.data(), (int)n_elts - 1, sizeof(T), th::cmpe<T>);
  verify_sort(original, iter.data(), n_elts);
}

template <typename T>
static void test_qsort_all_equal(size_t n_elts) {
  auto v = make_all_equal<T>(n_elts);
  std::vector<T> original = v;

  std::vector<T> rec = v;
  qsort_rec(rec.data(), 0, (int)n_elts - 1, sizeof(T), th::cmpe<T>);
  verify_sort(original, rec.data(), n_elts);

  std::vector<T> iter = v;
  qsort_iter(iter.data(), (int)n_elts - 1, sizeof(T), th::cmpe<T>);
  verify_sort(original, iter.data(), n_elts);
}

CATCH_TEST_CASE("Quicksort - Random Input", "[alg][sort][qsort]") {
  for (size_t i = 1; i < TH_NUM_ITEMS; ++i) {
    test_qsort_random<element8>(i);
    test_qsort_random<element4>(i);
    test_qsort_random<element2>(i);
    test_qsort_random<element1>(i);
  }
}

CATCH_TEST_CASE("Quicksort - Already Sorted", "[alg][sort][qsort]") {
  for (size_t i = 1; i < TH_NUM_ITEMS; ++i) {
    test_qsort_already_sorted<element4>(i);
    test_qsort_already_sorted<element1>(i);
  }
}

CATCH_TEST_CASE("Quicksort - Reverse Sorted", "[alg][sort][qsort]") {
  for (size_t i = 1; i < TH_NUM_ITEMS; ++i) {
    test_qsort_reverse_sorted<element4>(i);
    test_qsort_reverse_sorted<element1>(i);
  }
}

CATCH_TEST_CASE("Quicksort - All Equal", "[alg][sort][qsort]") {
  for (size_t i = 1; i < TH_NUM_ITEMS; ++i) {
    test_qsort_all_equal<element4>(i);
    test_qsort_all_equal<element1>(i);
  }
}

CATCH_TEST_CASE("Quicksort - Single Element", "[alg][sort][qsort]") {
  element4 e{};
  e.value1 = 42;
  qsort_rec(&e, 0, 0, sizeof(element4), th::cmpe<element4>);
  CATCH_REQUIRE(e.value1 == 42);
  qsort_iter(&e, 0, sizeof(element4), th::cmpe<element4>);
  CATCH_REQUIRE(e.value1 == 42);
}
