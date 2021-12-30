/**
 * @file sort-test.cpp
 *
 * Test of simple sorting library.
 *
 * @copyright 2017 John Harwell, All rights reserved.
 *
 * This file is part of RCSW.
 *
 * RCSW is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * RCSW is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * RCSW.  If not, see <http://www.gnu.org/licenses/
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "ds_test.hpp"
#include "rcsw/algorithm/sort.h"
#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_PREFIX_ALL
#include "catch.hpp"

/*******************************************************************************
 * Constant Definitions
 ******************************************************************************/
#define DATA_SIZE 10

/*******************************************************************************
 * Forward Declarations
 ******************************************************************************/
static int sort_cmpe(const void* const e1, const void* const e2) {
  return *(int*)e1 - *(int*)e2;
}

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("Insertion Sort Test","[sort]") {
    struct element_set data(DATA_SIZE);
    data.data_gen();
    insertion_sort(data.elts(),DATA_SIZE,sizeof(struct element),th_cmpe);
    /* for (int i = 0; i < DATA_SIZE-2; ++i) { */
      /* CATCH_REQUIRE(th_cmpe(data.elts()+i,data.elts()+i+1) <= 0); */
    /* } /\* for(i..) *\/ */
}


CATCH_TEST_CASE("Radix Sort", "[sort]") {
  size_t arr1[10] = {10, 43, 243, 87, 2, 45, 1, 4, 67, 78};
  size_t arr2[10] = {0x10, 0x43, 0x243, 0x87, 0x2, 0x45, 0x1, 0x4, 0x67, 0x78};
  size_t tmp[10];
  radix_sort(arr1, tmp, 10, 10);
  radix_sort(arr2, tmp, 10, 16);
  for (size_t i = 1; i < 10; ++i) {
    CATCH_REQUIRE(arr1[i] > arr1[i-1]);
    CATCH_REQUIRE(arr2[i] > arr2[i-1]);
  } /* for(i..) */
}
#include "rcsw/utils/mem.h"
CATCH_TEST_CASE("Quick Sort", "[sort]") {
  int arr1[10] = {10, 43, 243, 87, 2, 45, 1, 4, 67, 78};
  int arr2[10] = {0x10, 0x43, 0x243, 0x87, 0x2, 0x45, 0x1, 0x4, 0x67, 0x78};
  qsort_rec(arr1, 0, 9, sizeof(int), sort_cmpe);
  qsort_rec(arr1, 0, 9, sizeof(int), sort_cmpe);
  mem_dump32(arr2, 40);

  qsort_iter(arr2, 9, sizeof(int), sort_cmpe);
  qsort_iter(arr2, 9, sizeof(int), sort_cmpe);
  mem_dump32(arr2, 40);
  for (size_t i = 1; i < 10; ++i) {
    CATCH_REQUIRE(arr1[i-1] <= arr1[i]);
    CATCH_REQUIRE(arr2[i-1] <= arr2[i]);
  } /* for(i..) */
}
