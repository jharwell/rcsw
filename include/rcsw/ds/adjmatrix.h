/**
 * \file
 *
 * \copyright 2017 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * \ingroup ds
 *
 * \brief Adjacency matrix representation of a graph.
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <math.h>
#include <string.h>

#include "rcsw/al/types.h"
#include "rcsw/core/compilers.h"
#include "rcsw/core/core.h"
#include "rcsw/core/fpc.h"
#include "rcsw/ds/matrix.h"
#include "rcsw/utils/numeric.h"

/*******************************************************************************
 * Types
 ******************************************************************************/
/**
 * \brief Adjacency matrix initialization parameters.
 */
struct adjmatrix_config {
  /**
   * Pointer to application-allocated space for storing the \ref adjmatrix
   * data. Ignored unless \ref RCSW_NOALLOC_DATA is passed.
   */
  dptr_t* elements;

  /**
   * Configuration flags. See \ref adjmatrix.flags for valid flags.
   */
  uint32_t flags;

  /**
   * Initial # of vertices for graph, for space allocation.
   */
  size_t n_vertices;

  /**
   * Is the graph directed or undirected?
   */
  bool_t is_directed;

  /**
   * Are the graph edges weighted? Must be false if \ref adjmatrix_config.is_directed is false:
   * weighted undirected graphs are not supported (see \ref adjmatrix).
   */
  bool_t is_weighted;
};

/**
 * \ingroup ds
 * \brief Implementation of adjacency matrix representation of a "static" graph.
 *
 * Static in the sense that the # of vertices in the graph cannot be changed
 * after initialization. Graphs can be weighted or unweighted, and directed or
 * undirected, with one exception:
 *
 * \note Weighted graphs must be directed. \ref adjmatrix_init() rejects
 * <tt>is_weighted && !is_directed</tt> (function precondition: NULL under \c
 * LIBRA_FPC_RETURN), because \ref adjmatrix_edge_addu() takes no weight. To
 * model a weighted undirected graph, make it directed and add each edge in
 * both directions with \ref adjmatrix_edge_addd(), and remove both directions
 * when removing it.
 *
 * \note Edge weights are always \c double and unweighted edges are \c int;
 * the element type is not configurable.
 *
 * Sentinel values in the matrix used for detecting if an edge exists are NAN
 * for weighted graphs and 0 for unweighted graphs, so don't use those values
 * for valid edges.
 *
 * Pros: Removing edges takes O(1). Queries like "is there an edge from vertex u
 * to vertex v" are efficient and can be done in O(1).
 *
 * Cons: Consumes O(V^2) space, regardless of # edges (i.e. don't use this for
 * sparse graphs). Also you cannot use this data structure if the max # of edges
 * in the graph is not known a priori.
 */
struct adjmatrix {
  /** Is the graph directed? */
  bool_t is_directed;

  /** Is the graph weighted? */
  bool_t is_weighted;

  /** Number of edges currently in the graph */
  size_t n_edges;

  /**
   * Underlying matrix implementation handle. Its \ref matrix_config.n_rows is
   * the number of vertices, and its \ref matrix_config.elt_size the size of an
   * edge (\c double if weighted, \c int if not).
   */
  struct matrix matrix;

  /**
   * Configuration flags. Valid flags are:
   *
   * - \ref RCSW_NOALLOC_HANDLE
   * - \ref RCSW_NOALLOC_DATA
   * - \ref RCSW_ZALLOC
   *
   * All other flags are ignored.
   */
  uint32_t flags;
};
/** \brief The number of vertices in the graph. */
static inline size_t adjmatrix_n_vertices(const struct adjmatrix* matrix) {
  return matrix->matrix.cfg.n_rows;
}

/*******************************************************************************
 * Public API
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
 * if no edge exists--used \ref adjmatrix_edge_query() to be sure).
 */
static inline void* adjmatrix_access(const struct adjmatrix* const matrix,
                                     size_t                        u,
                                     size_t                        v) {
  RCSW_FPC_NV(NULL,
              NULL != matrix,
              u < adjmatrix_n_vertices(matrix),
              v < adjmatrix_n_vertices(matrix));
  return matrix_access(&matrix->matrix, u, v);
}

/**
 * \brief Get the # of bytes needed for an adjacency matrix.
 *
 * \param n_vertices # vertices in the graph to be represented.
 * \param is_weighted Is the graph weighted or not? Weighted graphs use doubles
 * as the edge weights, and unweighted graphs use ints.
 *
 * \return # total bytes required.
 */
static inline size_t adjmatrix_element_space(size_t n_vertices,
                                             bool_t is_weighted) {
  return matrix_element_space(n_vertices,
                              n_vertices,
                              is_weighted ? sizeof(double) : sizeof(int));
}

/**
 * \brief Determine if (u,v) exists.
 *
 * \param matrix The matrix handle.
 * \param u Vertex #1.
 * \param v Vertex #2.
 */
static inline bool_t adjmatrix_edge_query(const struct adjmatrix* const matrix,
                                          size_t                        u,
                                          size_t                        v) {
  RCSW_FPC_NV(false,
              NULL != matrix,
              u < adjmatrix_n_vertices(matrix),
              v < adjmatrix_n_vertices(matrix));
  if (matrix->is_weighted) {
    double w;
    memcpy(&w, adjmatrix_access(matrix, u, v), sizeof(w)); /* may be unaligned */
    return !isnan(w);
  }
  return !utils_zchk(adjmatrix_access(matrix, u, v), matrix->matrix.cfg.elt_size);
}

/**
 * \brief Get the # of edges currently defined in the graph.
 *
 * \param matrix The matrix handle.
 *
 * \return The # of edges, or 0 on ERROR.
 */
static inline size_t adjmatrix_n_edges(const struct adjmatrix* const matrix) {
  RCSW_FPC_NV(0, NULL != matrix);
  return matrix->n_edges;
}

/**
 * \brief Print an adjacency matrix.
 *
 * \param matrix The matrix handle.
 */
static inline void adjmatrix_print(const struct adjmatrix* const matrix) {
  RCSW_FPC_V(NULL != matrix);
  matrix_print(&matrix->matrix);
}

/**
 * \brief Determine if the matrix/graph is empty, defined as having 0 defined
 * edges.
 *
 * \param matrix The matrix handle.
 */
static inline bool_t adjmatrix_isempty(const struct adjmatrix* matrix) {
  RCSW_FPC_NV(false, NULL != matrix);
  return (0 == matrix->n_edges);
}

/**
 * \brief Transpose an adjacency matrix. Use for moving from row -> col major
 * and vice versa.
 *
 * \param matrix The matrix handle.
 *
 * \return \ref status_t.
 */
static inline status_t adjmatrix_transpose(struct adjmatrix* const matrix) {
  return matrix_transpose(&matrix->matrix);
}

/**
 * \brief Initialize an adjacency matrix.
 *
 * \param matrix_in Caller storage for the handle, used only if \ref
 *                  RCSW_NOALLOC_HANDLE is passed; ignored (may be NULL)
 *                  otherwise. See \rcswdoc{concepts/memory-model}.
 *
 * \param params Initialization parameters.
 *
 * \return The initialized adjacency matrix, or NULL if an error occurred.
 */
RCSW_API struct adjmatrix* adjmatrix_init(
  struct adjmatrix* matrix_in, const struct adjmatrix_config* params) RCSW_WUR;

/**
 * \brief Destroy an adjacency matrix. Any further use of the provided handle is
 * undefined.
 *
 * \param matrix The matrix handle.
 */
RCSW_API void adjmatrix_destroy(struct adjmatrix* matrix);

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
RCSW_API status_t adjmatrix_edge_addd(struct adjmatrix* matrix,
                                      size_t            u,
                                      size_t            v,
                                      const double*     w);

/**
 * \brief Add an undirected edge to the graph.
 *
 * Adding an edge (u, v) also adds the edge (v, u). Undirected graphs are
 * always unweighted (see \ref adjmatrix), so no weight is taken.
 *
 * \param matrix The matrix handle.
 * \param u Vertex #1.
 * \param v Vertex #2.
 *
 * \return \ref status_t.
 */
RCSW_API status_t adjmatrix_edge_addu(struct adjmatrix* matrix,
                                      size_t            u,
                                      size_t            v);

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
RCSW_API status_t adjmatrix_edge_remove(struct adjmatrix* matrix,
                                        size_t            u,
                                        size_t            v);

END_C_DECLS
