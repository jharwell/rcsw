/**
 * \file lcs.c
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/algorithm/lcs.h"
#include "rcsw/common/dbg.h"
#include "rcsw/common/fpc.h"

/*******************************************************************************
 * Forward Declarations
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Recursive subroutine to compute LCS of x and y using memoization
 *
 * \param x Sequence #1
 * \param y Sequence #2
 * \param c The memoization table
 * \param i Current index in sequence #1
 * \param j Current index in sequence #2
 * \param length The length of sequence #1 (X)
 *
 * \return  LCS(x,y)
 */
static int lcs_rec_sub(const char* x,
                       const char* y,
                       int* c,
                       size_t i,
                       size_t j,
                       size_t length);

/*******************************************************************************
 * API Functions
 ******************************************************************************/
status_t lcs_init(struct lcs_calculator* lcs, const char* x, const char* y) {
  RCSW_FPC_NV(ERROR, NULL != lcs, NULL != x, NULL != y);

  lcs->len_x = strlen(x);
  lcs->len_y = strlen(y);
  lcs->x = x;
  lcs->y = y;
  lcs->size = 0;
  lcs->results = malloc((lcs->len_x + 1) * (lcs->len_y + 1) * sizeof(int));
  RCSW_CHECK_PTR(lcs->results);
  memset(lcs->results, -1, (lcs->len_x + 1) * (lcs->len_y + 1) * sizeof(int));
  return OK;

error:
  return ERROR;
} /* lcs_init() */

void lcs_destroy(struct lcs_calculator* lcs) {
  if (!lcs) {
    return;
  }
  if (lcs->results) {
    free(lcs->results);
  }
  if (lcs->sequence) {
    free(lcs->sequence);
  }
} /* lcs_destroy() */

int lcs_rec(const struct lcs_calculator* lcs) {
  RCSW_CHECK_PTR(lcs);
  return lcs_rec_sub(lcs->x, lcs->y, lcs->results, 0, 0, lcs->len_x);
error:
  return -1;
} /* lcs_rec() */

int lcs_iter(struct lcs_calculator* lcs) {
  RCSW_CHECK_PTR(lcs);

  for (size_t i = 0; i <= lcs->len_x; ++i) {
    for (size_t j = 0; j <= lcs->len_y; ++j) {
      if (0 == i || 0 == j) {
        lcs->results[i * lcs->len_x + j] = 0;
      } else if (lcs->x[i - 1] == lcs->y[j - 1]) {
        lcs->results[i * lcs->len_x + j] =
            lcs->results[(i - 1) * lcs->len_x + j - 1] + 1;
      } else {
        lcs->results[i * lcs->len_x + j] =
            RCSW_MAX(lcs->results[(i - 1) * lcs->len_x + j],
                lcs->results[i * lcs->len_x + j - 1]);
      }
    } /* for(i=0...) */
  }   /* for(j=0...) */
  lcs->size = (size_t)lcs->results[lcs->len_x * lcs->len_x + lcs->len_y];

  lcs->sequence = malloc((lcs->size + 1) * sizeof(char));
  lcs->sequence[lcs->size] = '\0';
  size_t index = lcs->size;

  /*
   * Start from the right-most-bottom-most corner and
   * one by one store characters in lcs[]
   */
  size_t i = lcs->len_x, j = lcs->len_y;
  while (i > 0 && j > 0) {
    /*
     * If current character in X[] and Y are same, then current character is
     * part of LCS
     */
    if (lcs->x[i - 1] == lcs->y[j - 1]) {
      lcs->sequence[index - 1] = lcs->x[i - 1];
      i--;
      j--;
      index--;
    }
    /*
     * If not same, then find the larger of two and go in the direction of
     * larger value
     */
    else if (lcs->results[(i - 1) * lcs->len_x + j] >
             lcs->results[i * lcs->len_x + j - 1]) {
      i--;
    } else {
      j--;
    }
  } /* while() */
  return OK;

error:
  return ERROR;
} /* lcs_iter() */

/*******************************************************************************
 * Static Functions
 ******************************************************************************/
static int lcs_rec_sub(const char* x,
                       const char* y,
                       int* c,
                       size_t i,
                       size_t j,
                       size_t length) {
  /*
   * Base case: one of the strings has no characters: no possible match, so
   * the LCS is 0
   */
  if (x[i] == '\0' || y[j] == '\0') {
    return 0;
  }

  /*
   * If we don't have a memoized solution, we need to run the recursive
   * solver.
   */
  if (c[i * length + j] < 0) {
    /*
     * If x[i] and y[j] match, the LCS is the LCS of the i+1 and j+1 +
     * substrings, + 1 for the current char.
     *
     * If x[i] and y[j] do not match, then the LCS is the max of the LCS of x
     * and the j+1 substring of y and the LCS of y and the i+1 substring of x.
     */
    if (x[i] == y[j]) {
      c[i * length + j] = lcs_rec_sub(x, y, c, i + 1, j + 1, length) + 1;
    } else {
      c[i * length + j] = RCSW_MAX(lcs_rec_sub(x, y, c, i + 1, j, length),
                              lcs_rec_sub(x, y, c, i, j + 1, length));
    }
  }
  return c[i * length + j];
} /* lcs_rec_sub() */

END_C_DECLS
