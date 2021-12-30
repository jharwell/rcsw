/**
 * @file static_adj_matrix.c
 *
 * @copyright 2017 John Harwell, All rights reserved.
 *
 * This file is part of RCSW.
 *
 * RCSW is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * RCSW is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * RCSW.  If not, see <http://www.gnu.org/licenses/
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/static_adj_matrix.h"
#include "rcsw/common/dbg.h"

/*******************************************************************************
 * Constant Definitions
 ******************************************************************************/
#define MODULE_ID M_DS_STATIC_ADJ_MATRIX

/*******************************************************************************
 * Structure Definitions
 ******************************************************************************/

/*******************************************************************************
 * Global Variables
 ******************************************************************************/

/*******************************************************************************
 * Macros
 ******************************************************************************/

/*******************************************************************************
 * Forward Declarations
 ******************************************************************************/
BEGIN_C_DECLS

static void static_adj_matrix_printeu(const void* e);
static void static_adj_matrix_printew(const void* e);

/*******************************************************************************
 * API Functions
 ******************************************************************************/
struct static_adj_matrix* static_adj_matrix_init(
    struct static_adj_matrix* const matrix_in,
    const struct ds_params* const params) {
  RCSW_FPC_NV(NULL, NULL != params, params->tag == DS_ADJ_MATRIX);
  struct static_adj_matrix* matrix = NULL;

  if (params->flags & DS_APP_DOMAIN_HANDLE) {
    RCSW_CHECK_PTR(matrix_in);
    matrix = matrix_in;
  } else {
    matrix = malloc(sizeof(struct static_adj_matrix));
    RCSW_CHECK_PTR(matrix);
  }
  matrix->flags = params->flags;
  if (params->type.adjm.is_weighted) {
    matrix->el_size = sizeof(double);
  } else {
    matrix->el_size = sizeof(int);
  }
  matrix->is_weighted = params->type.adjm.is_weighted;
  matrix->is_directed = params->type.adjm.is_directed;
  matrix->n_vertices = params->type.adjm.n_vertices;
  matrix->n_edges = 0;

  struct ds_params mat_params = {
      .type = {.smat = {.n_rows = params->type.adjm.n_vertices,
                        .n_cols = params->type.adjm.n_vertices}},
      .elements = params->elements,
      .el_size = matrix->el_size,
      .flags = params->flags | DS_APP_DOMAIN_HANDLE,
      .tag = DS_STATIC_MATRIX};
  if (matrix->is_weighted) {
    mat_params.printe = static_adj_matrix_printew;
  } else {
    mat_params.printe = static_adj_matrix_printeu;
  }
  RCSW_CHECK(NULL != static_matrix_init(&matrix->matrix, &mat_params));

  /*
   * Static matrix initializes memory to 0, we need it to be NAN for weighted
   * graphs for correct edge detection.
   */
  if (matrix->is_weighted) {
    for (size_t i = 0; i < matrix->n_vertices; ++i) {
      for (size_t j = 0; j < matrix->n_vertices; ++j) {
        *(double*)static_matrix_access(&matrix->matrix, i, j) = NAN;
      } /* for(j..) */
    }   /* for(i..) */
  }

  return matrix;

error:
  static_adj_matrix_destroy(matrix);
  return NULL;
} /* static_adj_matrix_init() */

void static_adj_matrix_destroy(struct static_adj_matrix* const matrix) {
  RCSW_FPC_V(NULL != matrix);
  static_matrix_destroy(&matrix->matrix);
  if (!(matrix->flags & DS_APP_DOMAIN_HANDLE)) {
    free(matrix);
  }
} /* static_adj_matrix_destroy() */

status_t static_adj_matrix_edge_addu(struct static_adj_matrix* const matrix,
                                     size_t u,
                                     size_t v) {
  RCSW_FPC_NV(ERROR,
            NULL != matrix,
            !matrix->is_directed,
            u < matrix->n_vertices,
            v < matrix->n_vertices);

  int val = 1;
  DBGV("Add undirected edges: (%zu, %zu), (%zu, %zu)\n", u, v, v, u);
  RCSW_CHECK(OK == static_matrix_set(&matrix->matrix, u, v, &val));
  ++matrix->n_edges;

  RCSW_CHECK(OK == static_matrix_set(&matrix->matrix, v, u, &val));
  ++matrix->n_edges;

  return OK;

error:
  return ERROR;
} /* static_adj_matrix_edge_addu() */

status_t static_adj_matrix_edge_addd(struct static_adj_matrix* const matrix,
                                     size_t u,
                                     size_t v,
                                     const double* const w) {
  RCSW_FPC_NV(ERROR,
            NULL != matrix,
            matrix->is_directed,
            u < matrix->n_vertices,
            v < matrix->n_vertices);

  DBGV("Add directed edge: (%zu, %zu) = %f\n", u, v, w ? *w : 1.0);
  if (matrix->is_weighted) {
    RCSW_CHECK(OK == static_matrix_set(&matrix->matrix, u, v, w));
  } else {
    int val = 1;
    RCSW_CHECK(OK == static_matrix_set(&matrix->matrix, u, v, &val));
  }
  ++matrix->n_edges;
  return OK;

error:
  return ERROR;
} /* static_adj_matrix_edge_addd() */

status_t static_adj_matrix_edge_remove(struct static_adj_matrix* const matrix,
                                       size_t u,
                                       size_t v) {
  RCSW_FPC_NV(
      ERROR, NULL != matrix, u < matrix->n_vertices, v < matrix->n_vertices);
  DBGV("Remove edge: (%zu, %zu)\n", u, v);
  if (matrix->is_weighted) {
    *(double*)static_matrix_access(&matrix->matrix, u, v) = NAN;
  } else {
    RCSW_CHECK(OK == static_matrix_clear(&matrix->matrix, u, v));
  }

  --matrix->n_edges;

  /* If the graph is undirected, also remove edge from v to u. */
  if (!matrix->is_directed) {
    DBGV("Remove edge: (%zu, %zu)\n", v, u);
    if (matrix->is_weighted) {
      *(double*)static_matrix_access(&matrix->matrix, v, u) = NAN;
    } else {
      RCSW_CHECK(OK == static_matrix_clear(&matrix->matrix, v, u));
    }
    --matrix->n_edges;
  }
  return OK;

error:
  return ERROR;
} /* static_adj_matrix_edge_remove() */

/*******************************************************************************
 * Static Functions
 ******************************************************************************/
static void static_adj_matrix_printeu(const void* const e) {
  printf("%d", *(const int*)e);
} /* static_adj_matrix_printeu() */

static void static_adj_matrix_printew(const void* const e) {
  printf("%f", *(const double*)e);
} /* static_adj_matrix_printew() */

END_C_DECLS
