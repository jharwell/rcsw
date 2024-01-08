/**
 * \file ds-multififo-utest.cpp
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

#include "rcsw/ds/multififo.h"
#include "rcsw/ds/fifo.h"
#include "rcsw/utils/utils.h"
#include "tests/ds_test.h"
#include "tests/ds_test.hpp"

/*******************************************************************************
 * Namespaces/Decls
 ******************************************************************************/
using multififo_test_t = void(*)(int len, struct multififo_params *params);

/*******************************************************************************
 * Test Helper Functions
 ******************************************************************************/
template<typename T>
static void run_test(multififo_test_t test) {
  RCSW_ER_INIT(TH_ZLOG_CONF);

  struct multififo_params params;
  size_t children[] = {1};
  memset(&params, 0, sizeof(multififo_params));
  params.flags = 0;
  params.elt_size = sizeof(T);
  params.n_children = 1;
  params.children = children;
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
static void child_test(int len, struct multififo_params *  params) {
  struct multififo *multififo;
  struct multififo mymultififo;

  multififo = multififo_init(&mymultififo, params);
  CATCH_REQUIRE(nullptr != multififo);

  th::element_generator<T> g(gen_elt_type::ekPACKED_VALS, params->max_elts);

  for (int i = 0; i < len; i++) {
    T e = g.next();
    CATCH_REQUIRE(multififo_add(multififo, &e) == OK);
    CATCH_REQUIRE(multififo_remove(multififo, &e) == ERROR);
  } /* for() */

  struct fifo* child = &multififo->children.fifos[0];
  uint8_t assembled_elt[sizeof(element8)];
  size_t child_fifo_size = multififo->root.elt_size / child->elt_size;

  size_t n_elts = multififo_size(multififo);
  while (!multififo_isempty(multififo)) {
    CATCH_REQUIRE(fifo_size(child) == child_fifo_size);

    uint8_t* next = (uint8_t*)assembled_elt;
    for (size_t j = 0; j < child_fifo_size; ++j) {
      CATCH_REQUIRE(OK == fifo_remove(child, next));
      CATCH_REQUIRE(fifo_size(child) == child_fifo_size - j - 1);
      next += child->elt_size;
    } /* for(j..) */
    T e;
    CATCH_REQUIRE(multififo_remove(multififo, &e) == OK);
    CATCH_REQUIRE(((T*)assembled_elt)->value1 == e.value1);
  } /* while(..) */

  multififo_destroy(multififo);
} /* child_test() */

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("CHILD Test", "[ds][multififo]") {
  run_test<element8>(child_test<element8>);
  run_test<element4>(child_test<element4>);
  run_test<element2>(child_test<element2>);
}
