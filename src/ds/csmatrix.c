/**
 * \file csmatrix.c
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/csmatrix.h"

#define RCSW_ER_MODNAME RCSW_ER_MODNAME_BUILDER("rcsw", "ds", "csmat")
#define RCSW_ER_MODID ekLOG4CL_DS_CSMATRIX
#include "rcsw/algorithm/sort.h"
#include "rcsw/common/fpc.h"
#include "rcsw/er/client.h"
#include "rcsw/common/alloc.h"

/*******************************************************************************
 * Structure Definitions
 ******************************************************************************/
struct RCSW_ATTR(packed, aligned (sizeof(dptr_t))) col_pair {
  int row;
  int inner_index;
};

/*******************************************************************************
 * Private Functions
 ******************************************************************************/
/**
 * \brief Print a matrix entry
 *
 * \param matrix The matrix handle.
 * \param e1 The matrix entry.
 */
static void csmatrix_entry_print(const struct csmatrix* const matrix,
                                 const void* const e1) {
  switch (matrix->type) {
    case ekCSMATRIX_INT:
      DPRINTF("%d", *(const int*)e1);
      break;
    case ekCSMATRIX_FLOAT:
      DPRINTF("%f", *(const float*)e1);
      break;
    case ekCSMATRIX_DOUBLE:
      DPRINTF("%f", *(const double*)e1);
      break;
    default:
      break;
  } /* switch() */
} /* csmatrix_entry_print() */

/**
 * \brief Multiply an entry in the matrix by something (must be of same data
 * type)
 *
 * \param matrix The matrix handle.
 * \param e1 The matrix entry.
 * \param e2 The other element.
 *
 * \return The result.
 */
static double csmatrix_entry_mult(const struct csmatrix* const matrix,
                                  const void* const e1,
                                  const void* const e2) {
  switch (matrix->type) {
    case ekCSMATRIX_INT:
      return *(const int*)e1 * *(const int*)e2;
      break;
    case ekCSMATRIX_FLOAT:
      return *(const float*)e1 * *(const float*)e2;
      break;
    case ekCSMATRIX_DOUBLE:
      return *(const double*)e1 * *(const double*)e2;
      break;
    default:
      return -1;
  } /* switch() */
} /* csmatrix_entry_mult() */

/**
 * \brief Divide an entry in the matrix by something (must be of same data
 * type)
 *
 * \param matrix The matrix handle.
 * \param e1 The matrix entry.
 * \param e2 The other element.
 *
 * \return The result.
 */
static double csmatrix_entry_div(const struct csmatrix* const matrix,
                                 const void* const e1,
                                 const void* const e2) {
  switch (matrix->type) {
    case ekCSMATRIX_INT:
      return *(const int*)e1 / *(const int*)e2;
      break;
    case ekCSMATRIX_FLOAT:
      return *(const float*)e1 / *(const float*)e2;
      break;
    case ekCSMATRIX_DOUBLE:
      return *(const double*)e1 / *(const double*)e2;
      break;
    default:
      return -1;
  } /* switch() */
} /* csmatrix_entry_div() */


/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS

struct csmatrix* csmatrix_init(struct csmatrix* const matrix_in,
                               const struct csmatrix_params* const params) {
  RCSW_FPC_NV(NULL, NULL != params);
  RCSW_ER_MODULE_INIT();

  struct csmatrix* matrix = rcsw_alloc(matrix_in,
                                       sizeof(struct csmatrix),
                                       params->flags & RCSW_NOALLOC_HANDLE);

  RCSW_CHECK_PTR(matrix);
  matrix->flags = params->flags;
  matrix->type = params->type;
  matrix->n_cols = params->n_cols;
  matrix->n_eff_cols = 0;

  struct darray_params inner_params = {
      .init_size = params->n_nz_elts,
      .cmpe = NULL,
      .printe = NULL,
      .max_elts = params->n_nz_elts,
      .elt_size = sizeof(int),
      .flags = RCSW_NOALLOC_HANDLE | RCSW_DS_ORDERED};
  RCSW_CHECK(NULL != darray_init(&matrix->inner_indices, &inner_params));
  struct darray_params count_params = {
    .init_size = params->n_rows + 1,

      .cmpe = NULL,
      .printe = NULL,
      .max_elts = params->n_rows + 1,
      .elt_size = sizeof(int),
      .flags = RCSW_NOALLOC_HANDLE | RCSW_DS_ORDERED};
  RCSW_CHECK(NULL != darray_init(&matrix->outer_starts, &count_params));
  RCSW_CHECK(OK == darray_set_size(&matrix->outer_starts, params->n_rows + 1));
  struct darray_params coeff_params = {
                       .init_size = params->n_nz_elts,
      .cmpe = NULL,
      .printe = NULL,
      .max_elts = params->n_nz_elts,
      .elt_size = csmatrix_type_size(matrix),
      .flags = RCSW_NOALLOC_HANDLE | RCSW_DS_ORDERED};

  RCSW_CHECK(NULL != darray_init(&matrix->values, &coeff_params));
  ER_DEBUG("n_rows=%zu n_nz_elts=%zu flags=0x%08x",
           csmatrix_n_rows(matrix),
           darray_size(&matrix->outer_starts),
           matrix->flags);

  matrix->csizes = rcsw_alloc(NULL,
                              params->n_cols * sizeof(int),
                              RCSW_NONE);

  RCSW_CHECK_PTR(matrix->csizes);

  /* size_t n_elts = params->n_nz_elts/params->n_cols*10; */
  /* matrix->nodes = calloc(matrix->n_cols, llist_meta_space(n_elts)); */
  /* matrix->elts = calloc(matrix->n_cols, */
  /*                       llist_element_space(n_elts, sizeof(struct
   * col_pair))); */
  /* RCSW_CHECK_PTR(matrix->nodes); */
  /* RCSW_CHECK_PTR(matrix->elts); */
  struct llist_params llist_params = {
    .cmpe = NULL,
    .printe = NULL,
    .max_elts = -1,
    .elt_size = sizeof(struct col_pair),
    .flags = RCSW_NOALLOC_HANDLE
  };
  matrix->cols = rcsw_alloc(NULL,
                            params->n_cols * sizeof(struct llist),
                            RCSW_NONE);

  for (size_t i = 0; i < matrix->n_cols; ++i) {
    /* llist_params.nodes = matrix->nodes + i* llist_meta_space(n_elts); */
    /* llist_params.elements = matrix->elts + */
    /*     i*llist_element_space(n_elts, sizeof(struct col_pair)); */
    RCSW_CHECK(NULL != llist_init(matrix->cols + i, &llist_params));
  } /* for(i..) */

  return matrix;

error:
  return NULL;
} /* csmatrix_init() */

void csmatrix_destroy(struct csmatrix* const matrix) {
  RCSW_FPC_V(NULL != matrix);
  darray_destroy(&matrix->inner_indices);
  darray_destroy(&matrix->outer_starts);
  darray_destroy(&matrix->values);

  if (matrix->cols) {
    for (size_t i = 0; i < matrix->n_cols; ++i) {
      llist_destroy(matrix->cols + i);
    } /* for(i..) */
    free(matrix->cols);
  }
  if (matrix->csizes) {
    free(matrix->csizes);
  }
  /* if (matrix->nodes) { */
  /*     free(matrix->nodes); */
  /* } */
  /* if (matrix->elts) { */
  /*     free(matrix->elts); */
  /* } */
  rcsw_free(matrix, matrix->flags & RCSW_NOALLOC_HANDLE);
} /* csmatrix_destroy() */

status_t csmatrix_entry_add(struct csmatrix* const matrix,
                            bool_t sequential_insertions,
                            size_t row,
                            size_t col,
                            const void* const e) {
  RCSW_FPC_NV(ERROR, NULL != matrix, NULL != e);

  /* entry already exists--nothing to do */
  if (!sequential_insertions &&
      -1 != csmatrix_inner_index_get(matrix, row, col)) {
    return OK;
  }

  /* Update inner indices */
  int* row_start = &csmatrix_outer_starts(matrix)[row];
  assert(row_start);
  size_t j = 0;
  int rsize = csmatrix_rsize(matrix, row);

  while ((rsize > 0) &&
         (int)col > csmatrix_inner_indices(matrix)[row_start[0] + j]) {
    rsize--;
    j++;
  } /* while() */

  ER_TRACE("Add entry [%zu, %zu]: (rstart=%d,rsize=%zu,inner_index=%zu)",
           row,
           col,
           row_start[0],
           csmatrix_rsize(matrix, row),
           row_start[0] + j);
  RCSW_CHECK(OK == darray_insert(&matrix->inner_indices, &col, row_start[0] + j));
  RCSW_CHECK(OK == darray_insert(&matrix->values, e, row_start[0] + j));

  /*
   * Update outer starts (must be after insertions or indices don't line up).
   *
   */
  int* start = csmatrix_outer_starts(matrix);
  if (!sequential_insertions) {
    for (size_t i = row + 1; i < darray_size(&matrix->outer_starts); ++i) {
      (*(start + i))++;
    } /* for(j..) */
  } else {
    /*
     * In order for subsequent insertions in the same row to work, the next
     * TWO entries in the outer_starts array must be populated
     */
    int index = *(int*)darray_data_get(&matrix->outer_starts, row + 1);
    index++;
    darray_data_set(&matrix->outer_starts, row + 1, &index);

    if (row + 1 < darray_size(&matrix->outer_starts)) {
      darray_data_set(&matrix->outer_starts, row + 2, &index);
    }
  }
  matrix->n_eff_cols = RCSW_MAX(col, matrix->n_eff_cols);
  struct col_pair pair = { .row = row, .inner_index = row_start[0] + j };
  RCSW_CHECK(OK == llist_append(&matrix->cols[col], &pair));

  matrix->csizes[col]++;
  return OK;

error:
  return ERROR;
} /* csmatrix_entry_add() */

int csmatrix_inner_index_get(const struct csmatrix* const matrix,
                             size_t row,
                             size_t col) {
  RCSW_FPC_NV(0, NULL != matrix, row < darray_size(&matrix->outer_starts));

  size_t row_start = *(int*)darray_data_get(&matrix->outer_starts, row);
  size_t rsize = csmatrix_rsize(matrix, row);

  /*
   * Search through the column indices for the specified column
   */
  size_t i;

  for (i = row_start; i < row_start + rsize; ++i) {
    if (*(int*)darray_data_get(&matrix->inner_indices, i) == (int)col) {
      return i;
    }
  } /* for(i..) */
  return -1;
} /* csmatrix_inner_index_get() */

status_t csmatrix_entry_set(struct csmatrix* const matrix,
                            size_t row,
                            size_t col,
                            const void* const e) {
  RCSW_FPC_NV(ERROR,
              NULL != matrix,
              NULL != e,
              row < darray_size(&matrix->outer_starts));

  int i = csmatrix_inner_index_get(matrix, row, col);
  RCSW_CHECK(-1 != i);
  RCSW_CHECK(OK == darray_data_set(&matrix->values, i, e));
  return OK;

error:
  return ERROR;
} /* csmatrix_entry_set() */

void* csmatrix_entry_get(const struct csmatrix* const matrix,
                         size_t row,
                         size_t col) {
  RCSW_FPC_NV(0, NULL != matrix, row < darray_size(&matrix->outer_starts));

  int index = csmatrix_inner_index_get(matrix, row, col);
  RCSW_CHECK(-1 != index);

  return darray_data_get(&matrix->values, index);

error:
  return NULL;
} /* csmatrix_entry_get() */

status_t
csmatrix_resize(struct csmatrix* const matrix, size_t n_rows, size_t n_nz_elts) {
  RCSW_FPC_NV(ERROR, NULL != matrix);

  /* RCSW_CHECK(OK == darray_resize(&matrix->outer_starts, n_rows+1)); */
  /* RCSW_CHECK(OK == darray_resize(&matrix->inner_indices, n_nz_elts)); */
  /* RCSW_CHECK(OK == darray_resize(&matrix->values, n_nz_elts)); */

  RCSW_CHECK(OK == darray_set_size(&matrix->outer_starts, n_rows + 1));
  RCSW_CHECK(OK == darray_set_size(&matrix->inner_indices, n_nz_elts));
  RCSW_CHECK(OK == darray_set_size(&matrix->values, n_nz_elts));
  return OK;

error:
  return ERROR;
} /* cs_matrix_resize() */

status_t csmatrix_calc_clists(struct csmatrix* const matrix) {
  RCSW_FPC_NV(ERROR, NULL != matrix);

  for (size_t i = 0; i < csmatrix_n_rows(matrix); ++i) {
    int* links = csmatrix_row(matrix, i);
    size_t rsize = csmatrix_rsize(matrix, i);
    for (size_t j = 0; j < rsize; ++j) {
      struct col_pair pair = { .row = i, .inner_index = links[0] + j };

      RCSW_CHECK(OK == llist_append(&matrix->cols[links[j]], &pair));
    } /* for(j..) */
  } /* for(i..) */

  return OK;

error:
  return ERROR;
} /* csmatrix_calc_clists() */

status_t csmatrix_vmult(const struct csmatrix* const matrix,
                        const struct darray* const vector_in,
                        struct darray* const vector_out) {
  /* RCSW_FPC_NV(ERROR, NULL != matrix, NULL != vector_in, NULL != vector_out, */
  /*           csmatrix_n_rows(matrix) == darray_size(vector_out), */
  /*           csmatrix_n_cols(matrix) == darray_size(vector_in)); */

  for (size_t i = 0; i < csmatrix_n_rows(matrix); ++i) {
    double res = 0;
    int row_start = csmatrix_outer_starts(matrix)[i];
    int row_end = csmatrix_outer_starts(matrix)[i + 1];

    int* cols = csmatrix_row(matrix, i);
    double* vals = csmatrix_values(matrix) + i;
    for (int j = 0; j < row_end - row_start; ++j) {
      res += csmatrix_entry_mult(
          matrix, vals + j, darray_data_get(vector_in, cols[j]));
      ER_TRACE("Multiply in[(%zu, %d) -> %zu]=%f * vector[%d]=%f = %f",
               i,
               cols[j],
               vals + j - csmatrix_values(matrix),
               vals[j],
               cols[j],
               *(double*)darray_data_get(vector_in, cols[j]),
               res);
    } /* for(j..) */
    RCSW_CHECK(OK == darray_data_set(vector_out, i, &res));
  } /* for(i..) */
  return OK;

error:
  return ERROR;
} /* csmatrix_vmult() */

status_t csmatrix_cols_normalize(struct csmatrix* const matrix) {
  RCSW_FPC_NV(ERROR, NULL != matrix);

  for (size_t i = 0; i < csmatrix_n_cols(matrix); ++i) {
    if ((i % 100000) == 0) {
      DPRINTF("Column %zu/%zu (%zu)\n",
              i,
              csmatrix_n_cols(matrix),
              llist_size(matrix->cols + i));
    }
    double total = 0;

    {
      LLIST_FOREACH(matrix->cols + i, next, col) {
        struct col_pair* pair = (struct col_pair*)col->data;
        double val = csmatrix_values(matrix)[pair->inner_index];
        total += val;
      }
    }

    LLIST_FOREACH(matrix->cols + i, next, col) {
      struct col_pair* pair = (struct col_pair*)col->data;
      double res = csmatrix_entry_div(
          matrix, csmatrix_entry_get(matrix, pair->row, i), &total);

      csmatrix_values(matrix)[pair->inner_index] = res;
    }
  } /* for(i..) */

  return OK;
} /* csmatrix_cols_normalize() */

struct csmatrix* csmatrix_transpose(struct csmatrix* const matrix) {
  RCSW_FPC_NV(NULL, NULL != matrix);

  struct csmatrix_params params = { .n_rows = csmatrix_n_cols(matrix),
                                    .n_nz_elts = csmatrix_size(matrix),
                                    .n_cols = csmatrix_n_rows(matrix),
                                    .type = matrix->type,
                                    .flags = 0 };
  struct csmatrix* new = csmatrix_init(NULL, &params);
  RCSW_CHECK_PTR(new);

  ER_DEBUG("TRANSPOSE: Sorting column lists");
  for (size_t i = 0; i < csmatrix_n_cols(matrix); ++i) {
    RCSW_CHECK(OK == llist_sort(matrix->cols + i, ekEXEC_REC));
  } /* for(i..) */
  ER_DEBUG("TRANSPOSE: Begin");
  for (size_t i = 0; i < csmatrix_n_cols(matrix); ++i) {
    if ((i % 100000) == 0) {
      DPRINTF("Column %zu/%zu (%zu)\n",
              i,
              csmatrix_n_cols(matrix),
              llist_size(matrix->cols + i));
    }

    LLIST_FOREACH(matrix->cols + i, next, col) {
      struct col_pair* pair = (struct col_pair*)col->data;
      double val = csmatrix_values(matrix)[pair->inner_index];
      ER_TRACE("TRANSPOSE: new[%zu, %d] = old[%d, %zu] (%f)",
           i,
           pair->row,
           pair->row,
           i,
           val);
      RCSW_CHECK(OK == csmatrix_entry_add(new, true, i, pair->row, &val));
    }
  } /* for(i..) */

  return new;

error:
  return NULL;
} /* csmatrix_transpose() */

void csmatrix_print(const struct csmatrix* matrix) {
  if (NULL == matrix) {
    return;
  }
  DPRINTF("{");
  for (size_t i = 0; i < csmatrix_n_rows(matrix); ++i) {
    DPRINTF("{");
    for (size_t j = 0; j < csmatrix_n_cols(matrix); ++j) {
      /* row i has a column j */
      if (-1 != csmatrix_inner_index_get(matrix, i, j)) {
        /* DPRINTF("(%zu,%zu) exists\n", i, j); */
        csmatrix_entry_print(matrix, csmatrix_entry_get(matrix, i, j));
      } else {
        double val = 0.0;
        csmatrix_entry_print(matrix, &val);
      }
      if (j < csmatrix_n_cols(matrix) - 1) {
        DPRINTF(", ");
      }
    } /* for(j..) */
    DPRINTF("}");
    if (i < csmatrix_n_rows(matrix) - 1) {
      DPRINTF("\n");
    }
  } /* for(i..) */
  DPRINTF("}\n");
} /* csmatrix_print() */


END_C_DECLS
