/**
 * \file search-test.cpp
 *
 * Test of simple searching library.
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
#include "rcsw/algorithm/search.h"
#include "rcsw/utils/mem.h"
#include "rcsw/algorithm/sort.h"

/*******************************************************************************
 * Test Functions
 ******************************************************************************/
template<typename T>
void test_bsearch(size_t n_elts) {
  th::element_set<T> data1(n_elts);
  data1.data_gen();

  qsort_rec(data1.elts.data(), 0, n_elts - 1, sizeof(T), th::cmpe<T>);

  for (int i = 0; i < (int)n_elts - 1; ++i) {
    CATCH_REQUIRE(bsearch_iter(data1.elts.data(),
                               &data1.elts[i],
                               th::cmpe<T>,
                               sizeof(T),
                               0,
                               data1.elts.size() - 1) != -1);
  } /* for(i..) */
}

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("Quick Search", "[alg][search]") {
  for (size_t i = 1; i < TH_NUM_ITEMS; ++i) {
    test_bsearch<element8>(i);
    test_bsearch<element4>(i);
    test_bsearch<element2>(i);
    test_bsearch<element1>(i);
  } /* for(i..) */
}
