/**
 * \file
 *
 * \copyright 2026 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * Shared helpers for the algorithm unit tests.
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "tests/unit/algorithm/alg_test.hpp"

/*******************************************************************************
 * Helpers
 ******************************************************************************/
namespace th {

int cmp_first_int(const void* const a, const void* const b) {
  int x, y;
  memcpy(&x, a, sizeof(x));
  memcpy(&y, b, sizeof(y));
  return (x > y) - (x < y);
}

bool_t eq_int(const void* a, const void* b) {
  return *static_cast<const int*>(a) == *static_cast<const int*>(b);
}

size_t len_int(const void* s) {
  const int* p = static_cast<const int*>(s);
  size_t     n = 0;
  while (0 != p[n]) {
    ++n;
  }
  return n;
}
} /* namespace th */
