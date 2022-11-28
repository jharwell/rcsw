/**
 * \file hashmap.h
 * \ingroup ds
 * \brief Implementation of hashmap using dynamic arrays and linked lists.
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef INCLUDE_RCSW_DS_HASHMAP_H_
#define INCLUDE_RCSW_DS_HASHMAP_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/ds.h"
#include "rcsw/ds/darray.h"

/*******************************************************************************
 * Constant Definitions
 ******************************************************************************/
/** Max size of keys for hashmap */
#define HASHMAP_MAX_KEYSIZE 64

/*******************************************************************************
 * Structure Definitions
 ******************************************************************************/
/**
 * \brief The hashmap data structure.
 */
struct hashmap {
    uint32_t (*hash)(const void *const key, size_t len);  /// Hashing function

    /**
     * Dynamic arrays that will be set to a fixed size during
     * initialization. Each bucket holds hashnodes.
     */
    struct darray *buckets;
    struct darray *last_used;  /// last bucket that was added to/removed from
    uint8_t *elements;    /// Space for the hashmap data
    uint8_t *nodes;       /// Space for the arrays used to manage the data.
    size_t el_size;       /// Size of data elements for hashmapn
    size_t max_elts;      /// Max # of elements allowed in the hashmap
    size_t n_buckets;     /// # of buckets in the hashmap
    size_t n_nodes;       /// Total number of nodes in the hashmap
    size_t n_collisions;  /// # of collisions since last reset
    size_t n_adds;        /// # of successful node additions since last reset
    size_t n_addfails;    /// # of unsuccessful node additions since last reset
    /**
     * # of successful adds to wait before automatically sorting hashmap. -1 =
     * do not automatically sort hashmap (sorting must be done manually).
     */
    int sort_thresh;

    size_t keysize;      /// Size in bytes of hashnode keys
    bool_t sorted;       /// Is the hashmap sorted?
    uint32_t flags;      /// Run time configuration flags
};

/** Hashmap statistics */
struct hashmap_stats {
    size_t n_buckets;        /// # buckets in hashmap
    size_t n_nodes;          /// # hashnodes in hashmap
    size_t n_adds;           /// # adds to hashmap */
    size_t n_addfails;       /// # of failures to add to hashmap
    size_t n_collisions;     /// # of collisions when adding to hashmap
    double collision_ratio;  /// ratio of colliding/non-colliding adds
    bool_t sorted;           /// Is the hashmap sorted?
    double max_util;         /// Max bucket utilization
    double min_util;         /// Min bucket utilization
    double average_util;     /// Average bucket utilization
};

/** Nodes within the hashmap (each bucket is filled with these) */
struct hashnode {
    uint8_t key[HASHMAP_MAX_KEYSIZE];  /// Key for key-value pair
    void *data;                        /// Value for key-value pair
    uint32_t hash;                     /// Calculated hash
};

/*******************************************************************************
 * Inline Functions
 ******************************************************************************/
/* Get a bucket index from a reference to a bucket */
static inline size_t hashmap_bucket_index(const struct hashmap* const map,
                                          const struct darray* const bucket) {
    return (bucket - map->buckets) % sizeof(struct darray);
}

/**
 * \brief Calculate the # of bytes that the hashmap will require if \ref
 * DS_APP_DOMAIN_DATA is passed to manage a specified # of elements of a
 * specified size
 *
 * \param max_elts # of desired elements the hashmap will hold
 * \param el_size size of elements in bytes
 *
 * \return The total # of bytes the application would need to allocate
 */
static inline size_t hashmap_element_space(size_t max_elts, size_t el_size) {
    return ds_calc_meta_space(max_elts) +
        ds_calc_element_space1(sizeof(struct hashnode) + el_size, max_elts);
}

/**
 * \brief Calculate the space needed for the nodes in the hashmap, given a
 * max # of elements
 *
 * Used in conjunction with \ref DS_APP_DOMAIN_NODES
 *
 * \param max_elts # of desired elements the hashmap will hold
 *
 * \return The # of bytes required
 */
static inline size_t hashmap_node_space(size_t max_elts) {
    return sizeof(struct darray) * max_elts;
}

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Initialize a hashmap.
 *
 * \param map_in The hashmap handle to be filled (can be NULL if
 * \ref DS_APP_DOMAIN_HANDLE not passed).
 * \param params Initialization parameters
 *
 * \return  The initialized hashmap, or NULL if an error occurred
 */
struct hashmap *hashmap_init(
    struct hashmap *map_in,
    const struct ds_params * params) RCSW_CHECK_RET;

/**
 * \brief destroy a hashmap. Any further use of the hashmap after calling this
 * function is undefined.
 */
void hashmap_destroy(struct hashmap *map);


/**
 * \brief Sort a hashmap
 *
 * Sorts each bucket in. This function should only be called if \ref
 * DS_KEEP_SORTED was not passed
 * during initialization and sort_threshold was set to -1.
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
 * \return: The data, or NULL if an error occurred or the data was not found
 */
void *hashmap_data_get(struct hashmap * map, const void * key);

/**
 * \brief Add a node to the hashmap
 *
 * This function takes the key and data for the new node, calculates the hash
 * for it, and then adds it to the correct bucket. If the bucket is currently
 * full, it returns failure, unless \ref DS_HASHMAP_LINEAR_PROBING was passed
 * during initialization. If the node already exists in the bucket, failure is
 * returned (no duplicates are allowed).
 *
 * \param map The hashmap handle.
 * \param key The key to add.
 * \param data The data to add.
 *
 * \return \ref status_t
 */
status_t hashmap_add(struct hashmap * map, const void * key,
                     const void * data);

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
status_t hashmap_gather(
    const struct hashmap * map, struct hashmap_stats * stats);

/**
 * \brief Print the hashmap distribution.
 *
 * The function provides a visual representation of how many elements are in
 * the hashmap and what their distribution is across the buckets.
 *
 * \param map The hashmap handle.
 */
void hashmap_print_distribution(const struct hashmap * map);

/**
 * \brief Get the bucket a key can be found in.
 *
 * \param map The hashmap handle.
 * \param key The key to identify.
 * \param hash_out The hash of the element, if non-NULL.
 *
 * \return The bucket, or NULL if an ERROR occurred.
 *
 **/
struct darray *hashmap_query(
    const struct hashmap * map, const void * key,
    uint32_t * hash_out);

END_C_DECLS

#endif /*  INCLUDE_RCSW_DS_HASHMAP_H_  */
