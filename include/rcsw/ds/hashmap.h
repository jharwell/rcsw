/**
 * \file hashmap.h
 * \ingroup ds
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/ds.h"
#include "rcsw/ds/darray.h"

/*******************************************************************************
 * Constant Definitions
 ******************************************************************************/
/** Size of keys for hashmap */
#define RCSW_HASHMAP_KEYSIZE 64

/*******************************************************************************
 * Structure Definitions
 ******************************************************************************/
/**
 * \brief Hashmap initialization parameters.
 */
struct hashmap_params {
  /**
   * Pointer to application-allocated space for storing the \ref hashmap
   * buckets. Ignored unless \ref RCSW_NOALLOC_META is passed.
   */
  dptr_t *meta;

  /**
   * Pointer to application-allocated space for storing data managed by the \ref
   * hashmap. Ignored unless \ref RCSW_NOALLOC_DATA is passed.
   */
  dptr_t *elements;

  /**
   * Size of elements in bytes.
   */
  size_t elt_size;

  /**
   * Configuration flags. See \ref hashmap.flags for valid flags.
   */
  uint32_t flags;

  /** Hashing function to use. Must be non-NULL. */
  uint32_t (*hash)(const void *const key, size_t len);

  /**
   * Initial size of hash buckets; passed to \ref darray_init() as \ref
   * darray_params.init_size; this is also used as \ref darray_params.max_elts ;
   * that is, fixed size hash buckets.
   */
  size_t bsize;

  /**
   * Number of buckets for hashmap.
   */
  size_t n_buckets;

  /**
   * Number of inserts before automatically sorting. -1 = do not automatically
   * sort.
   */
  int sort_thresh;
};

/**
 * \brief Hashmap statistics.
 *
 * Provides detailed info about how well the hashmap is working.
 */
struct hashmap_stats {
  size_t n_buckets;        /// Number of buckets in hashmap
  size_t n_nodes;          /// Number of hashnodes in hashmap. Updated on add/remove.
  size_t n_adds;           /// Number of adds to hashmap since last reset
  size_t n_addfails;       /// Number of failures to add to hashmap since last reset
  size_t n_collisions;     /// Number of collisions when adding since last reset
  double collision_ratio;  /// Ratio of colliding/non-colliding adds
  bool_t sorted;           /// Is the hashmap sorted?
  double max_util;         /// Max bucket utilization
  double min_util;         /// Min bucket utilization
  double average_util;     /// Average bucket utilization
};

/**
 * \brief Simple convenience struct for the space for hashmap data.
 *
 * Element space is chunked into 3 parts (in order):
 *
 * - An allocation map of data blocks (array of max_elts ints)
 * - Datablock array (max_elts * elt_size)
 * - Metadata array of \ref hashnode
 *
 * Meta space from \ref hashmap_params.meta is used for the \ref darray handles
 * (buckets).
 */
struct hashmap_space_mgmt {
  /**
   * Raw pointer to space for elements.
   */
  dptr_t*               elements;
  struct allocm_entry*  db_map;
  dptr_t*               datablocks;
  struct hashnode*      hashnodes;

  /**
   * Dynamic arrays that will be set to a fixed size during
   * initialization. Each bucket holds hashnodes.
   */
  struct darray           *buckets;

  /**
   * Raw space for the \ref darray objects used to manage the data.
   */
  uint8_t                 *nodes;
};

/**
 * \brief The hashmap data structure (an associative array).
 *
 * The elements in this data structure are \ref hashnode objects, and the nodes
 * are the dynamic arrays (\ref darray) used to implement the buckets.
 */
struct hashmap {
  /**
   * Hashing function. Can't be NULL (duh).
   */
  uint32_t (*hash)(const void *const key, size_t len);

  /**
   * Last bucket that was added to/removed from.
   */
  struct darray *last_used;

  /**
   * Management of all node and element space for the hashmap.
   */
  struct hashmap_space_mgmt space;

  /**
   * Size of data elements for hashmap.
   */
  size_t elt_size;

  /**
   * Max number of of elements allowed in the hashmap.
   */
  size_t max_elts;

  /**
   * Number of of buckets in the hashmap. Does not change after initialization.
   */
  size_t n_buckets;

  /**
   * Current hashmap statistics
   */
  struct hashmap_stats stats;


  /**
   * Number of of successful adds to wait before automatically sorting
   * hashmap. -1 = do not automatically sort hashmap (sorting must be done
   * manually) via \ref hashmap_sort().
   */
  int sort_thresh;

  /**
   * Is the hashmap currently sorted?
   */
  bool_t sorted;

  /**
   * Run time configuration flags. Valid flags are:
   *
   * - \ref RCSW_ZALLOC
   * - \ref RCSW_NOALLOC_HANDLE.
   * - \ref RCSW_NOALLOC_DATA.
   * - \ref RCSW_NOALLOC_META.
   * - \ref RCSW_DS_HASHMAP_LINPROB
   * - \ref RCSW_DS_SORTED
   *
   * All other flags are ignored.
   */
  uint32_t flags;
};

/**
 * Nodes within the hashmap (each bucket is filled with these).
 *
 * As you woulde xpect for an associative array element, this is a key-value
 * pair
 *
 * Must be packed and aligned to the same size as \ref ptr_t so that casts from
 * \ref hashnode.data are safe on all targets.
 */
struct RCSW_ATTR(packed, aligned (sizeof(dptr_t))) hashnode {
  /**
   * Key for key-value pair.
   */
  uint8_t key[RCSW_HASHMAP_KEYSIZE];

  /**
   * Value for key-value pair.
   */
  dptr_t *data;

  /**
   * Calculated hash. Not used at runtime, but useful when printing elements for
   * debugging.
   */
  uint32_t hash;
};

/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS
/**
 * \brief Get a bucket index from a reference to a bucket.
 *
 * Not really necessary to be a function, but helps with readability.
 */
static inline size_t hashmap_bucket_index(const struct hashmap* const map,
                                          const struct darray* const bucket) {
  return (bucket - map->space.buckets) % sizeof(struct darray);
}

/**
 * \brief Calculate the # of bytes that the hashmap will require if \ref
 * RCSW_NOALLOC_DATA is passed to manage a specified # of elements of a
 * specified size.
 *
 * \param n_buckets The # buckets in the hashmap.
 *
 * \param bucket_size The size of each bucket in the hashmap.
 *
 * \param elt_size Size of elements in bytes.
 *
 * We compute the # max elements so that you don't have to worry about the #
 * buckets not being an even multiple of the max # elements, and allocating too
 * little space because of rounding errors.
 *
 * \return The total # of bytes the application would need to allocate.
 */
static inline size_t hashmap_element_space(size_t n_buckets,
                                           size_t bucket_size,
                                           size_t elt_size) {
  size_t max_elts = n_buckets * bucket_size;
  return ds_meta_space(max_elts) +
      darray_element_space(max_elts, sizeof(struct hashnode)) +
      ds_elt_space_simple(max_elts, elt_size);
}

/**
 * \brief Calculate the space needed for the nodes in the hashmap
 *
 * Used in conjunction with \ref RCSW_NOALLOC_META.
 *
 * \param n_buckets # of desired buckets in the hashmap
 *
 * \return The # of bytes required
 */
static inline size_t hashmap_meta_space(size_t n_buckets) {
  return sizeof(struct darray) * n_buckets;
}


/**
 * \brief Initialize a hashmap.
 *
 * \param map_in The handle to be filled. Must be non-NULL if \ref
 *                RCSW_NOALLOC_HANDLE passed in \ref hashmap_params.flags.
 *
 * \param params Initialization parameters
 *
 * \return  The initialized hashmap, or NULL if an error occurred
 */
struct hashmap *hashmap_init(struct hashmap *map_in,
                             const struct hashmap_params * params) RCSW_CHECK_RET;

/**
 * \brief destroy a hashmap. Any further use of the hashmap after calling this
 * function is undefined.
 */
void hashmap_destroy(struct hashmap *map);


/**
 * \brief Sort a hashmap
 *
 * Sorts each bucket in. This function only needs to be called if \ref
 * RCSW_DS_SORTED was not passed during initialization AND sort_threshold was
 * set to -1.
 *
 * \param map The hashmap handle.
 *
 * \return \ref status_t
 */
status_t hashmap_sort(struct hashmap * map);

/**
 * \brief Clear a hashmap, but don't deallocate its data
 *
 * \param map The hashmap handle.
 *
 * \return \ref status_t
 */
status_t hashmap_clear(const struct hashmap * map);

/**
 * \brief Returns the data from the hashmap corresponding to the given key.
 *
 * \param map The hashmap handle.
 * \param key Key to match with
 *
 * \return: The data, or NULL if an error occurred or the data was not found.
 */
void *hashmap_data_get(struct hashmap * map, const void * key);

/**
 * \brief Add a node to the hashmap
 *
 * This function takes the key and data for the new node, calculates the hash
 * for it, and then adds it to the correct bucket. If the bucket is currently
 * full, it returns failure, unless \ref RCSW_DS_HASHMAP_LINPROB was passed
 * during initialization. If the node already exists in the bucket, failure is
 * returned (no duplicates are allowed).
 *
 * \param map The hashmap handle.
 * \param key The key to add.
 * \param data The data to add.
 *
 * \return \ref status_t
 */
status_t hashmap_add(struct hashmap * map, const void * key, const void * data);

/**
 * \brief Remove a node from a hashmap
 *
 * This function searches the hashmap for the specified key. If it is found, it
 * is removed from the appropriate bucket. If it is not found, no operation is
 * performed, and success is returned.
 *
 * \param map The hashmap handle.
 * \param key The key for the data to remove.
 *
 * \return \ref status_t
 */
status_t hashmap_remove(struct hashmap * map, const void * key);

/**
 * \brief Show stats about a hashmap.
 *
 * This function shows some basic stastics about the hashmap: total # of
 * collisions, utilization, etc.
 *
 * \param map The hashmap handle.
 */
void hashmap_print(const struct hashmap * map);

/**
 * \brief Gather statistics about current state of hashmap.
 *
 * \param map The hashmap handle.
 * \param stats The statistics to be filled.
 *
 * \return \ref status_t
 */
status_t hashmap_gather(const struct hashmap * map,
                        struct hashmap_stats * stats);

/**
 * \brief Print the hashmap distribution.
 *
 * The function provides a visual representation of how many elements are in
 * the hashmap and what their distribution is across the buckets.
 *
 * \param map The hashmap handle.
 */
void hashmap_print_dist(const struct hashmap * map);

/**
 * \brief Get the bucket a key can be found in.
 *
 * \param map The hashmap handle.
 * \param key The key to identify.
 * \param hash_out The hash of the element, if non-NULL.
 *
 * \return The bucket, or NULL if an ERROR occurred.
 *
 */
struct darray *hashmap_query(const struct hashmap * map,
                             const void * key,
                             uint32_t * hash_out);

END_C_DECLS
