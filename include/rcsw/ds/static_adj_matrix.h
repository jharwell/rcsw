/**
 * \file static_adj_matrix.h
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <math.h>
#include "rcsw/ds/ds.h"
#include "rcsw/ds/static_matrix.h"
#include "rcsw/common/fpc.h"

/*******************************************************************************
 * Constant Definitions
 ******************************************************************************/

/*******************************************************************************
 * Structure Definitions
 ******************************************************************************/
/**
 * \ingroup ds
 * \brief Implementation of adjacency matrix representation of a "static" graph.
 *
 * Static in the sense that the # of vertices in the graph cannot be changed
 * after initialization. Graphs can be both weighted/unweighted,
 * directed/undirected, per configuration. However, you can't have an undirected
 * graph that is weighted.
 *
 * Sentinel values in the matrix used for detecting if an edge exists are 0 for
 * undirected graphs, and NAN for directed graphs, so don't use those values for
 * valid edges (though why would you?).
 *
 * Pros: Removing edges takes O(1). Queries like "is there an edge from vertex u
 * to vertex v" are efficient and can be done in O(1).
 *
 * Cons: Consumes O(V^2) space, regardless of # edges (i.e. don't use this for
 * sparse graphs). Also you cannot use this data structure if the max # of edges
 * in the graph is not known a priori.
 */
struct static_adj_matrix {
  bool_t is_directed;            /// Is the graph directed?
  bool_t is_weighted;            /// Is the graph weighted?
  size_t n_edges;                /// # edges currently in the graph?
  /**
   * Size of elements in bytes (only used to make edge
   * queries a bit faster.)
  */
  size_t elt_size;
  size_t n_vertices;             /// # vertices in the graph (this is static!).
  struct static_matrix  matrix;  /// Underlying matrix implementation handle.
  uint32_t flags;                /// Configuration flags.
};

/*******************************************************************************
 * Inline Functions
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Access an element of the adjacency matrix.
 *
 * \param matrix The matrix handle.
 * \param u Edge source.
 * \param v Edge sink.
 *
 * \return The value of the edge between the two vertices (may dereference as 0
 * if no edge exists--used \ref static_adj_matrix_edge_query() to be sure).
 */
static inline void* static_adj_matrix_access(const struct static_adj_matrix* const matrix,
                                       size_t u, size_t v) {
  RCSW_FPC_NV(NULL, NULL != matrix, u < matrix->n_vertices,
            v < matrix->n_vertices);
  return static_matrix_access(&matrix->matrix, u, v);
}

/**
 * \brief Get the # of bytes needed for an adjacency matrix.
 *
 * \param n_vertices # edges in the graph to be represented.
 * \param is_weighted Is the graph weighted or not? Weighted graphs use doubles
 * as the edge weights, and unweighted graphs use ints.
 *
 * \return
 */
static inline size_t static_adj_matrix_space(size_t n_vertices, bool_t is_weighted) {
  return static_matrix_space(n_vertices, n_vertices,
                             is_weighted?sizeof(double):sizeof(int));
}

/**
 * \brief Determine if (u,v) exists.
 *
 * \param matrix The matrix handle.
 * \param u Vertex #1.
 * \param v Vertex #2.
 *
 * \return \ref bool_t.
 */
static inline bool_t static_adj_matrix_edge_query(struct static_adj_matrix* const matrix,
                                            size_t u, size_t v) {
  RCSW_FPC_NV(FALSE, NULL != matrix, u < matrix->n_vertices,
            v < matrix->n_vertices);
  if (matrix->is_weighted) {
    return (bool_t)(!isnan(*(double*)static_adj_matrix_access(matrix, u, v)));
  } else {
    return (bool_t)!ds_elt_zchk(static_adj_matrix_access(matrix, u, v),
                                matrix->elt_size);
  }
}

/**
 * \brief Get the # of edges currently defined in the graph.
 *
 * \param matrix The matrix handle.
 *
 * \return The # of edges, or 0 on ERROR.
 */
static inline size_t adj_matrix_n_edges(
    const struct static_adj_matrix *const matrix) {
  RCSW_FPC_NV(0, NULL != matrix);
  return matrix->n_edges;
}

/**
 * \brief Print an adjacency matrix.
 *
 * \param matrix The matrix handle.
 */
static inline void static_adj_matrix_print(const struct static_adj_matrix* const matrix) {
  static_matrix_print(&matrix->matrix);
}

/**
 * \brief Determine if the matrix/graph is empty, defined as having 0 defined
 * edges.
 *
 * \param matrix The matrix handle.
 *
 * \return \ref bool_t.
 */
static inline bool_t static_adj_matrix_isempty(const struct static_adj_matrix* matrix) {
  RCSW_FPC_NV(FALSE, NULL != matrix);
  return (bool_t)(0 == matrix->n_edges);
}

/**
 * \brief Transpose an adjacency matrix. Use for moving from row -> col major
 * and vice versa.
 *
 * \param matrix The matrix handle.
 *
 * \return \ref status_t.
 */
static inline status_t static_adj_matrix_transpose(struct static_adj_matrix* const matrix) {
  return static_matrix_transpose(&matrix->matrix);
}

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/

/**
 * \brief Initialize an adjacency matrix.
 *
 * \param matrix_in The matrix handle to be filled (can be NULL if
 * \ref RCSW_DS_NOALLOC_HANDLE not passed).
 * \param params Initialization parameters.
 *
 * \return The initialized adjacency matrix, or NULL if an error occurred.
 */
struct static_adj_matrix* static_adj_matrix_init(struct static_adj_matrix* matrix_in,
                                     const struct ds_params* params) RCSW_CHECK_RET;

/**
 * \brief Destroy an adjacency matrix. Any further use of the provided handle is
 * undefined.
 *
 * \param matrix The matrix handle.
 */
void static_adj_matrix_destroy(struct static_adj_matrix* matrix);

/**
 * \brief Add a directed edge to the graph (which must have been initialized as
 * a directed graph obviously).
 *
 * \param matrix The matrix handle.
 * \param u Source vertex.
 * \param v Sink vertex.
 * \param w Weight of edge. Ignored for unweighted graphs (pass NULL probably).
 *
 * \return \ref status_t.
 */
status_t static_adj_matrix_edge_addd(struct static_adj_matrix* matrix,
                               size_t u, size_t v, const double *w);

/**
 * \brief Add an undirected edge to the graph (which must have been initialized
 * as an undirected graph obviously).
 *
 * Adding an edge  (u, v) will also automatically add an edge (v, u) with the
 * same value (1.0). This is OK because the having a graph that is both
 * undirected and weighted is disallowed, and was checked during
 * initialization.
 *
 * \param matrix The matrix handle.
 * \param u Vertex #1.
 * \param v Vertex #2.
 *
 * \return \ref status_t.
 */
status_t static_adj_matrix_edge_addu(struct static_adj_matrix* matrix, size_t u,
                               size_t v);

/**
 * \brief Remove an edge (u,v). If the graph was undirected, also remove the
 * edge (v,u).
 *
 * \param matrix The matrix handle.
 * \param u Source vertex.
 * \param v Sink vertex.
 *
 * \return \ref status_t.
 */
status_t static_adj_matrix_edge_remove(struct static_adj_matrix* matrix,
                                 size_t u, size_t v);

END_C_DECLS

