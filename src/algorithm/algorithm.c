/**
 * \file algorithm.c
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/algorithm/algorithm.h"

#include <stdio.h>

#include "rcsw/common/dbg.h"
#include "rcsw/common/fpc.h"
#include "rcsw/utils/utils.h"

/*******************************************************************************
 * API Functions
 ******************************************************************************/
size_t alg_arr_largest_num(const size_t* const array, size_t n_elts) {
  RCSW_FPC_NV(-1, NULL != array);
  size_t largest = 0;

  for (size_t i = 0; i < n_elts; i++) {
    if (array[i] > largest) {
      largest = array[i];
    }
  } /* for(i..) */
  return largest;
} /* largest_num() */

bool_t str_is_parenthesizable(const char* const x,
                              char* const r,
                              char el,
                              char (*multiply_cb)(char x, char y)) {
  RCSW_FPC_NV(FALSE, NULL != x, NULL != r, NULL != multiply_cb);

  size_t len = strlen(x);
  memset(r, '\0', len * len);

  /*
   * First, initialize the diagonal of the result matrix with the elements of
   * x. These are the base cases from which successively larger subproblems
   * will be built.
   */
  for (size_t i = 0; i < len; i++) {
    r[len * i + i] = x[i];
  } /* for(i..) */

  for (size_t i = 1; i <= len; i++) { /* starting subsequence index */
    for (size_t j = 0; j <= len - i; j++) { /* subsequence length */
      size_t k = j + i - 1;
      for (int q = (int)j; q <= (int)k - 1; q++) {
        if (el ==
            multiply_cb(r[j + (size_t)q * len], r[(size_t)q + 1 + len * k])) {
          r[j + len * k] = el;
        }
      } /* for(q=j)... */
    } /* for(j=1)... */
  } /* for(i=1)... */
  return (bool_t)(r[0 * len + len] == el);
} /* str_is_parenthesizable() */
