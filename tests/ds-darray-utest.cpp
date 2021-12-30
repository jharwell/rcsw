/**
 * @file darray-test.cpp
 *
 * Test of dynamic array.
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
#include <stdlib.h>

extern "C" {
#include "rcsw/ds/darray.h"
#include "rcsw/algorithm/sort.h"
#include "tests/ds_test.h"
}

#define CATCH_CONFIG_MAIN
#include <catch.hpp>

/******************************************************************************
 * Constant declarations
 *****************************************************************************/
#define NUM_TESTS       6
#define MODULE_ID M_TESTING

/*******************************************************************************
 * Test Functions
 ******************************************************************************/
/**
 * @brief Test appending/prepending items into an darray and then removing them.
 */
static void addremove_test(int len, struct ds_params* params);

/**
 * @brief Test deleting arrs of different sizes.
 */
static void delete_test(int len, struct ds_params* params);

/**
 * @brief Test for looking up items in darray
 */
static void contains_test(int len, struct ds_params* params);

/**
 * @brief Test of \ref darray_filter().
 */
static void filter_test(int len, struct ds_params* params);

/**
 * @brief Test of \ref darray_copy().
 */
static void copy_test(int len, struct ds_params* params);

/**
 * @brief Test of \ref darray_sort()
 */
static void sort_test(int len, struct ds_params *params);

/**
 * @brief Test of builtin darray binary search on sorted _arrs.
 */
static void binarysearch_test(int len, struct ds_params *params);

/**
 * @brief Test of \ref darray_inject()
 */
static void inject_test(int len, struct ds_params * params);

/**
 * @brief Test of darray iteration.
 */
static void iter_test(int len, struct ds_params * params);

/*******************************************************************************
 * Test Helper Functions
 ******************************************************************************/
static void test_runner1(void (*test)(int len, struct ds_params *params)) {
  /* dbg_init(); */
  /* dbg_insmod(M_TESTING,"Testing"); */
  /* dbg_insmod(M_DS_DARRAY,"DARRAY"); */

  struct ds_params params;
  params.tag = DS_DARRAY;
  params.flags = 0;
  params.cmpe = th_key_cmp;
  params.printe = th_printe;
  params.el_size = sizeof(struct element);
  REQUIRE(th_ds_init(&params) == OK);
  int j, k;
  /* test with defined sizes */
  for (j = 1; j <= NUM_ITEMS; ++j) {
    params.type.da.init_size = j + 4;
    for (int i = 0; i <= 0x100; ++i) {
      params.flags = i;
      test(j, &params);
    } /* for(i...) */
  } /* for(j...) */
  th_ds_shutdown(&params);
} /* test_runner1() */

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
TEST_CASE("darray Add/Remove Test", "[darray]") { test_runner1(addremove_test); }
TEST_CASE("darray Delete Test", "[darray]") { test_runner1(delete_test); }
TEST_CASE("darray Contains Test", "[darray]") { test_runner1(contains_test); }
TEST_CASE("darray Filter Test", "[darray]") { test_runner1(inject_test); }
TEST_CASE("darray Copy Test", "[darray]") { test_runner1(copy_test); }
TEST_CASE("darray Sort Test", "[darray]") { test_runner1(sort_test); }
TEST_CASE("darray Binary Search Test", "[darray]") { test_runner1(binarysearch_test); }
TEST_CASE("darray Inject Test", "[darray]") { test_runner1(inject_test); }
TEST_CASE("darray Iterator Test", "[darray]") { test_runner1(iter_test); }

/*******************************************************************************
 * Test Functions
 ******************************************************************************/
static void addremove_test(int len, struct ds_params *params) {
  struct darray *_arr;
  struct darray my_arr;
  if (params->flags & DS_APP_DOMAIN_HANDLE) {
    _arr = darray_init(&my_arr, params);
  } else {
    _arr = darray_init(NULL, params);
  }

  int i, j;
  int arr[len];
  REQUIRE(NULL != _arr);

  for (i = 0; i < len; i++) {
    struct element e1;
    e1.value1 = i;

    if (rand() % 2) {  /* prepend */
      REQUIRE(darray_insert(_arr, &e1, 0) == OK);
    } else { /* append */
      REQUIRE(darray_insert(_arr, &e1, _arr->current) == OK);
    }

    for (j = 0; j < (len/4) %(j + 1); ++i) {
      struct element e = {.value1 = arr[len %(j+1)], .value2 = 17};
      darray_remove(_arr, NULL, len % (j+1));
      REQUIRE(darray_index_query(_arr, &e) == -1);
    }
    arr[i] = i;
  } /* for() */

  REQUIRE(_arr->current == len);

  for (i = 0; i < len; ++i) {
    struct element e = {.value1 = arr[i], .value2 = 17};
    REQUIRE(darray_index_query(_arr, &e) != -1);
  }

  for (i = 0; i < len %(i + 1); i++) {
    struct element e =  {.value1 = arr[len%(i+1)], .value2 = 17};
    darray_remove(_arr, NULL, len % (i+1));
    REQUIRE(darray_index_query(_arr, &e) == -1);
  }
  darray_destroy(_arr);
} /* addremove_test () */

static void delete_test(int len, struct ds_params *params) {
  struct darray* _arr;
  int i;
  struct darray my_arr;

  if (params->flags & DS_APP_DOMAIN_HANDLE) {
    _arr = darray_init(&my_arr, params);
  } else {
    _arr = darray_init(NULL, params);
  }

  REQUIRE(NULL != _arr);

  for (i = 1; i <= len; i++) {
    struct element e;
    int value = (rand() % len) % (len + 1);
    e.value1 = value;
    REQUIRE(darray_insert(_arr, &e, _arr->current) == OK);
  }

  darray_destroy(_arr);
} /* delete_test() */

static void contains_test(int len, struct ds_params *params) {
  struct darray* _arr;
  int i, j;
  int arr[len];
  struct darray my_arr;

  if (params->flags & DS_APP_DOMAIN_HANDLE) {
    _arr = darray_init(&my_arr, params);
  } else {
    _arr = darray_init(NULL, params);
  }

  REQUIRE(NULL != _arr);

  for(i = 0; i < len; i++) {
    struct element e;
    int value = (rand() % len) % (len + 1);
    e.value1 = value;
    REQUIRE(darray_insert(_arr, &e, _arr->current) == OK);
    arr[i] = value;
  }

  for (i = 0; i < len; i++) {
    int value = rand() % (i+ 1);
    for (j = 0; j < len; j++){
      if (value == arr[j]) {
        struct element e = {.value1 = value, .value2 = 17};
        REQUIRE(darray_index_query(_arr, &e) != -1);
        break;
      }
    } /* end for() */
  } /* end for() */

  darray_destroy(_arr);
} /* contains_test() */

static void filter_test(int len, struct ds_params *params) {
  struct darray* _arr1, *_arr2;
  int i;
  int arr[len];
  struct darray my_arr;

  if (params->flags & DS_APP_DOMAIN_HANDLE) {
    _arr1 = darray_init(&my_arr, params);
  } else {
    _arr1 = darray_init(NULL, params);
  }

  REQUIRE(NULL != _arr1);

  for (i = 0; i < len; i++) {
    struct element e;
    int value = rand() % (i+1);
    e.value1 = value;
    e.value2 = value + 1;
    REQUIRE(darray_insert(_arr1, &e, _arr1->current) == OK);
    arr[i] = value;
  }

  if (!(params->flags & (DS_APP_DOMAIN_DATA | DS_APP_DOMAIN_HANDLE))) {
    _arr2 = darray_filter(_arr1, th_filter_func, params);
  } else {
    _arr2 = darray_filter(_arr1, th_filter_func, NULL);
  }
  REQUIRE(NULL != _arr2);

  for (i = 0; i < len; i++) {
    struct element e = {.value1 = arr[i], .value2 = 17};
    if (th_filter_func(&e)) {
      REQUIRE(darray_index_query(_arr2, &e) != -1);
      REQUIRE(darray_index_query(_arr1, &e) == -1);
    }
  }

  darray_destroy(_arr1);
  darray_destroy(_arr2);
} /* filter_test() */

static void copy_test(int len, struct ds_params *params) {
  struct darray* _arr1, *_arr2;
  int i;
  int arr[len];
  struct darray my_arr;

  if (params->flags & DS_APP_DOMAIN_HANDLE) {
    _arr1 = darray_init(&my_arr, params);
  } else {
    _arr1 = darray_init(NULL, params);
  }
  REQUIRE(NULL != _arr1);

  for (i = 0; i < len; i++) {
    struct element e;
    int value = rand() % (i+1);
    e.value1 = value;
    REQUIRE(darray_insert(_arr1, &e, _arr1->current) == OK);
    arr[i] = value;
  }


  if (!(params->flags & (DS_APP_DOMAIN_DATA | DS_APP_DOMAIN_HANDLE))) {
    _arr2 = darray_copy(_arr1, params);
  } else {
    _arr2 = darray_copy(_arr1, NULL);
  }

  REQUIRE(NULL != _arr2);

  for (i = 0; i < len; i++) {
    struct element e = {.value1 = arr[i], .value2 = 17};
    REQUIRE(darray_index_query(_arr1, &e) != -1);
    REQUIRE(darray_index_query(_arr2, &e) != -1);
  }

  darray_destroy(_arr1);
  darray_destroy(_arr2);
} /* copy_test() */

static void sort_test(int len, struct ds_params *params) {
  struct darray* _arr1;
  int i;
  struct darray my_arr;

  if (params->flags & DS_APP_DOMAIN_HANDLE) {
    _arr1 = darray_init(&my_arr, params);
  } else {
    _arr1 = darray_init(NULL, params);
  }

  REQUIRE(NULL != _arr1);

  for (i = 0; i < len; i++) {
    struct element e;
    int value = rand() % (i+1);
    e.value1 = value;
    REQUIRE(darray_insert(_arr1, &e, _arr1->current) == OK);
  }

  if (rand() %2) {
    darray_sort(_arr1, QSORT_ITER);
  } else {
    darray_sort(_arr1, QSORT_REC);
  }

  /* validate sorting */
  for (i = 0; i < len-1; i++) {
    REQUIRE(((struct element*)darray_data_get(_arr1, i))->value1 <=
            ((struct element*)darray_data_get(_arr1, i+1))->value1);
  } /* for() */

  darray_destroy(_arr1);
} /* sort_test() */

static void binarysearch_test(int len, struct ds_params *params) {
  struct darray* _arr1;
  int i;
  int arr[len];
  struct darray my_arr;

  if (params->flags & DS_APP_DOMAIN_HANDLE) {
    _arr1 = darray_init(&my_arr, params);
  } else {
    _arr1 = darray_init(NULL, params);
  }

  REQUIRE(NULL != _arr1);

  for (i = 0; i < len; i++) {
    struct element e;
    int value = rand() % (i+1) + 3;
    e.value1 = value;
    arr[i] = value;
    REQUIRE(darray_insert(_arr1, &e, _arr1->current) == OK);
  } /* for() */

  darray_sort(_arr1, QSORT_ITER);

  for (i = 0; i < len; i++) {
    struct element e = {.value1 = arr[i], .value2 = 17};
    REQUIRE(darray_index_query(_arr1, &e) != -1);
  } /* for() */

  darray_destroy(_arr1);
} /* binarysearch_test() */

static void inject_test(int len, struct ds_params * params) {
  struct darray *arr;
  struct darray myarr;
  int i;
  int sum = 0;
  if (params->flags & DS_APP_DOMAIN_HANDLE) {
    arr = darray_init(&myarr, params);
  } else {
    arr = darray_init(NULL, params);
  }
  REQUIRE(NULL != arr);

  for (i = 0; i < len; i++) {
    struct element e = {.value1 = i, .value2 = 17};
    sum +=i;
    REQUIRE(darray_insert(arr, &e, arr->current) == OK);
  }


  int total = 0;
  REQUIRE(darray_inject(arr, th_inject_func, &total) == OK);
  REQUIRE(total == sum);

  darray_destroy(arr);
} /* inject_test() */

static void iter_test(int len, struct ds_params * params) {
  struct darray *arr;
  struct darray myarr;
  int i;

  if (params->flags & DS_APP_DOMAIN_HANDLE) {
    arr = darray_init(&myarr, params);
  } else {
    arr = darray_init(NULL, params);
  }
  REQUIRE(NULL != arr);

  for (i = 0; i < len; i++) {
    struct element e = {.value1 = i, .value2 = 17};
    REQUIRE(darray_insert(arr, &e, arr->current) == OK);
  }

  struct element * e;
  struct ds_iterator * iter = ds_iter_init(DS_DARRAY, arr, th_iter_func);
  REQUIRE(NULL != iter);
  while ((e = (element*)ds_iter_next(iter)) != NULL) {
    REQUIRE(e->value1 % 2 == 0);
  }

  iter = ds_iter_init(DS_DARRAY, arr, NULL);
  REQUIRE(NULL != iter);
  int count = 0;
  while ((e = (element*)ds_iter_next(iter)) != NULL) {
    REQUIRE((int)e->value1 == count);
    count++;
  }
  REQUIRE(count == arr->current);

  darray_destroy(arr);
} /* iter_test() */
