/**
 * \file omp_radix_sort.c
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/multithread/omp_radix_sort.h"

#include <omp.h>

#define RCSW_ER_MODNAME "rcsw.mt"
#define RCSW_ER_MODID ekLOG4CL_MT_RADIX
#include "rcsw/er/client.h"
#include "rcsw/algorithm/algorithm.h"
#include "rcsw/algorithm/sort.h"
#include "rcsw/common/fpc.h"
#include "rcsw/common/alloc.h"

/*******************************************************************************
 * Forward Declarations
 ******************************************************************************/
/**
 * \brief Perform a single step of the radix sort algorithm; that is, sort the
 * next digit.
 *
 * \param sorter The sorting algorithm handle.
 * \param digit The current digit (1, 10, 100, 1000, etc. in base 10 for
 * example).
 *
 * \return \ref status_t.
 */
static status_t omp_radix_sorter_step(struct omp_radix_sorter* const sorter,
                                      int digit);
static void
omp_radix_sorter_first_touch_alloc(struct omp_radix_sorter* const sorter);

BEGIN_C_DECLS

/*******************************************************************************
 * API Functions
 ******************************************************************************/
struct omp_radix_sorter*
omp_radix_sorter_init(const struct omp_radix_sorter_params* const params) {
  RCSW_FPC_NV(NULL, NULL != params, NULL != params->data, params->base > 0);
  RCSW_ER_MODULE_INIT();

  struct omp_radix_sorter* sorter = rcsw_alloc(NULL,
                                               sizeof(struct omp_radix_sorter),
                                               RCSW_NONE);
  RCSW_CHECK_PTR(sorter);

  sorter->n_elts = params->n_elts;
  sorter->base = params->base;
  sorter->n_threads = params->n_threads;
  sorter->chunk_size = sorter->n_elts / sorter->n_threads;
  sorter->cum_prefix_sums = NULL;
  sorter->data = NULL;

  /*
   * Allocate memory. Make the FIFOs use a contiguous chunk of memory, rather
   * than each malloc()ing their own, to improve cache efficiency.
   */
  sorter->bins = rcsw_alloc(NULL,
                            sizeof(struct fifo) * sorter->n_threads * sorter->base,
                            RCSW_NONE);
  RCSW_CHECK_PTR(sorter->bins);
  sorter->data = rcsw_alloc(NULL,
                            sizeof(size_t) * sorter->n_elts,
                            RCSW_NONE);
  RCSW_CHECK_PTR(sorter->data);

  struct fifo_params impl_params = { .elt_size = sizeof(size_t),
                                   .max_elts = sorter->chunk_size,
                                   .elements = NULL,
                                   .flags = RCSW_NOALLOC_HANDLE };
  for (size_t i = 0; i < sorter->n_threads; ++i) {
    for (size_t j = 0; j < sorter->base; ++j) {
      RCSW_CHECK(NULL !=
                 fifo_init(&sorter->bins[i * sorter->base + j], &impl_params));
    } /* for(j..) */
  } /* for(i..) */

  sorter->cum_prefix_sums = rcsw_alloc(NULL,
                                       sizeof(size_t) * sorter->base * sorter->n_threads,
                                       RCSW_NONE);
  RCSW_CHECK_PTR(sorter->cum_prefix_sums);

  /* perform first touch allocation */
  omp_radix_sorter_first_touch_alloc(sorter);

  /* Now you can copy the data in and still get good memory page locality */
  for (size_t i = 0; i < sorter->n_elts; ++i) {
    sorter->data[i] = params->data[i];
  } /* for(i..) */

  ER_DEBUG("n_threads=%zu n_elts=%zu chunk_size=%zu base=%zu",
       sorter->n_threads,
       sorter->n_elts,
       sorter->chunk_size,
       sorter->base);
  return sorter;

error:
  omp_radix_sorter_destroy(sorter);
  return NULL;
} /* omp_radix_sorter_init() */

void omp_radix_sorter_destroy(struct omp_radix_sorter* const sorter) {
  RCSW_FPC_V(NULL != sorter);

  if (sorter->bins) {
    for (size_t i = 0; i < sorter->base * sorter->n_threads; ++i) {
      fifo_destroy(&sorter->bins[i]);
    } /* for(i..) */
    free(sorter->bins);
  }
  if (sorter->cum_prefix_sums) {
    free(sorter->cum_prefix_sums);
  }
  if (sorter->data) {
    free(sorter->data);
  }
  free(sorter);
} /* omp_radix_sorter_destroy() */

status_t omp_radix_sorter_exec(struct omp_radix_sorter* const sorter) {
  ER_INFO("Starting radix sort");

  int m;
  /* Get largest # in array to get total # of digits */
  m = alg_arr_largest_num(sorter->data, sorter->n_elts);
  memset(sorter->cum_prefix_sums,
         0,
         sizeof(size_t) * sorter->base * sorter->n_threads);

  for (int exp = 1; m / exp > 0; exp *= sorter->base) {
    RCSW_CHECK(OK == omp_radix_sorter_step(sorter, exp));
  } /* for(exp...) */
  ER_INFO("Finished sorting");
  return OK;

error:
  return ERROR;
} /* omp_radix_sorter_exec() */

/*******************************************************************************
 * Static Functions
 ******************************************************************************/
static status_t omp_radix_sorter_step(struct omp_radix_sorter* const sorter,
                                      int digit) {
  RCSW_FPC_NV(ERROR, NULL != sorter, sorter->base > 0);

  ER_INFO("Radix sort digit %d", digit);

/* Each thread flushes its own FIFOs */
#pragma omp parallel for num_threads(sorter->n_threads)
  for (size_t j = 0; j < sorter->n_threads; ++j) {
    for (size_t i = 0; i < sorter->base; ++i) {
      fifo_clear(&sorter->bins[j * sorter->base + i]);
    } /* for(i..) */
  } /* for(j..) */

/* Each worker sorts its own chunk of the data into bins (the FIFOs) */
#pragma omp parallel for num_threads(sorter->n_threads) schedule(static)
  for (size_t i = 0; i < sorter->n_elts; ++i) {
    fifo_add(sorter->bins + (i / sorter->chunk_size) * sorter->base +
                 ((sorter->data[i] / digit) % sorter->base),
             sorter->data + i);
  } /* for(i..) */
  ER_DEBUG("Finished sorting digit %d", digit);

  /*
   * Calculate all prefix sums for symbol 0 for all threads, to make math in
   * second loops cleaner.
   */
  for (size_t i = 1; i < sorter->n_threads; ++i) {
    sorter->cum_prefix_sums[i * sorter->base] =
        sorter->cum_prefix_sums[(i - 1) * sorter->base] +
        fifo_size(&sorter->bins[(i - 1) * sorter->base]);
  } /* for(i..) */

  /* Calculate all prefix sums for remaining symbols */
  for (size_t j = 1; j < sorter->base; ++j) {
    sorter->cum_prefix_sums[j] =
        sorter->cum_prefix_sums[(sorter->n_threads - 1) * sorter->base + j - 1] +
        fifo_size(
            &sorter->bins[(sorter->n_threads - 1) * sorter->base + j - 1]);
    for (size_t i = 1; i < sorter->n_threads; ++i) {
      sorter->cum_prefix_sums[j + i * sorter->base] =
          sorter->cum_prefix_sums[j + (i - 1) * sorter->base] +
          fifo_size(&sorter->bins[j + (i - 1) * sorter->base]);
    } /* for(i..) */
  } /* for(j..) */

  ER_TRACE("Computed all prefix sums");

/* all threads copy elements back into original array in parallel */
#pragma omp parallel for num_threads(sorter->n_threads) schedule(static)
  for (size_t j = 0; j < sorter->n_threads; ++j) {
    for (size_t i = 0; i < sorter->base; ++i) {
      struct fifo* f = &sorter->bins[j * sorter->base + i];
      size_t n_elts = 0;
      while (!fifo_isempty(f)) {
        fifo_remove(f,
                    &sorter->data[sorter->cum_prefix_sums[j * sorter->base + i] +
                                  n_elts++]);
      } /* while() */
    } /* for(i..) */
  } /* for(j..) */
  return OK;
} /* omp_radix_sorter_step() */

static void
omp_radix_sorter_first_touch_alloc(struct omp_radix_sorter* const sorter) {
#pragma omp parallel for num_threads(sorter->n_threads)
  for (size_t i = 0; i < sorter->n_elts; ++i) {
    sorter->data[i] = 0;
  } /* for(i..) */
} /* omp_radix_sorter_first_touch_alloc() */

END_C_DECLS
