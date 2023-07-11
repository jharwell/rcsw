/**
 * \file adj_matrix.h
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
#include "rcsw/ds/matrix.h"
#include "rcsw/common/fpc.h"

/*******************************************************************************
 * Structure Definitions
 ******************************************************************************/
/**
 * \brief Adjacency matrix initialization parameters.
 */
struct adj_matrix_params {
  RCSW_DECLARE_DS_PARAMS_COMMON;
  /**
   * Initial # of vertices for graph, for space allocation.
   */
  size_t n_vertices;  /// Max # of vertices graph will hold.
  bool_t is_directed;  /// Is the graph directed or undirected?
  /**
   * Are the graph edges weighted? If a graph is undirected it cannot be
   * weighted.
   */
  bool_t is_weighted;
};

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
struct adj_matrix {
  /** Is the graph directed? */
  bool_t        is_directed;

  /** Is the graph weighted? */
  bool_t        is_weighted;

  /** # edges currently in the graph? */

  size_t        n_edges;

  /**
   * Size of elements in bytes (only used to make edge queries a bit faster.)
   */
  size_t        elt_size;

  /** # vertices in the graph */
  size_t        n_vertices;

  /** Underlying matrix implementation handle. */
  struct matrix matrix;

  /** Configuration flags. */
  uint32_t      flags;
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
 * if no edge exists--used \ref adj_matrix_edge_query() to be sure).
 */
static inline void* adj_matrix_access(const struct adj_matrix* const matrix,
                                       size_t u,
                                      size_t v) {
  RCSW_FPC_NV(NULL,
              NULL != matrix,
              u < matrix->n_vertices,
              v < matrix->n_vertices);
  return matrix_access(&matrix->matrix, u, v);
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
static inline size_t adj_matrix_element_space(size_t n_vertices,
                                              bool_t is_weighted) {
  return matrix_element_space(n_vertices, n_vertices,
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
static inline bool_t adj_matrix_edge_query(struct adj_matrix* const matrix,
                                            size_t u,
                                           size_t v) {
  RCSW_FPC_NV(false,
              NULL != matrix,
              u < matrix->n_vertices,
              v < matrix->n_vertices);
  if (matrix->is_weighted) {
    return (bool_t)(!isnan(*(double*)adj_matrix_access(matrix, u, v)));
  } else {
    return (bool_t)!ds_elt_zchk(adj_matrix_access(matrix, u, v),
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
static inline size_t adj_matrix_n_edges(const struct adj_matrix *const matrix) {
  RCSW_FPC_NV(0, NULL != matrix);
  return matrix->n_edges;
}

/**
 * \brief Print an adjacency matrix.
 *
 * \param matrix The matrix handle.
 */
static inline void adj_matrix_print(const struct adj_matrix* const matrix) {
  RCSW_FPC_V(NULL != matrix);
  matrix_print(&matrix->matrix);
}

/**
 * \brief Determine if the matrix/graph is empty, defined as having 0 defined
 * edges.
 *
 * \param matrix The matrix handle.
 *
 * \return \ref bool_t.
 */
static inline bool_t adj_matrix_isempty(const struct adj_matrix* matrix) {
  RCSW_FPC_NV(false, NULL != matrix);
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
static inline status_t adj_matrix_transpose(struct adj_matrix* const matrix) {
  return matrix_transpose(&matrix->matrix);
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
struct adj_matrix* adj_matrix_init(struct adj_matrix* matrix_in,
                                     const struct adj_matrix_params* params) RCSW_CHECK_RET;

/**
 * \brief Destroy an adjacency matrix. Any further use of the provided handle is
 * undefined.
 *
 * \param matrix The matrix handle.
 */
void adj_matrix_destroy(struct adj_matrix* matrix);

/**
 * \brief Add a directed edge to the graph.
 *
 * \param matrix The matrix handle.
 * \param u Source vertex.
 * \param v Sink vertex.
 * \param w Weight of edge. Ignored for unweighted graphs (pass NULL probably).
 *
 * \return \ref status_t.
 */
status_t adj_matrix_edge_addd(struct adj_matrix* matrix,
                              size_t u,
                              size_t v,
                              const double *w);

/**
 * \brief Add an undirected edge to the graph.
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
status_t adj_matrix_edge_addu(struct adj_matrix* matrix, size_t u, size_t v);

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
status_t adj_matrix_edge_remove(struct adj_matrix* matrix,
                                size_t u,
                                size_t v);

END_C_DECLS
