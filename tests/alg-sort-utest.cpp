/**
 * \file sort-test.cpp
 *
 * Test of simple sorting library.
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

#include "tests/ds_test.h"
#include "tests/ds_test.hpp"
#include "rcsw/algorithm/sort.h"
#include "rcsw/utils/mem.h"

/*******************************************************************************
 * Test Functions
 ******************************************************************************/
template<typename T>
static void test_insertion_sort(size_t n_elts) {
  struct element_set<T> data(n_elts);
  data.data_gen();
  insertion_sort(data.elts.data(),
                 n_elts,
                 sizeof(T),
                 th_cmpe<T>);
  for (int i = 0; i < (int)n_elts - 1; ++i) {
    CATCH_REQUIRE(th_cmpe<T>(data.elts.data() + i,
                             data.elts.data() + i + 1) <= 0);
  } /* for(i..) */
}

static void test_radix_sort(size_t n_elts) {
  size_t arr1[TH_NUM_ITEMS] = {
    10, 43, 243, 87, 2, 45, 1, 4, 67, 78, 12, 34, 56, 66, 19, 93
  };
  size_t arr2[TH_NUM_ITEMS] = {
    0x10, 0x43, 0x243, 0x87, 0x2, 0x45, 0x1, 0x4, 0x67, 0x78, 0x56, 0xFF, 0x12, 0x34, 0x98, 0xAA
  };
  size_t tmp[TH_NUM_ITEMS];

  radix_sort(arr1, tmp, TH_NUM_ITEMS, 10);
  radix_sort(arr2, tmp, TH_NUM_ITEMS, 16);

  for (int i = 0; i < (int)n_elts - 1; ++i) {
    CATCH_REQUIRE(arr1[i] <= arr1[i + 1]);
    CATCH_REQUIRE(arr2[i] <= arr2[i + 1]);
  } /* for(i..) */
}

template<typename T>
void test_qsort(size_t n_elts) {
  struct element_set<T> data1(n_elts);
  data1.data_gen();

  qsort_rec(data1.elts.data(), 0, n_elts - 1, sizeof(T), th_cmpe<T>);

  struct element_set<T> data2(n_elts);
  data2.data_gen();

  qsort_iter(data2.elts.data(), n_elts - 1, sizeof(T), th_cmpe<T>);

  for (int i = 0; i < (int)n_elts - 1; ++i) {
    CATCH_REQUIRE(th_cmpe<T>(data1.elts.data() + i,
                             data1.elts.data() + i + 1) <= 0);
    CATCH_REQUIRE(th_cmpe<T>(data2.elts.data() + i,
                             data2.elts.data() + i + 1) <= 0);
  } /* for(i..) */
}

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("Insertion Sort Test","[alg][sort]") {
  for (size_t i = 0; i < TH_NUM_ITEMS; ++i) {
    test_insertion_sort<element8>(i);
    test_insertion_sort<element4>(i);
    test_insertion_sort<element2>(i);
    test_insertion_sort<element1>(i);
  } /* for(i..) */
}

CATCH_TEST_CASE("Radix Sort", "[alg][sort]") {
  for (size_t i = 0; i < TH_NUM_ITEMS; ++i) {
    test_radix_sort(i);
  } /* for(i..) */
}

CATCH_TEST_CASE("Quick Sort", "[alg][sort]") {
  for (size_t i = 1; i < TH_NUM_ITEMS; ++i) {
    test_qsort<element8>(i);
    test_qsort<element4>(i);
    test_qsort<element2>(i);
    test_qsort<element1>(i);
  } /* for(i..) */
}
