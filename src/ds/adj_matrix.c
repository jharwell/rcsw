/**
 * \file adj_matrix.c
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/adj_matrix.h"

#define RCSW_ER_MODNAME "rcsw.ds.adj_matrix"
#define RCSW_ER_MODID M_DS_ADJ_MATRIX
#include "rcsw/er/client.h"

/*******************************************************************************
 * Static Functions
 ******************************************************************************/
BEGIN_C_DECLS
static void adj_matrix_printeu(const void* const e) {
  DPRINTF("%d", *(const int*)e);
} /* adj_matrix_printeu() */

static void adj_matrix_printew(const void* const e) {
  DPRINTF("%f", *(const double*)e);
} /* adj_matrix_printew() */

/*******************************************************************************
 * API Functions
 ******************************************************************************/
struct adj_matrix*
adj_matrix_init(struct adj_matrix* const matrix_in,
                const struct adj_matrix_params* const params) {
  RCSW_FPC_NV(NULL, NULL != params);
  RCSW_ER_MODULE_INIT();
  struct adj_matrix* matrix = NULL;

  if (params->flags & RCSW_NOALLOC_HANDLE) {
    RCSW_CHECK_PTR(matrix_in);
    matrix = matrix_in;
  } else {
    matrix = malloc(sizeof(struct adj_matrix));
    RCSW_CHECK_PTR(matrix);
  }
  matrix->flags = params->flags;
  if (params->is_weighted) {
    matrix->elt_size = sizeof(double);
  } else {
    matrix->elt_size = sizeof(int);
  }
  matrix->is_weighted = params->is_weighted;
  matrix->is_directed = params->is_directed;
  matrix->n_vertices = params->n_vertices;
  matrix->n_edges = 0;

  struct matrix_params mat_params = {
    .n_rows = params->n_vertices,
    .n_cols = params->n_vertices,
    .elements = params->elements,
    .elt_size = matrix->elt_size,
    .flags = params->flags | RCSW_NOALLOC_HANDLE,
  };
  if (matrix->is_weighted) {
    mat_params.printe = adj_matrix_printew;
  } else {
    mat_params.printe = adj_matrix_printeu;
  }
  RCSW_CHECK(NULL != matrix_init(&matrix->matrix, &mat_params));

  /*
   * Static matrix initializes memory to 0, we need it to be NAN for weighted
   * graphs for correct edge detection.
   */
  if (matrix->is_weighted) {
    for (size_t i = 0; i < matrix->n_vertices; ++i) {
      for (size_t j = 0; j < matrix->n_vertices; ++j) {
        *(double*)matrix_access(&matrix->matrix, i, j) = NAN;
      } /* for(j..) */
    } /* for(i..) */
  }

  return matrix;

error:
  adj_matrix_destroy(matrix);
  return NULL;
} /* adj_matrix_init() */

void adj_matrix_destroy(struct adj_matrix* const matrix) {
  RCSW_FPC_V(NULL != matrix);
  matrix_destroy(&matrix->matrix);
  if (!(matrix->flags & RCSW_NOALLOC_HANDLE)) {
    free(matrix);
  }
} /* adj_matrix_destroy() */

status_t adj_matrix_edge_addu(struct adj_matrix* const matrix,
                                     size_t u,
                                     size_t v) {
  RCSW_FPC_NV(ERROR,
              NULL != matrix,
              !matrix->is_directed,
              u < matrix->n_vertices,
              v < matrix->n_vertices);

  int val = 1;
  ER_TRACE("Add undirected edge: (%zu, %zu), (%zu, %zu)", u, v, v, u);
  RCSW_CHECK(OK == matrix_set(&matrix->matrix, u, v, &val));
  ++matrix->n_edges;

  RCSW_CHECK(OK == matrix_set(&matrix->matrix, v, u, &val));
  ++matrix->n_edges;

  return OK;

error:
  return ERROR;
} /* adj_matrix_edge_addu() */

status_t adj_matrix_edge_addd(struct adj_matrix* const matrix,
                              size_t u,
                              size_t v,
                              const double* const w) {
  RCSW_FPC_NV(ERROR,
              NULL != matrix,
              matrix->is_directed,
              u < matrix->n_vertices,
              v < matrix->n_vertices);

  ER_TRACE("Add directed edge: (%zu, %zu) = %f", u, v, w ? *w : 1.0);
  if (matrix->is_weighted) {
    RCSW_CHECK(OK == matrix_set(&matrix->matrix, u, v, w));
  } else {
    int val = 1;
    RCSW_CHECK(OK == matrix_set(&matrix->matrix, u, v, &val));
  }
  ++matrix->n_edges;
  return OK;

error:
  return ERROR;
} /* adj_matrix_edge_addd() */

status_t adj_matrix_edge_remove(struct adj_matrix* const matrix,
                                       size_t u,
                                       size_t v) {
  RCSW_FPC_NV(ERROR,
              NULL != matrix,
              u < matrix->n_vertices,
              v < matrix->n_vertices);
  ER_TRACE("Remove edge: (%zu, %zu)", u, v);

  if (matrix->is_weighted) {
    *(double*)matrix_access(&matrix->matrix, u, v) = NAN;
  } else {
    RCSW_CHECK(OK == matrix_elt_clear(&matrix->matrix, u, v));
  }

  --matrix->n_edges;

  /* If the graph is undirected, also remove edge from v to u. */
  if (!matrix->is_directed) {
    ER_TRACE("Remove edge: (%zu, %zu)", v, u);
    if (matrix->is_weighted) {
      *(double*)matrix_access(&matrix->matrix, v, u) = NAN;
    } else {
      RCSW_CHECK(OK == matrix_elt_clear(&matrix->matrix, v, u));
    }
    --matrix->n_edges;
  }
  return OK;

error:
  return ERROR;
} /* adj_matrix_edge_remove() */

END_C_DECLS
