/**
 * \file mpi_radix_sort.c
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/multiprocess/mpi_radix_sort.h"

#include <mpi.h>

#define RCSW_ER_MODNAME "rcsw.mp"
#define RCSW_ER_MODID M_MULTIPROCESS
#include "rcsw/er/client.h"
#include "rcsw/algorithm/algorithm.h"
#include "rcsw/algorithm/sort.h"
#include "rcsw/common/fpc.h"

/*******************************************************************************
 * Forward Declarations
 ******************************************************************************/
BEGIN_C_DECLS

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
static status_t mpi_radix_sorter_step(struct mpi_radix_sorter* const sorter,
                                      int digit);

/*******************************************************************************
 * API Functions
 ******************************************************************************/
struct mpi_radix_sorter*
mpi_radix_sorter_init(const struct mpi_radix_sorter_params* const params) {
  RCSW_FPC_NV(NULL, NULL != params, NULL != params->data);
  RCSW_ER_MODULE_INIT();

  /* All MPI processes perform the same basic initialization */
  struct mpi_radix_sorter* sorter = malloc(sizeof(struct mpi_radix_sorter));
  RCSW_CHECK_PTR(sorter);
  sorter->n_elts = params->n_elts;
  sorter->base = params->base;
  sorter->mpi_rank = params->mpi_rank;
  sorter->mpi_world_size = params->mpi_world_size;
  sorter->chunk_size = sorter->n_elts / sorter->mpi_world_size;

  /*
   * Allocate memory. The master needs more space for the prefix sums,
   * because it has to receive ALL prefix sums from workers
   */
  sorter->prefix_sums = malloc(sizeof(size_t) * (sorter->base + 1));
  RCSW_CHECK_PTR(sorter->prefix_sums);

  sorter->tmp_arr = malloc(sizeof(size_t) * sorter->n_elts);
  RCSW_CHECK_PTR(sorter->tmp_arr);

  sorter->cum_prefix_sums = malloc(sizeof(size_t) * sorter->mpi_world_size * 2);
  RCSW_CHECK_PTR(sorter->cum_prefix_sums);

  sorter->cum_data = params->data;
  sorter->data = malloc(sizeof(size_t) * sorter->chunk_size);
  RCSW_CHECK_PTR(sorter->data);

  ER_DEBUG("Rank %d: n_elts=%zu chunk_size=%zu base=%zu world_size=%d",
       sorter->mpi_rank,
       sorter->n_elts,
       sorter->chunk_size,
       sorter->base,
       sorter->mpi_world_size);
  return sorter;

error:
  mpi_radix_sorter_destroy(sorter);
  return NULL;
} /* mpi_radix_sorter_init() */

void mpi_radix_sorter_destroy(struct mpi_radix_sorter* const sorter) {
  if (sorter) {
    if (sorter->prefix_sums) {
      free(sorter->prefix_sums);
    }
    if (sorter->tmp_arr) {
      free(sorter->tmp_arr);
    }
    if (sorter->cum_prefix_sums) {
      free(sorter->cum_prefix_sums);
    }
    if (sorter->data) {
      free(sorter->data);
    }
    free(sorter);
  }
} /* mpi_radix_sorter_destroy() */

status_t mpi_radix_sorter_exec(struct mpi_radix_sorter* const sorter) {
  DBGN("Rank %d: Starting radix sort", sorter->mpi_rank);
  memset(sorter->prefix_sums, 0, sizeof(size_t) * (sorter->base + 1));

  /*
   * Get largest # in array to get total # of digits. Only the master has the
   * input data, so you must broadcast to other workers.
   */
  int m;
  if (0 == sorter->mpi_rank) {
    m = alg_arr_largest_num(sorter->cum_data, sorter->n_elts);
  }
  RCSW_CHECK(MPI_SUCCESS == MPI_Bcast(&m, 1, MPI_INT, 0, MPI_COMM_WORLD));

  for (int exp = 1; m / exp > 0; exp *= sorter->base) {
    RCSW_CHECK(OK == mpi_radix_sorter_step(sorter, exp));
  } /* for(exp...) */

  return OK;

error:
  return ERROR;
} /* mpi_radix_sorter_exec() */

/*******************************************************************************
 * Static Functions
 ******************************************************************************/
static status_t mpi_radix_sorter_step(struct mpi_radix_sorter* const sorter,
                                      int digit) {
  ER_DEBUG("Rank %d: Radix sort digit %d", sorter->mpi_rank, digit);
  int prefix_recv_counts[sorter->mpi_world_size];
  int prefix_disp_counts[sorter->mpi_world_size];
  int data_recv_counts[sorter->mpi_world_size];
  int data_disp_counts[sorter->mpi_world_size];

  /* Send data to all processes */
  RCSW_CHECK(MPI_SUCCESS == MPI_Scatter(sorter->cum_data,
                                        sorter->chunk_size,
                                        MPI_INT,
                                        sorter->data,
                                        sorter->chunk_size,
                                        MPI_INT,
                                        0,
                                        MPI_COMM_WORLD));
  ER_TRACE("Rank %d: All data received (%zu bytes)",
       sorter->mpi_rank,
       sorter->n_elts);
  radix_counting_sort(
      sorter->data, sorter->tmp_arr, sorter->chunk_size, digit, sorter->base);

  /* compute prefix sums for current digit (all ranks) */
  radix_sort_prefix_sum(sorter->data,
                        sorter->chunk_size,
                        sorter->base,
                        digit,
                        sorter->prefix_sums + 1);

  ER_TRACE("Rank %d: Finished sorting", sorter->mpi_rank);

  /* receive prefix sums and data from workers*/
  for (size_t i = 0; i < (size_t)sorter->mpi_world_size; ++i) {
    prefix_recv_counts[i] = 2;
    prefix_disp_counts[i] = i * 2;
  } /* for(i..) */

  memset(data_recv_counts, 0, sizeof(size_t) * sorter->mpi_world_size);
  memset(data_disp_counts, 0, sizeof(size_t) * sorter->mpi_world_size);

  /*
   * You need to keep track of the total # of bytes received from workers up
   * through the current symbol (i.e. 0-9) you are processing, for both
   * the current and previous iterations, so that things end up in the right
   * place in the output buffer.
   */
  size_t total = 0;
  size_t prev_total = 0;
  for (size_t i = 0; i < sorter->base; ++i) {
    RCSW_CHECK(MPI_SUCCESS == MPI_Gatherv(sorter->prefix_sums + i,
                                          2,
                                          MPI_INT,
                                          sorter->cum_prefix_sums,
                                          prefix_recv_counts,
                                          prefix_disp_counts,
                                          MPI_INT,
                                          0,
                                          MPI_COMM_WORLD));
    ER_TRACE("Rank %d: Received prefixes for value %zu at master",
         sorter->mpi_rank,
         i);
    /* Only the master needs to compute recv counts/displacements  */
    if (0 == sorter->mpi_rank) {
      for (size_t j = 0; j < (size_t)sorter->mpi_world_size; ++j) {
        data_recv_counts[j] =
            sorter->cum_prefix_sums[2 * j + 1] - sorter->cum_prefix_sums[2 * j];
        total += data_recv_counts[j];
      } /* for(j..) */

      /*
       * The displacement for rank 0 is always the # of bytes received up
       * through the PREVIOUS symbol that was processed
       */
      data_disp_counts[0] = prev_total;

      /* The displacement for the rest of the ranks for the current digit
       * is computed by added the displacement for rank 0 for the current
       * symbol PLUS the # of bytes received by rank-1.
       */
      for (size_t j = 1; j < (size_t)sorter->mpi_world_size; ++j) {
        data_disp_counts[j] = data_disp_counts[j - 1] + data_recv_counts[j - 1];
      } /* for(j..) */
    }
    RCSW_CHECK(MPI_SUCCESS ==
               MPI_Gatherv(sorter->data + sorter->prefix_sums[i],
                           sorter->prefix_sums[i + 1] - sorter->prefix_sums[i],
                           MPI_INT,
                           sorter->cum_data,
                           data_recv_counts,
                           data_disp_counts,
                           MPI_INT,
                           0,
                           MPI_COMM_WORLD));
    ER_TRACE("Rank %d: Received data for value %zu to master", sorter->mpi_rank, i);
    prev_total = total;
  } /* for(i..) */
  return OK;
error:
  return ERROR;
} /* mpi_radix_sorter_step() */

END_C_DECLS
