/**
 * \file mpi_spmv_mult.c
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/multiprocess/mpi_spmv_mult.h"

#define RCSW_ER_MODNAME "rcsw.mp"
#define RCSW_ER_MODID M_MULTIPROCESS
#include "rcsw/er/client.h"

/*******************************************************************************
 * Structure Definitions
 ******************************************************************************/
struct spmv_comm_data {
  int src_vertex;
  int sink_vertex;
  double value;
};

/*******************************************************************************
 * Forward Declarations
 ******************************************************************************/
static status_t mpi_spmv_alloc_init(struct mpi_spmv_mult* const mult);

BEGIN_C_DECLS

/*******************************************************************************
 * API Functions
 ******************************************************************************/
struct mpi_spmv_mult*
mpi_spmv_mult_init(const struct mpi_spmv_mult_params* const params) {
  RCSW_FPC_NV(NULL, NULL != params);
  RCSW_ER_MODULE_INIT();

  ER_DEBUG("Initializing sparse matrix -> vector multiplier");

  /* All MPI processes perform the same basic initialization */
  struct mpi_spmv_mult* mult = malloc(sizeof(struct mpi_spmv_mult));
  RCSW_CHECK_PTR(mult);

  mult->matrix = params->matrix;
  mult->mpi_rank = params->mpi_rank;
  mult->mpi_world_size = params->mpi_world_size;
  mult->vector_in = NULL;

  /* Broadcast # rows/columns in matrix */
  if (0 == mult->mpi_rank) {
    mult->n_rows_init = csmatrix_n_rows(mult->matrix);
    mult->n_cols_init = csmatrix_n_cols(mult->matrix);
  }
  RCSW_CHECK(MPI_SUCCESS ==
             MPI_Bcast(&mult->n_rows_init, 1, MPI_INT, 0, MPI_COMM_WORLD));
  RCSW_CHECK(MPI_SUCCESS ==
             MPI_Bcast(&mult->n_cols_init, 1, MPI_INT, 0, MPI_COMM_WORLD));

  if (0 == mult->mpi_rank) {
    RCSW_CHECK_PTR(mult->matrix);

    /* To make dividing up the work easy  */
    RCSW_CHECK(csmatrix_n_cols(mult->matrix) % mult->mpi_world_size == 0);

    mult->rank_alloc_elts = calloc(mult->mpi_world_size, sizeof(int));
    RCSW_CHECK_PTR(mult->rank_alloc_elts);

    mult->rank_alloc_rows = calloc(mult->mpi_world_size, sizeof(int));
    RCSW_CHECK_PTR(mult->rank_alloc_rows);

    mult->rank_alloc_row_prefix_sums = calloc(mult->n_rows_init, sizeof(int));
    RCSW_CHECK_PTR(mult->rank_alloc_row_prefix_sums);

    mult->row_sizes = calloc(mult->n_rows_init, sizeof(int));
    RCSW_CHECK_PTR(mult->row_sizes);
  } else {
    mult->rank_alloc_elts = NULL;
    mult->rank_alloc_rows = NULL;
    mult->rank_alloc_row_prefix_sums = NULL;
    mult->row_sizes = NULL;
  }

  mult->row_owners = calloc(mult->n_rows_init + 2, sizeof(int));
  RCSW_CHECK_PTR(mult->row_owners);

  ER_DEBUG("Rank%d: world_size=%d n_rows_init=%d",
       mult->mpi_rank,
       mult->mpi_world_size,
       mult->n_rows_init);
  return mult;

error:
  mpi_spmv_mult_destroy(mult);
  return NULL;
} /* mpi_spmv_mult_init() */

void mpi_spmv_mult_destroy(struct mpi_spmv_mult* mult) {
  if (mult) {
    if (mult->row_owners) {
      free(mult->row_owners);
    }
    if (mult->rank_alloc_elts && mult->mpi_rank == 0) {
      free(mult->rank_alloc_elts);
    }
    if (mult->rank_alloc_rows && mult->mpi_rank == 0) {
      free(mult->rank_alloc_rows);
    }
    if (mult->rank_alloc_row_prefix_sums && mult->mpi_rank == 0) {
      free(mult->rank_alloc_row_prefix_sums);
    }
    if (mult->row_sizes && mult->mpi_rank == 0) {
      free(mult->row_sizes);
    }
    if (mult->vector_in) {
      darray_destroy(mult->vector_in);
    }
    if (mult->vector_out) {
      darray_destroy(mult->vector_out);
    }

    if (mult->matrix) {
      csmatrix_destroy(mult->matrix);
    }
    MPI_Type_free(&mult->spmv_comm_type);
    free(mult);
  }
  return;
} /* mpi_spmv_mult_destroy() */

status_t mpi_spmv_mult_ds_init(struct mpi_spmv_mult* const mult) {
  RCSW_FPC_NV(ERROR, NULL != mult);
  MPI_Barrier(MPI_COMM_WORLD);

  ER_DEBUG("Rank%d: Initializing data structures", mult->mpi_rank);
  RCSW_CHECK(OK == mpi_spmv_alloc_init(mult));

  if (0 != mult->mpi_rank) {
    ER_DEBUG("Rank%d: Initialize sparse matrix (%d x %d, %d elts)",
         mult->mpi_rank,
         mult->n_rows_alloc,
         mult->n_rows_init,
         mult->n_elts_alloc);
    /* initialize sparse matrix */
    struct csmatrix_params matrix_params = { .n_rows = mult->n_rows_alloc,
                                             .n_nz_elts = mult->n_elts_alloc,
                                             .n_cols = mult->n_cols_init,
                                             .type = CSMATRIX_DOUBLE,
                                             .flags = 0 };
    mult->matrix = csmatrix_init(NULL, &matrix_params);
    RCSW_CHECK_PTR(mult->matrix);
  }

  /* Initialize input/output vector at all nodes */
  ER_TRACE("Rank%d: Initialize input/output vector (%d/%d elts)",
       mult->mpi_rank,
       mult->n_cols_init,
       mult->n_rows_alloc);
  struct ds_params vector_params = {.type = {.da =
                                                 {
                                                     .init_size =
                                                         mult->n_cols_init,
                                                 }},
                                    .max_elts = mult->n_cols_init,
                                    .cmpe = NULL,
                                    .printe = NULL,
                                    .el_size = sizeof(double),
                                    .tag = DS_DARRAY,
                                    .flags = 0};

  if (0 != mult->mpi_rank) {
    mult->vector_in = darray_init(NULL, &vector_params);
    vector_params.type.da.init_size = mult->n_rows_alloc;
    vector_params.max_elts = mult->n_rows_alloc;
  } else {
    vector_params.type.da.init_size = mult->n_rows_init;
    vector_params.max_elts = mult->n_rows_init;
    mult->vector_in = NULL;
  }
  mult->vector_out = darray_init(NULL, &vector_params);
  RCSW_CHECK_PTR(mult->vector_out);

  /*
   * Now create main communication datatype used for sending/receiving
   * elements of the vector to multiply.
   */
  int count = 3;
  int blocklens[] = { 1, 1, 1 };
  MPI_Aint indices[3];
  indices[0] = (MPI_Aint)offsetof(struct spmv_comm_data, src_vertex);
  indices[1] = (MPI_Aint)offsetof(struct spmv_comm_data, sink_vertex);
  indices[2] = (MPI_Aint)offsetof(struct spmv_comm_data, value);
  MPI_Datatype old_types[] = { MPI_INT, MPI_INT, MPI_DOUBLE };
  RCSW_CHECK(MPI_SUCCESS ==
             MPI_Type_struct(
                 count, blocklens, indices, old_types, &mult->spmv_comm_type));
  RCSW_CHECK(MPI_SUCCESS == MPI_Type_commit(&mult->spmv_comm_type));
  return OK;

error:
  return ERROR;
} /* mpi_spmv_mult_ds_init() */

status_t mpi_spmv_mult_distribute(struct mpi_spmv_mult* const mult,
                                  struct darray* const vector) {
  RCSW_FPC_NV(ERROR, NULL != mult);
  ER_DEBUG("Rank%d: Distribute data to all ranks", mult->mpi_rank);
  /*
   * First, distribute the matrix to all ranks
   */
  if (0 == mult->mpi_rank) {
    /* send matrix data */
    int elt_accum = 0;
    int row_accum = 0;
    int owner = 0;
    for (int i = 0; i < mult->mpi_world_size; ++i) {
      int correction_factor = csmatrix_outer_starts(mult->matrix)[owner];
      /*
       * The outer_start elements sent to other ranks will not start at 0,
       * so correct for this by subtracting the value of the first element
       * from all elements that belong to rank i.
       *
       * Note the <= -- this is for the extra element at the end of
       * outer_starts
       */
      while (mult->row_owners[owner] != i + 1 &&
             owner <= (int)csmatrix_n_rows(mult->matrix)) {
        csmatrix_outer_starts(mult->matrix)[owner] -= correction_factor;
        owner++;
      }

      /* Note the +1 for the extra element at the end of outer_starts */
      ER_DEBUG("Send outer_starts[%d-%d] (%d)to rank %d",
           row_accum,
           row_accum + mult->rank_alloc_rows[i],
           mult->rank_alloc_rows[i] + 1,
           i);
      MPI_Request req;

      RCSW_CHECK(MPI_SUCCESS ==
                 MPI_Isend(csmatrix_outer_starts(mult->matrix) + row_accum,
                           mult->rank_alloc_rows[i] + 1,
                           MPI_INT,
                           i,
                           0,
                           MPI_COMM_WORLD,
                           &req));

      ER_DEBUG("Send inner_indices[%d-%d]  (%d) to rank %d",
           elt_accum,
           elt_accum + mult->rank_alloc_elts[i] - 1,
           mult->rank_alloc_elts[i],
           i);
      RCSW_CHECK(MPI_SUCCESS ==
                 MPI_Isend(csmatrix_inner_indices(mult->matrix) + elt_accum,
                           mult->rank_alloc_elts[i],
                           MPI_INT,
                           i,
                           0,
                           MPI_COMM_WORLD,
                           &req));

      ER_DEBUG("Send values[%d-%d] (%d) to rank %d ",
           elt_accum,
           elt_accum + mult->rank_alloc_elts[i] - 1,
           mult->rank_alloc_elts[i],
           i);

      RCSW_CHECK(MPI_SUCCESS ==
                 MPI_Isend(csmatrix_values(mult->matrix) + elt_accum,
                           mult->rank_alloc_elts[i],
                           MPI_DOUBLE,
                           i,
                           0,
                           MPI_COMM_WORLD,
                           &req));
      row_accum += mult->rank_alloc_rows[i];
      elt_accum += mult->rank_alloc_elts[i];
    } /* for(i..) */
  }
  ER_DEBUG("Rank%d: Receive matrix data", mult->mpi_rank);

  /* receive matrix data */
  RCSW_CHECK(MPI_SUCCESS == MPI_Recv(csmatrix_outer_starts(mult->matrix),
                                     mult->n_rows_alloc + 1,
                                     MPI_INT,
                                     0,
                                     0,
                                     MPI_COMM_WORLD,
                                     MPI_STATUS_IGNORE));

  RCSW_CHECK(MPI_SUCCESS == MPI_Recv(csmatrix_inner_indices(mult->matrix),
                                     mult->n_elts_alloc,
                                     MPI_INT,
                                     0,
                                     0,
                                     MPI_COMM_WORLD,
                                     MPI_STATUS_IGNORE));
  RCSW_CHECK(MPI_SUCCESS == MPI_Recv(csmatrix_values(mult->matrix),
                                     mult->n_elts_alloc,
                                     MPI_DOUBLE,
                                     0,
                                     0,
                                     MPI_COMM_WORLD,
                                     MPI_STATUS_IGNORE));

  ER_DEBUG("Rank%d: Receive matrix data...done", mult->mpi_rank);

  if (0 == mult->mpi_rank) {
    mult->vector_in = darray_copy(vector, NULL);
    RCSW_CHECK_PTR(mult->vector_in);
  }
  /*
   * Now that the master has sent the necessary data to the other ranks,
   * resize its matrix, which is currently full size, to the same size as
   * the other ranks. This reduction is safe to do, because all the
   * information that will be lost has already been sent to the ranks that
   * need it.
   */
  RCSW_CHECK(OK == csmatrix_resize(
                       mult->matrix, mult->n_rows_alloc, mult->n_elts_alloc));
  ER_TRACE("Rank%d: Broadcast row->rank mappings", mult->mpi_rank);

  /*
   * Finally, get the row -> rank mappings so you know where to
   * send/receive data to/from during execution
   */
  RCSW_CHECK(
      MPI_SUCCESS ==
      MPI_Bcast(mult->row_owners, mult->n_rows_init, MPI_INT, 0, MPI_COMM_WORLD));
  return OK;

error:
  return ERROR;
} /* mpi_spmv_mult_distribute() */

struct darray* mpi_spmv_mult_exec(struct mpi_spmv_mult* const mult) {
  RCSW_FPC_NV(ERROR, NULL != mult);
  /*
   * Send the vector to multiply with from the root to all other ranks. All
   * other ranks won't need ALL of the elements in the vector, but it will
   * need some unknown subset. It is much easier just to broadcast the whole
   * thing rather than trying to figure out who needs what exactly.
   */
  RCSW_CHECK(MPI_SUCCESS == MPI_Bcast(mult->vector_in->elements,
                                      mult->n_cols_init,
                                      MPI_DOUBLE,
                                      0,
                                      MPI_COMM_WORLD));

  RCSW_CHECK(OK == darray_set_n_elts(mult->vector_in, mult->n_cols_init));

  /*
   * Now do the actual multiply!
   */
  darray_set_n_elts(mult->vector_out, mult->n_rows_alloc);
  ER_DEBUG("Rank%d: Multiply: [%zu x %zu] x [%zu x 1] = [%zu x 1]",
       mult->mpi_rank,
       csmatrix_n_rows(mult->matrix),
       csmatrix_n_cols(mult->matrix),
       darray_n_elts(mult->vector_in),
       darray_n_elts(mult->vector_out));
  RCSW_CHECK(OK ==
             csmatrix_vmult(mult->matrix, mult->vector_in, mult->vector_out));

  /*
   * Consolidate the data back at the root
   */
  MPI_Gatherv(darray_data_get(mult->vector_out, 0),
              mult->n_rows_alloc,
              MPI_DOUBLE,
              darray_data_get(mult->vector_out, 0),
              mult->row_sizes,
              mult->rank_alloc_row_prefix_sums,
              MPI_DOUBLE,
              0,
              MPI_COMM_WORLD);
  if (0 == mult->mpi_rank) {
    darray_set_n_elts(mult->vector_out, mult->n_rows_init);
  }

  return mult->vector_out;

error:
  return NULL;
} /* mpi_spmv_mult_exec() */

/*******************************************************************************
 * Static Functions
 ******************************************************************************/
static status_t mpi_spmv_alloc_init(struct mpi_spmv_mult* const mult) {
  RCSW_FPC_NV(ERROR, NULL != mult);

  /*
   * Assign approximately n/p non-zero entries to each rank
   */
  if (0 == mult->mpi_rank) {
    size_t target_size = csmatrix_n_elts(mult->matrix) / mult->mpi_world_size;
    ER_DEBUG("Allocate ~ %zu/%zu elts to %d ranks",
         target_size,
         csmatrix_n_elts(mult->matrix),
         mult->mpi_world_size);
    int curr_row = 0;
    size_t curr_size = 0;
    for (int j = 0; j < mult->mpi_world_size; ++j) {
      size_t rsize = csmatrix_rsize(mult->matrix, curr_row);
      curr_size = 0;
      while (curr_size + rsize <= target_size) {
        rsize = csmatrix_rsize(mult->matrix, curr_row);
        mult->row_owners[curr_row] = j;
        curr_size += rsize;
        mult->rank_alloc_elts[j] = curr_size;
        mult->rank_alloc_rows[j]++;

        ER_TRACE("Rank%d owns row %d (%zu/%zu): %d alloc()ed",
             j,
             curr_row,
             curr_size,
             target_size,
             mult->rank_alloc_elts[j]);
        curr_row++;
      } /* while() */
    } /* for(j..) */
    while (curr_row < (int)csmatrix_n_rows(mult->matrix)) {
      mult->rank_alloc_rows[mult->mpi_world_size - 1]++;
      mult->rank_alloc_elts[mult->mpi_world_size - 1] +=
          csmatrix_rsize(mult->matrix, curr_row);
      mult->row_owners[curr_row] = mult->mpi_world_size - 1;

      curr_row++;
    } /* while() */

    /*
     * Compute size prefix sums and row sizes for all rows.
     */
    for (int i = 1; i <= mult->mpi_world_size; ++i) {
      size_t size = mult->rank_alloc_rows[i - 1];
      mult->rank_alloc_row_prefix_sums[i] =
          mult->rank_alloc_row_prefix_sums[i - 1] + size;
      mult->row_sizes[i - 1] = size;
    } /* for(i..) */
  }

  /* Receive allocation parameters from master */
  RCSW_CHECK(MPI_SUCCESS == MPI_Scatter(mult->rank_alloc_elts,
                                        1,
                                        MPI_INT,
                                        &mult->n_elts_alloc,
                                        1,
                                        MPI_INT,
                                        0,
                                        MPI_COMM_WORLD));
  RCSW_CHECK(MPI_SUCCESS == MPI_Scatter(mult->rank_alloc_rows,
                                        1,
                                        MPI_INT,
                                        &mult->n_rows_alloc,
                                        1,
                                        MPI_INT,
                                        0,
                                        MPI_COMM_WORLD));
  RCSW_CHECK(
      MPI_SUCCESS ==
      MPI_Bcast(mult->row_owners, mult->n_rows_init, MPI_INT, 0, MPI_COMM_WORLD));
  ER_DEBUG("Rank%d: Received allocation parameters: n_rows=%d n_elts=%d",
       mult->mpi_rank,
       mult->n_rows_alloc,
       mult->n_elts_alloc);
  return OK;

error:
  return ERROR;
} /* mpi_spmv_alloc_init() */
END_C_DECLS
