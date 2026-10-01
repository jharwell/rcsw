/**
 * \file
 *
 * \copyright 2017 John Harwell
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/algorithm/mcm_opt.h"

#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>

#include "rcsw/core/alloc.h"
#include "rcsw/core/flags.h"
#include "rcsw/core/fpc.h"
#include "rcsw/er/client.h"

/*******************************************************************************
 * Private Functions
 ******************************************************************************/
BEGIN_C_DECLS
/**
 * \brief Print the optimal parenthesization to stdout
 *
 * Since you want m[1][n], call with i=1, j=length-1
 *
 * \param arr Array containing parenthesization info
 * \param i Current index into row dimension of matrix
 * \param j Current index into column dimension of matrix
 * \param length The length of the matrix chain
 *
 */
static void mcm_opt_print_parens(const uint64_t* arr,
                                 uint64_t        i,
                                 uint64_t        j,
                                 uint64_t        length) {
  if (i == j) {
    DPRINTF("A%" PRIu64, i);
  } else {
    DPRINTF("(");
    uint64_t k = arr[i + (length * j)];
    mcm_opt_print_parens(arr, i, k, length);
    mcm_opt_print_parens(arr, k + 1, j, length);
    DPRINTF(")");
  }
} /* mcm_opt_print_parens() */

/**
 * \brief Report optimal parenthesization
 *
 * Since you want m[1][n], call with i=1, j=length-1
 *
 * \param arr Array containing parenthesization info
 * \param i Current index into row dimension of matrix
 * \param j Current index into column dimension of matrix
 * \param length The length of the matrix chain
 * \param ordering Array containing indices of matrices in the chain, to be
 * filled
 * \param count How many positions in the ordering array have been filled so
 * far. Pass this as 0.
 *
 */
static void mcm_opt_report_parens(const uint64_t* arr,
                                  uint64_t        i,
                                  uint64_t        j,
                                  uint64_t        length,
                                  uint64_t*       ordering,
                                  uint64_t*       count) {
  if (i == j) {
  } else {
    uint64_t k = arr[i + (length * j)];
    mcm_opt_report_parens(arr, i, k, length, ordering, count);
    mcm_opt_report_parens(arr, k + 1, j, length, ordering, count);
    if (i == k) {
      ordering[*count] = k;
      *count += 1;
    }
    if (k + 1 == j) {
      ordering[*count] = k + 1;
      *count += 1;
    }
  }
} /* mcm_opt_report_parens() */

/*******************************************************************************
 * Public API
 ******************************************************************************/
status_t mcm_opt_init(struct mcm_optimizer* mcm,
                      const uint64_t*       matrices,
                      uint64_t              size) {
  RCSW_FPC_NV(ERROR, NULL != mcm, NULL != matrices, size >= 2);
  mcm->matrices = matrices;
  mcm->size     = size;

  mcm->results = rcsw_alloc(NULL, size * size * sizeof(uint64_t), RCSW_ZALLOC);
  RCSW_CHECK_PTR(mcm->results);

  mcm->route = rcsw_alloc(NULL, size * size * sizeof(uint64_t), RCSW_ZALLOC);
  RCSW_CHECK_PTR(mcm->route);
  return OK;

error:
  mcm_opt_destroy(mcm);
  return ERROR;
} /* mcm_opt_init() */

void mcm_opt_destroy(struct mcm_optimizer* mcm) {
  RCSW_FPC_V(NULL != mcm);

  rcsw_free(mcm->results, RCSW_NONE);
  rcsw_free(mcm->route, RCSW_NONE);
} /* mcm_opt_destroy() */

status_t mcm_opt_optimize(struct mcm_optimizer* mcm) {
  RCSW_FPC_NV(ERROR, NULL != mcm);

  uint64_t n   = mcm->size - 1;
  uint64_t i   = 0;
  uint64_t j   = 0;
  uint64_t k   = 0;
  uint64_t q   = 0;
  uint64_t num = 0;

  /*
   * A[i][i]Only one matrix multiplication, so the number 0, M[i][i]=0; These
   * are the entries in the main diagonal of m.
   */
  for (i = 1; i < mcm->size; i++) {
    mcm->results[i + (mcm->size * i)] = 0;
  }
  /*
   * i represents the matrix chain length we are currently optimizing over. We
   *  start at two, because the cost of a chain length of 1 is 0 (see above)
   */
  for (i = 2; i <= n; i++) {
    /*
     * loop over all possible partition points k that result in a chain of
     * length i
     */
    for (j = 1; j <= n - i + 1; j++) {
      k = j + i - 1;
      /*
       * m[j][k] is the optimal results from j to k using the
       * optimal partitioning. Initialize to a very large #.
       */
      mcm->results[j + (k * mcm->size)] = UINT64_MAX;
      /*
       * Compute the cost of splitting the current problem at point
       * k. Cost is cost of sub-chain to left of k + cost of sub-chain to
       * right of k (at k+1), + a const # of scalar multiplications
       * derived from the dimensions of the two multiplied matrices.
       *
       * As you are looping through, if you find a better result update
       * m[i][j] accordingly.
       */
      for (q = j; q <= k - 1; q++) {
        num = mcm->results[j + (q * mcm->size)] +
              mcm->results[q + 1 + (mcm->size * k)] +
              mcm->matrices[j - 1] * mcm->matrices[q] * mcm->matrices[k];
        if (num < mcm->results[j + (mcm->size * k)]) {
          mcm->results[j + (mcm->size * k)] = num;
          mcm->route[j + (mcm->size * k)]   = q;
        }
      } /* for(q=j)... */
    } /* for(j=1)... */
  } /* for(i=2)... */
  mcm->min_mults = mcm->results[1 + (mcm->size * (mcm->size - 1))];
  return OK;
} /* mcm_opt_optimize() */

status_t mcm_opt_report(const struct mcm_optimizer* mcm, uint64_t* ordering) {
  RCSW_FPC_NV(ERROR, NULL != mcm, NULL != ordering);
  uint64_t count = 0;
  mcm_opt_report_parens(mcm->route,
                        1,
                        mcm->size - 1,
                        mcm->size,
                        ordering,
                        &count);
  return OK;
} /* mcm_opt_report() */

status_t mcm_opt_print(const struct mcm_optimizer* mcm) {
  RCSW_FPC_NV(ERROR, NULL != mcm);
  DPRINTF("Minimum scalar multiplications: %" PRIu64 "\n", mcm->min_mults);
  DPRINTF("Parenthesization:\n");
  mcm_opt_print_parens(mcm->route, 1, mcm->size - 1, mcm->size);
  DPRINTF("\n");
  return OK;
} /* mcm_opt_print() */

END_C_DECLS
