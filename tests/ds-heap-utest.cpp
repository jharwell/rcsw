/**
 * @file heap_test.cpp
 *
 * Test of binary heap.
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
#include <limits.h>

extern "C" {
#include "rcsw/ds/bin_heap.h"
#include "rcsw/utils/utils.h"
#include "rcsw/utils/mem.h"
#include "tests/ds_test.h"
}

#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_PREFIX_ALL
#include <catch.hpp>

/*******************************************************************************
 * Test Function Forward Declarations
 ******************************************************************************/
/**
 * @brief Test adding data to the heap in order, in reverse order, or
 * in a random order.
 */
static void insert_test(int type, struct ds_params* params);

/**
 * @brief Test of \ref bin_heap_delete_key()
 */
static void delete_test(int type, struct ds_params* params);

/**
 * @brief Test of \ref bin_heap_make()
 */
static void make_test(int type, struct ds_params* params);

/*******************************************************************************
 * Test Helper Functions
 ******************************************************************************/
static void test_runner(void (*test)(int type,
                                     struct ds_params *params), int type) {
  dbg_init();
  dbg_insmod(M_TESTING, "Testing");
  dbg_insmod(M_DS_BIN_HEAP, "BIN_HEAP");
  dbg_insmod(M_DS_DARRAY, "DARRAY");
  /* dbg_mod_lvl_set(M_DS_BIN_HEAP, DBG_V); */
  /* dbg_mod_lvl_set(M_DS_DARRAY, DBG_V); */
  struct ds_params params;
  params.tag = DS_BIN_HEAP;
  params.flags = 0;
  params.cmpe = th_key_cmp;
  params.printe = th_printe;
  params.el_size = sizeof(struct element);
  CATCH_REQUIRE(th_ds_init(&params) == OK);

  for (int j = 1; j <= NUM_ITEMS; ++j) {
    for (int i = DS_APP_DOMAIN_HANDLE; i <= DS_APP_DOMAIN_DATA; i <<= 1) {
    params.flags = i;
    params.max_elts = j;
    params.type.bhp.init_size = 0;
    test(type, &params);
    } /* for(i..) */
  } /* for(j..) */
  th_ds_shutdown(&params);
} /* test_runner() */

static void verify_heap(struct bin_heap* heap) {
  size_t i = 0;
  while (++i <= bin_heap_n_elts(heap)) {
    element* parent = (element*)darray_data_get(&heap->arr, i);

    if (BIN_HEAP_LCHILD(i) <= bin_heap_n_elts(heap)) {
      element* l_child = (element*)darray_data_get(&heap->arr,
                                                   BIN_HEAP_LCHILD(i));
      if (heap->flags & DS_RCSW_MIN_HEAP) {
        CATCH_REQUIRE(heap->arr.cmpe(parent, l_child) < 0);
      } else {
        CATCH_REQUIRE(heap->arr.cmpe(parent, l_child) > 0);
      }
    }
    if (BIN_HEAP_RCHILD(i) <= bin_heap_n_elts(heap)) {
      element* r_child = (element*)darray_data_get(&heap->arr,
                                                   BIN_HEAP_RCHILD(i));
      if (heap->flags & DS_RCSW_MIN_HEAP) {
        CATCH_REQUIRE(heap->arr.cmpe(parent, r_child) < 0);
      } else {
        CATCH_REQUIRE(heap->arr.cmpe(parent, r_child) > 0);
      }
    }
  } /* while() */
}
/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("in-order Test", "[bin_heap]") { test_runner(insert_test, 0); }
CATCH_TEST_CASE("rev-order Test", "[bin_heap]") { test_runner(insert_test, 1); }
CATCH_TEST_CASE("unordered Test", "[bin_heap]") { test_runner(insert_test, 2); }
CATCH_TEST_CASE("delete test", "[bin_heap]") { test_runner(delete_test, -1); }
CATCH_TEST_CASE("make Test", "[bin_heap]") { test_runner(make_test, -1); }

/*******************************************************************************
 * Test Functions
 ******************************************************************************/
static void insert_test(int type, struct ds_params * params) {
  struct bin_heap *heap;
  struct bin_heap myheap;

  if (params->flags & DS_APP_DOMAIN_HANDLE) {
    heap = bin_heap_init(&myheap, params);
  } else {
    heap = bin_heap_init(NULL, params);
  }

  CATCH_REQUIRE(NULL != heap);

  for (int i = 0; i < (int)params->max_elts; ++i) {
    struct element e;
    e.value2 = -1;
    if (0 == type) {
      e.value1 = i;
    } else if (1 == type) {
      e.value1 = (int)params->max_elts - i;
    } else {
      e.value1 = rand();
    }
    CATCH_REQUIRE(bin_heap_insert(heap, &e) == OK);
  } /* for() */

  /* verify heap */
  CATCH_REQUIRE(bin_heap_n_elts(heap) == params->max_elts);
  verify_heap(heap);

  bin_heap_destroy(heap);
} /* insert_test() */

static void delete_test(int type, struct ds_params * params) {
  struct bin_heap *heap;
  struct bin_heap myheap;

  if (params->flags & DS_APP_DOMAIN_HANDLE) {
    heap = bin_heap_init(&myheap, params);
  } else {
    heap = bin_heap_init(NULL, params);
  }

  CATCH_REQUIRE(NULL != heap);

  for (int i = 0; i < (int)params->max_elts; ++i) {
    struct element e;
    e.value2 = -1;
    e.value1 = i;
    CATCH_REQUIRE(bin_heap_insert(heap, &e) == OK);
  } /* for() */

  CATCH_REQUIRE(bin_heap_n_elts(heap) == params->max_elts);

  size_t old_elts = bin_heap_n_elts(heap);
  struct element min = {
    .value1 = INT_MAX,
    .value2 = -1,
  };
  while (!bin_heap_isempty(heap)) {
    size_t index = std::max<int>(rand() % bin_heap_n_elts(heap), 1);
    CATCH_REQUIRE(OK == bin_heap_delete_key(heap, index, &min));
    CATCH_REQUIRE(bin_heap_n_elts(heap) == old_elts - 1);
    verify_heap(heap);
    old_elts = bin_heap_n_elts(heap);
  } /* while() */

  bin_heap_destroy(heap);
} /* delete_test() */

static void make_test(int type, struct ds_params * params) {
  struct bin_heap *heap;
  struct bin_heap myheap;

  if (params->flags & DS_APP_DOMAIN_HANDLE) {
    heap = bin_heap_init(&myheap, params);
  } else {
    heap = bin_heap_init(NULL, params);
  }

  CATCH_REQUIRE(NULL != heap);
  struct element arr[params->max_elts];
  for (int i = 0; i < (int)params->max_elts; ++i) {
    struct element e;
    e.value2 = -1;
    e.value1 = i;
    arr[i] = e;
  } /* for() */

  CATCH_REQUIRE(OK == bin_heap_make(heap, arr, params->max_elts));
  verify_heap(heap);
  bin_heap_destroy(heap);
} /* make_test() */
