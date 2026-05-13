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
#define CATCH_CONFIG_PREFIX_ALL
#include <catch2/catch_test_macros.hpp>

#include "rcsw/ds/fifo.h"
#include "tests/ds/ds_test.hpp"

/*******************************************************************************
 * Namespaces/Decls
 ******************************************************************************/
using fifo_test_t = void (*)(int len, struct fifo_config *config);

/*******************************************************************************
 * Test Helper Functions
 ******************************************************************************/
template <typename T>
static void run_test(fifo_test_t test) {
  RCSW_ER_INIT(TH_ZLOG_CONF);

  struct fifo_config config;
  memset(&config, 0, sizeof(fifo_config));
  config.printe   = th::printe<T>;
  config.elt_size = sizeof(T);
  CATCH_REQUIRE(th::ds_init(&config) == OK);

  uint32_t flags[] = {
    RCSW_NONE,
    RCSW_ZALLOC,
    RCSW_NOALLOC_HANDLE,
    RCSW_NOALLOC_DATA,
  };

  /* Test each flag in isolation */
  for (size_t i = 0; i < RCSW_ARRAY_ELTS(flags); ++i) {
    for (int k = 1; k < TH_NUM_ITEMS; ++k) {
      config.flags    = flags[i];
      config.max_elts = k;
      test(k, &config);
    }
  }

  /* Test all pairwise combinations */
  for (size_t i = 0; i < RCSW_ARRAY_ELTS(flags); ++i) {
    for (size_t j = i + 1; j < RCSW_ARRAY_ELTS(flags); ++j) {
      uint32_t applied = flags[i] | flags[j];
      for (int k = 1; k < TH_NUM_ITEMS; ++k) {
        config.flags    = applied;
        config.max_elts = k;
        test(k, &config);
      }
    }
  }

  th::ds_shutdown(&config);
  RCSW_ER_DEINIT();
}

/*******************************************************************************
 * Test Functions
 ******************************************************************************/
template <typename T>
static void rdwr_test(int len, struct fifo_config *config) {
  struct fifo *fifo;
  struct fifo  myfifo;

  /* RCSW_NOALLOC_HANDLE requires a non-NULL handle */
  if (config->flags & RCSW_NOALLOC_HANDLE) {
    CATCH_REQUIRE(nullptr == fifo_init(nullptr, config));
  }
  fifo = fifo_init(&myfifo, config);
  CATCH_REQUIRE(nullptr != fifo);

  /* Verify empty-state invariants before any inserts */
  CATCH_REQUIRE(fifo_isempty(fifo));
  CATCH_REQUIRE(fifo_size(fifo) == 0);
  {
    T dummy;
    CATCH_REQUIRE(fifo_remove(fifo, &dummy) == ERROR);
  }

  th::element_generator<T> g(th::gen_elt_type::ekINC_VALS, config->max_elts);

  /* Fill to capacity; attempts beyond capacity must fail */
  for (int i = 0; i < len * 2; ++i) {
    T e = g.next();
    if (i < len) {
      CATCH_REQUIRE(fifo_add(fifo, &e) == OK);
    } else {
      CATCH_REQUIRE(fifo_add(fifo, &e) == ERROR);
    }
  }

  CATCH_REQUIRE(!fifo_isempty(fifo));
  CATCH_REQUIRE((int)fifo_size(fifo) == len);
  fifo_print(fifo);

  /* Drain and verify FIFO ordering (ekINC_VALS → 0, 1, 2, ...) */
  for (int i = 0; i < len; ++i) {
    T e;
    CATCH_REQUIRE(fifo_remove(fifo, &e) == OK);
    CATCH_REQUIRE(e.value1 == i);
  }

  CATCH_REQUIRE(fifo_isempty(fifo));
  {
    T dummy;
    CATCH_REQUIRE(fifo_remove(fifo, &dummy) == ERROR);
  }

  fifo_destroy(fifo);
}

template <typename T>
static void map_test(int, struct fifo_config *config) {
  struct fifo myfifo;
  struct fifo *fifo = fifo_init(&myfifo, config);
  CATCH_REQUIRE(nullptr != fifo);

  /*
   * map/inject logic lives in rbuffer and is tested there; here we only
   * verify that a NULL callback returns ERROR through the fifo wrapper.
   */
  CATCH_REQUIRE(fifo_map(fifo, nullptr) == ERROR);

  fifo_destroy(fifo);
}

template <typename T>
static void inject_test(int, struct fifo_config *config) {
  struct fifo myfifo;
  struct fifo *fifo = fifo_init(&myfifo, config);
  CATCH_REQUIRE(nullptr != fifo);

  /* See map_test comment above. */
  CATCH_REQUIRE(fifo_inject(fifo, nullptr, nullptr) == ERROR);

  fifo_destroy(fifo);
}

template <typename T>
static void print_test(int len, struct fifo_config *config) {
  struct fifo *fifo;
  struct fifo  myfifo;

  /* Coverage: NULL handle must not crash */
  fifo_print(nullptr);

  fifo = fifo_init(&myfifo, config);
  CATCH_REQUIRE(nullptr != fifo);

  /* Print empty */
  fifo_print(fifo);

  th::element_generator<T> g(th::gen_elt_type::ekINC_VALS, config->max_elts);
  for (int i = 0; i < len; ++i) {
    T e = g.next();
    CATCH_REQUIRE(fifo_add(fifo, &e) == OK);
  }

  /* Print populated */
  fifo_print(fifo);
  fifo_destroy(fifo);
}

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("fifo RDWR Test", "[ds][fifo]") {
  run_test<element8>(rdwr_test<element8>);
  run_test<element4>(rdwr_test<element4>);
  run_test<element2>(rdwr_test<element2>);
  run_test<element1>(rdwr_test<element1>);
}
CATCH_TEST_CASE("fifo Map Test", "[ds][fifo]") {
  run_test<element8>(map_test<element8>);
  run_test<element4>(map_test<element4>);
  run_test<element2>(map_test<element2>);
  run_test<element1>(map_test<element1>);
}
CATCH_TEST_CASE("fifo Inject Test", "[ds][fifo]") {
  run_test<element8>(inject_test<element8>);
  run_test<element4>(inject_test<element4>);
  run_test<element2>(inject_test<element2>);
  run_test<element1>(inject_test<element1>);
}
CATCH_TEST_CASE("fifo Print Test", "[ds][fifo]") {
  run_test<element8>(print_test<element8>);
}
