/**
 * \file
 *
 * \copyright 2023 John Harwell
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#define CATCH_CONFIG_PREFIX_ALL
#include <cerrno>
#include <cstring>

#include <catch2/catch_test_macros.hpp>

#include "rcsw/ds/fifo.h"
#include "rcsw/ds/multififo.h"
#include "tests/unit/ds/ds_test.hpp"

/*******************************************************************************
 * Namespaces/Decls
 ******************************************************************************/
using multififo_test_t = void (*)(int len, struct multififo_config* config);

/*******************************************************************************
 * Test Helper Functions
 ******************************************************************************/
template <typename T>
static void run_test(multififo_test_t test) {
  RCSW_ER_INIT(TH_ZLOG_CONF);

  struct multififo_config config;
  size_t                  children[] = {1};
  memset(&config, 0, sizeof(multififo_config));
  config.flags      = 0;
  config.elt_size   = sizeof(T);
  config.n_children = 1;
  config.children   = children;
  CATCH_REQUIRE(th::ds_init(&config) == OK);

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
        config.flags    = applied;
        config.max_elts = k;
        test(k, &config);
      } /* for(k..) */

      applied &= ~flags[j];
    } /* for(j..) */
  } /* for(i..) */

  th::ds_shutdown(&config);

  RCSW_ER_DEINIT();
}

/*******************************************************************************
 * Test Functions
 ******************************************************************************/
template <typename T>
static void child_test(int len, struct multififo_config* config) {
  struct multififo* multififo;
  struct multififo  mymultififo;

  multififo = multififo_init(&mymultififo, config);
  CATCH_REQUIRE(nullptr != multififo);

  th::element_generator<T> g(th::gen_elt_type::PACKED_VALS, config->max_elts);

  for (int i = 0; i < len; i++) {
    T e = g.next();
    CATCH_REQUIRE(multififo_add(multififo, &e) == OK);
    CATCH_REQUIRE(multififo_remove(multififo, &e) == ERROR);
  } /* for() */

  struct fifo* child = &multififo->children.fifos[0];
  uint8_t      assembled_elt[sizeof(element8)];
  size_t       child_fifo_size = multififo->root.rb.elt_size / child->rb.elt_size;

  size_t n_elts = multififo_size(multififo);
  while (!multififo_isempty(multififo)) {
    CATCH_REQUIRE(fifo_size(child) == child_fifo_size);

    uint8_t* next = (uint8_t*)assembled_elt;
    for (size_t j = 0; j < child_fifo_size; ++j) {
      CATCH_REQUIRE(OK == fifo_remove(child, next));
      CATCH_REQUIRE(fifo_size(child) == child_fifo_size - j - 1);
      next += child->rb.elt_size;
    } /* for(j..) */
    T e;
    CATCH_REQUIRE(multififo_remove(multififo, &e) == OK);
    // NOLINTNEXTLINE(clang-analyzer-core.UndefinedBinaryOperatorResult)
    CATCH_REQUIRE(((T*)assembled_elt)->value1 == e.value1);
  } /* while(..) */

  multififo_destroy(multififo);
} /* child_test() */

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("CHILD Test", "[ds][multififo][noalloc]") {
  run_test<element8>(child_test<element8>);
  run_test<element4>(child_test<element4>);
  run_test<element2>(child_test<element2>);
}
CATCH_TEST_CASE(
  "multififo: RCSW_NOALLOC_DATA without RCSW_NOALLOC_META "
  "allocates child space from the heap",
  "[ds][multififo]") {
  static dptr_t           space[64];
  size_t                  children[] = {4};
  struct multififo_config c;
  memset(&c, 0, sizeof(c));
  c.elements          = space;
  c.meta              = nullptr; /* child space must then come from the heap */
  c.elt_size          = 8;
  c.max_elts          = 4;
  c.n_children        = 1;
  c.children          = children;
  c.flags             = RCSW_NOALLOC_DATA;
  struct multififo* f = multififo_init(nullptr, &c);
  CATCH_REQUIRE(nullptr != f);
  uint64_t v = 42;
  CATCH_REQUIRE(OK == multififo_add(f, &v));
  multififo_destroy(f);
}

CATCH_TEST_CASE(
  "multififo_init rejects more children than front_refmask "
  "can track",
  "[ds][multififo][noalloc]") {
  size_t                  children[9] = {4, 4, 4, 4, 4, 4, 4, 4, 4};
  struct multififo_config c;
  memset(&c, 0, sizeof(c));
  c.elt_size          = 8;
  c.max_elts          = 4;
  c.n_children        = 9;
  c.children          = children;
  c.flags             = RCSW_NONE;
  struct multififo* f = multififo_init(nullptr, &c);
  CATCH_REQUIRE(nullptr == f);
}

CATCH_TEST_CASE("multififo_init rejects a zero child element size",
                "[ds][multififo][noalloc]") {
  size_t                  children[] = {0};
  struct multififo_config c;
  memset(&c, 0, sizeof(c));
  c.elt_size   = 8;
  c.max_elts   = 4;
  c.n_children = 1;
  c.children   = children;
  /* a zero-size child cannot hold anything */
  CATCH_REQUIRE(nullptr == multififo_init(nullptr, &c));
}

CATCH_TEST_CASE(
  "multififo_add/remove on a busy multififo fail with EAGAIN "
  "and leave the lock to its holder",
  "[ds][multififo]") {
  size_t                  children[] = {4};
  struct multififo_config c;
  memset(&c, 0, sizeof(c));
  c.elt_size          = 8;
  c.max_elts          = 4;
  c.n_children        = 1;
  c.children          = children;
  struct multififo* f = multififo_init(nullptr, &c);
  CATCH_REQUIRE(nullptr != f);

  uint64_t v = 7;
  CATCH_REQUIRE(OK == multififo_add(f, &v));
  CATCH_REQUIRE(!multififo_islocked(f));

  f->locked = true; /* someone else is mid-operation */
  errno     = 0;
  CATCH_REQUIRE(ERROR == multififo_add(f, &v));
  CATCH_REQUIRE(EAGAIN == errno);
  errno = 0;
  CATCH_REQUIRE(ERROR == multififo_remove(f, nullptr));
  CATCH_REQUIRE(EAGAIN == errno);
  CATCH_REQUIRE(multififo_islocked(f)); /* not released by the failed calls */
  CATCH_REQUIRE(1 == multififo_size(f));

  f->locked = false;
  CATCH_REQUIRE(OK == multififo_add(f, &v));
  CATCH_REQUIRE(2 == multififo_size(f));
  multififo_destroy(f);
}
