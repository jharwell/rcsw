/**
 * \file ds-fifo-utest.cpp
 *
 * \copyright 2023 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_PREFIX_ALL
#include <catch.hpp>

#include "rcsw/ds/fifo.h"
#include "rcsw/utils/utils.h"
#include "tests/ds_test.h"
#include "tests/ds_test.hpp"

/*******************************************************************************
 * Namespaces/Decls
 ******************************************************************************/
using fifo_test_t = void(*)(int len, struct fifo_params *params);

/*******************************************************************************
 * Test Helper Functions
 ******************************************************************************/
template<typename T>
static void run_test(fifo_test_t test) {
  RCSW_ER_INIT();

  struct fifo_params params;
  memset(&params, 0, sizeof(fifo_params));
  params.flags = 0;
  params.printe = th::printe<T>;
  params.elt_size = sizeof(T);
  CATCH_REQUIRE(th::ds_init(&params) == OK);


  uint32_t flags[] = {
    RCSW_NONE,
    RCSW_ZALLOC,
    RCSW_NOALLOC_HANDLE,
    RCSW_NOALLOC_DATA,
  };

  uint32_t applied = 0;
  for (size_t i = 0; i < RCSW_ARRAY_ELTS(flags); ++i) {
    applied |= flags[i];
    for (size_t j = i + 1; j < RCSW_ARRAY_ELTS(flags); ++j) {
      applied |= flags[j];

      for (int k = 1; k < TH_NUM_ITEMS; ++k) {
        params.flags = applied;
        params.max_elts = k;
        test(k, &params);
      } /* for(k..) */

      applied &= ~flags[j];
    } /* for(j..) */
  } /* for(i..) */

  th::ds_shutdown(&params);

  RCSW_ER_DEINIT();
}

/*******************************************************************************
 * Test Functions
 ******************************************************************************/
template<typename T>
static void rdwr_test(int len, struct fifo_params *  params) {
  struct fifo *fifo;
  struct fifo myfifo;

  fifo = fifo_init(&myfifo, params);
  CATCH_REQUIRE(nullptr != fifo);

  th::element_generator<T> g(gen_elt_type::ekINC_VALS, params->max_elts);

  for (int i = 0; i < len * 2; i++) {
    T e = g.next();
    if (i < len) {
      CATCH_REQUIRE(fifo_add(fifo, &e) == OK);
    } else {
      CATCH_REQUIRE(fifo_add(fifo, &e) == ERROR);
    }
  } /* for() */
  fifo_print(fifo);

  /* verify fifo contents */
  for (int i = 0; i < len; ++i) {
    T e;
    CATCH_REQUIRE(OK == fifo_remove(fifo, &e));
    CATCH_REQUIRE(e.value1 == i);
  } /* for() */
  CATCH_REQUIRE(fifo_isempty(fifo));
  T e;
  CATCH_REQUIRE(fifo_remove(fifo, &e) == ERROR);
  fifo_destroy(fifo);
} /* rdwr_test() */

template<typename T>
static void map_test(int, struct fifo_params *  params) {
  struct fifo *fifo;
  struct fifo myfifo;

  fifo = fifo_init(&myfifo, params);
  CATCH_REQUIRE((nullptr != fifo));

  /*
   * Stub--this is already tested in rbuffer test; this is only here for code
   * coverage.
   */
  CATCH_REQUIRE(ERROR == fifo_map(fifo, nullptr));

  fifo_destroy(fifo);
} /* map_test() */

template<typename T>
static void inject_test(int, struct fifo_params *  params) {
  struct fifo *fifo;
  struct fifo myfifo;

  fifo = fifo_init(&myfifo, params);
  CATCH_REQUIRE(nullptr != fifo);

  /*
   * Stub--this is already tested in rbuffer test; this is only here for code
   * coverage.
   */
  CATCH_REQUIRE(ERROR == fifo_inject(fifo, nullptr, nullptr));

  fifo_destroy(fifo);
} /* inject_test() */

template<typename T>
static void print_test(int len, struct fifo_params *  params) {
  struct fifo *fifo;
  struct fifo myfifo;

  fifo = fifo_init(&myfifo, params);
  CATCH_REQUIRE(nullptr != fifo);

  th::element_generator<T> g(gen_elt_type::ekINC_VALS, params->max_elts);

  for (int i = 0; i < len * 2; i++) {
    T e = g.next();
    if (i < len) {
      CATCH_REQUIRE(fifo_add(fifo, &e) == OK);
    } else {
      CATCH_REQUIRE(fifo_add(fifo, &e) == ERROR);
    }
  } /* for() */


  /* verify fifo contents */
  for (int i = 0; i < len; ++i) {
    T e;
    CATCH_REQUIRE(OK == fifo_remove(fifo, &e));
    CATCH_REQUIRE(e.value1 == i);
  } /* for() */

  fifo_print(nullptr);
  fifo_print(fifo);
  fifo_destroy(fifo);
} /* print_test() */

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("RDWR Test", "[ds][fifo]") {
  run_test<element8>(rdwr_test<element8>);
  run_test<element4>(rdwr_test<element4>);
  run_test<element2>(rdwr_test<element2>);
  run_test<element1>(rdwr_test<element1>);
}
CATCH_TEST_CASE("Map Test", "[ds][fifo]") {
  run_test<element8>(map_test<element8>);
  run_test<element4>(map_test<element4>);
  run_test<element2>(map_test<element2>);
  run_test<element1>(map_test<element1>);
}
CATCH_TEST_CASE("Inject Test", "[ds][fifo]") {
  run_test<element8>(inject_test<element8>);
  run_test<element4>(inject_test<element4>);
  run_test<element2>(inject_test<element2>);
  run_test<element1>(inject_test<element1>);
}
CATCH_TEST_CASE("Print Test", "[ds][fifo]") {
  run_test<element8>(print_test<element8>);
}
