/**
 * \file edit_dist.c
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/algorithm/edit_dist.h"

#include "rcsw/common/fpc.h"
#include "rcsw/er/client.h"
#include "rcsw/common/alloc.h"
#include "rcsw/common/flags.h"

/*******************************************************************************
 * Private Functions
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Find min # of operation to convert A -> B recursively with memoization
 *
 * \param a String # 1
 * \param b String # 2
 * \param c The memoization table [strlen(a) x strlen(b)]
 * \param seq_len A callback to determine the length of a sequence.
 * \param cmpe Callback for comparing two elements for equality
 * \param elt_size Size of elements in bytes
 *
 * \return minimum # operations, or -1 if an error occurred
 */
static int edit_dist_rec_sub(const char* a,
                             const char* b,
                             int* c,
                             size_t i,
                             size_t j,
                             size_t length,
                             bool_t (*cmpe)(const void* e1, const void* e2),
                             size_t elt_size) {
  /* If we have memoized solution, return it */
  if (c[i * length + j] >= 0) {
    return c[i * length + j];
  }
  /*
   * If i or j is 0, then we have no choice but to insert all the characters
   * from the other string
   */
  if (0 == i) {
    return (int)j;
  } else if (0 == j) {
    return (int)i;
  }
  /*
   * Otherwise, if the last chars in a and b are the same (at the i-1 and j-1
   * index), then return the edit distance of the substrings. If the last
   * chars in a and are not the same, then return the minimum of what happens
   * if you substitute, delete, or insert chars from a to transform it into b.
   */
  if (true ==
      cmpe(((const uint8_t*)a) + (i - 1), ((const uint8_t*)b) + (j - 1))) {
    c[i * length + j] =
        edit_dist_rec_sub(a, b, c, i - 1, j - 1, length, cmpe, elt_size);
    return c[i * length + j];
  } else {
    c[i * length + j] =
        1 +
        RCSW_MIN3(
            edit_dist_rec_sub(
                a, b, c, i - 1, j - 1, length, cmpe, elt_size), /* substitute
                                                                */
            edit_dist_rec_sub(a, b, c, i - 1, j, length, cmpe, elt_size), /* delete
                                                                          */
            edit_dist_rec_sub(a, b, c, i, j - 1, length, cmpe, elt_size)); /* insert
                                                                           */
    return c[i * length + j];
  }
} /* edit_dist_rec_sub() */

/**
 * \brief Find min # of operations to convert A -> B
 *
 * \param a The first string
 * \param b The string to transform a into
 * \param c The memoization table
 * \param i Current index in a
 * \param j Current index in b
 *
 * \param length String length of a, so it does not have to be computed each
 *               recursive step.
 *
 * \param cmpe Callback for comparing two elements for equality
 * \param elt_size Size of elements in bytes
 *
 * \return # of operations, or -1 if an error occurred.
 */
static int edit_dist_rec(const char* a,
                         const char* b,
                         int* c,
                         size_t (*seq_len)(const void* seq),
                         bool_t (*cmpe)(const void* e1, const void* e2),
                         size_t elt_size) {
  RCSW_FPC_NV(-1, NULL != a, NULL != b, NULL != c);
  size_t len_x = seq_len(a);
  size_t len_y = seq_len(b);

  memset(c, -1, sizeof(int) * (len_x + 1) * (len_y + 1));
  return edit_dist_rec_sub(a, b, c, len_x, len_y, len_x, cmpe, elt_size);
} /* edit_dist_rec() */

/**
 * \brief Compute min # of operations to convert A -> B using
 * bottom up dynamic programming.
 *
 * \param a String # 1
 * \param b String # 2
 * \param c The memoization table [strlen(a) x strlen(b)]
 * \param seq_len A callback to determine the length of a sequence.
 * \param cmpe Callback for comparing two elements for equality
 * \param elt_size Size of elements in bytes
 *
 * \return min # of of operations, or -1 if an error occurred
 */
static int edit_dist_iter(const void* a,
                          const void* b,
                          int* c,
                          size_t (*seq_len)(const void* seq),
                          bool_t (*cmpe)(const void* e1, const void* e2),
                          size_t elt_size) {
  size_t m = seq_len(a);
  size_t n = seq_len(b);
  memset(c, -1, m * n * sizeof(int));

  for (size_t i = 0; i <= m; ++i) {
    for (size_t j = 0; j <= n; ++j) {
      if (0 == i) {
        c[i * m + j] = (int)j;
      } else if (0 == j) {
        c[i * m + j] = (int)i;
      } else if (true == cmpe(((const uint8_t*)a) + (i - 1) * elt_size,
                              ((const uint8_t*)b) + (j - 1) * elt_size)) {
        c[i * m + j] = c[(i - 1) * m + j - 1];
      } else {
        c[i * m + j] = 1 + RCSW_MIN3(c[(i - 1) * m + j - 1], /* substitute */
                                     c[(i - 1) * m + j], /* delete */
                                     c[(i)*m + j - 1]); /* insert */
      }
    } /* for(j..) */
  } /* for(i..) */

  return c[m * m + n];
} /* edit_dist_iter() */


/*******************************************************************************
 * API Functions
 ******************************************************************************/
status_t edit_dist_init(struct edit_dist_finder* finder,
                        const void* a,
                        const void* b,
                        size_t elt_size,
                        bool_t (*cmpe)(const void* e1, const void* e2),
                        size_t (*seq_len)(const void* seq)) {
  RCSW_FPC_NV(ERROR,
              NULL != finder,
              NULL != a,
              NULL != b,
              elt_size > 0,
              NULL != cmpe,
              NULL != seq_len);
  finder->seq_a = a;
  finder->seq_b = b;
  finder->elt_size = elt_size;
  finder->cmpe = cmpe;
  finder->seq_len = seq_len;

  size_t n_elts1 = finder->seq_len(a) + 1;
  size_t n_elts2 = finder->seq_len(b) + 1;
  finder->memoization  = rcsw_alloc(NULL,
                                    n_elts1 * n_elts2 * sizeof(int),
                                    RCSW_NONE);
  RCSW_CHECK_PTR(finder->memoization);

  return OK;

error:
  edit_dist_destroy(finder);
  return ERROR;
} /* edit_dist_init() */

void edit_dist_destroy(struct edit_dist_finder* finder) {
  RCSW_FPC_V(NULL != finder);

  rcsw_free(finder->memoization, RCSW_NONE);
} /* edit_dist_destroy() */

int edit_dist_find(struct edit_dist_finder* finder,
                   enum exec_type type) {
  RCSW_FPC_NV(-1, NULL != finder);
  switch (type) {
    case ekEXEC_REC:
      return edit_dist_rec(finder->seq_a,
                           finder->seq_b,
                           finder->memoization,
                           finder->seq_len,
                           finder->cmpe,
                           finder->elt_size);
    case ekEXEC_ITER:
      return edit_dist_iter(finder->seq_a,
                            finder->seq_b,
                            finder->memoization,
                            finder->seq_len,
                            finder->cmpe,
                            finder->elt_size);
    default:
      return -1;
  } /* switch() */
} /* edit_dist_find() */


END_C_DECLS
