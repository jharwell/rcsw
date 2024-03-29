/**
 * \file hashmap.c
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */
/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/hashmap.h"

#include <math.h>

#define RCSW_ER_MODNAME "rcsw.ds.hm"
#define RCSW_ER_MODID ekLOG4CL_DS_HASHMAP
#include "rcsw/er/client.h"
#include "rcsw/algorithm/sort.h"
#include "rcsw/common/fpc.h"
#include "rcsw/ds/darray.h"
#include "rcsw/utils/hash.h"
#include "rcsw/common/alloc.h"

/*******************************************************************************
 * Private Functions
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Get a bucket index from a reference to a bucket.
 *
 * Not really necessary to be a function, but helps with readability.
 */
static size_t hashmap_bucket_index(const struct hashmap* const map,
                                   const struct darray* const bucket) {
  return (bucket - map->space.buckets) % sizeof(struct darray);
}

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
static struct darray *hashmap_query(const struct hashmap * map,
                             const void * key,
                             uint32_t * hash_out);

/**
 * \brief Allocate a datablock.
 *
 * \param map The hashmap handle.
 *
 * \return The allocated datablock, or NULL if no valid block could be found.
 *
 */
static dptr_t* hashmap_db_alloc(const struct hashmap* const map) {
  /*
   * Try to find an available data block. Using hashing/linear probing instead
   * of linear scan. This reduces startup times if initializing/building a
   * large hashmap.
   */

  /* make sure that we have 32 bits of randomness */
  uint32_t val =
      (uint32_t)(random() & 0xff) | (uint32_t)((random() & 0xff) << 8) |
      (uint32_t)((random() & 0xff) << 16) | (uint32_t)((random() & 0xff) << 24);

  size_t search_idx = hash_fnv1a(&val, 4) % map->max_elts;

  int alloc_idx = allocm_probe(map->space.db_map, map->max_elts, search_idx);
  RCSW_CHECK(-1 != alloc_idx);
  dptr_t* datablock = (void*)((uint8_t*)map->space.datablocks + (alloc_idx * map->elt_size));

  /* mark data block as in use */
  allocm_mark_inuse(map->space.db_map + alloc_idx);

  ER_TRACE("Allocated data block %d/%zu", alloc_idx, map->max_elts);

  return datablock;

error:
  return NULL;
} /* datablock_alloc() */

/**
 * \brief Deallocate a datablock.
 *
 * \param map The hashmap handle.
 * \param datablock The datablock to deallocate.
 *
 */
static void hashmap_db_dealloc(const struct hashmap* const map,
                               const void* const datablock) {
  if (NULL == datablock) {
    return;
  }
  size_t block_index = ((const uint8_t*)datablock - (uint8_t*)map->space.datablocks) / (map->elt_size);

  /* mark data block as available */
  allocm_mark_free(map->space.db_map + block_index);

  ER_TRACE("Dellocated data block %zu/%zu", block_index, map->max_elts);
} /* datablock_dealloc() */


/**
 * \brief Use linear probing, starting at the specified bucket, to
 * find a hashnode
 *
 * \param map The hashmap handle.
 * \param node The node to try to find.
 * \param bucket_index Set to hashed bucket index at start; filled with the
 * actual bucket.
 * \param node_index Filled with node index within the bucket the hashnode was
 * found in.
 */
static void hashmap_linear_probe(const struct hashmap* const map,
                                 const struct hashnode* const node,
                                 int* bucket_index,
                                 int* node_index) {
  for (int i = (*bucket_index + 1) % (int)map->n_buckets; i != *bucket_index;
       i++) {
    *node_index = darray_idx_query(map->space.buckets + i, node);
    if (*node_index != -1) {
      *bucket_index = i;
      return;
    }
    if (i + 1 == (int)map->n_buckets) {
      i = -1;
    }
  } /* for() */

  *bucket_index = -1;
  *node_index = -1;
} /* linear_probe() */

/**
 * \brief Compare hashnodes for equality
 *
 * \param n1 hashnode #1
 * \param n2 hashnode #2
 *
 * \return true if n1 = n2, false otherwise
 */
static int hashnode_cmp(const void* const n1, const void* const n2) {
  return memcmp(((const struct hashnode*)n1)->key,
                ((const struct hashnode*)n2)->key,
                RCSW_HASHMAP_KEYSIZE);
} /* hashnode_cmp() */

/*******************************************************************************
 * API Functions
 ******************************************************************************/
struct hashmap* hashmap_init(struct hashmap* map_in,
                             const struct hashmap_params* const params) {
  RCSW_FPC_NV(NULL,
              params != NULL,
              params->elt_size > 0,
              params->sort_thresh != 0,
              params->n_buckets > 0);
  RCSW_ER_MODULE_INIT();

  struct hashmap* map = rcsw_alloc(map_in,
                                    sizeof(struct hashmap),
                                    params->flags & RCSW_NOALLOC_HANDLE);
  RCSW_CHECK_PTR(map);

  map->flags = params->flags;
  map->hash = params->hash;
  map->n_buckets = params->n_buckets;
  map->elt_size = params->elt_size;
  map->max_elts = params->bsize * params->n_buckets;
  map->stats.n_nodes = 0;
  map->stats.n_collisions = 0;
  map->stats.n_adds = 0;
  map->stats.n_addfails = 0;
  map->sort_thresh = params->sort_thresh;
  map->sorted = false;
  map->space.elements = NULL;
  map->space.buckets = NULL;

  /* Allocate space for hashmap buckets */
  map->space.buckets = rcsw_alloc(params->meta,
                                   params->n_buckets * sizeof(struct darray),
                                   params->flags & RCSW_NOALLOC_META);

  RCSW_CHECK_PTR(map->space.buckets);

  /* Allocate space for hashnodes+datablocks+datablock alloc map */
  size_t element_bytes = hashmap_element_space(map->n_buckets,
                                               params->bsize,
                                               map->elt_size);
  map->space.elements = rcsw_alloc(params->elements,
                                   element_bytes,
                                   params->flags & RCSW_NOALLOC_DATA);

  RCSW_CHECK_PTR(map->space.elements);

  /* initialize free pool of hashnodes */
  map->space.db_map = (struct allocm_entry*)map->space.elements;
  for (size_t i = 0; i < map->max_elts; ++i) {
    allocm_mark_free(map->space.db_map + i);
  } /* for() */

  /* initialize buckets */
  struct darray_params bucket_params = {
    .init_size = params->bsize,
    .cmpe = hashnode_cmp,
    .printe = NULL,
    .max_elts = (int)params->bsize,
    .elt_size = sizeof(struct hashnode),

    .flags = RCSW_NOALLOC_HANDLE | RCSW_NOALLOC_DATA
  };

  if (params->flags & RCSW_DS_SORTED) {
    bucket_params.flags |= RCSW_DS_SORTED;
  }

  map->space.datablocks = (void*)((uint8_t*)map->space.elements + ds_meta_space(map->n_buckets * params->bsize));
  size_t db_space_per_bucket =
      darray_element_space(params->bsize, params->elt_size);
  map->space.hashnodes = (void*)((uint8_t*)map->space.datablocks + db_space_per_bucket * map->n_buckets);

  size_t hn_space_per_bucket =
      darray_element_space(params->bsize, sizeof(struct hashnode));

  for (size_t i = 0; i < map->n_buckets; i++) {
    /*
     * Each bucket is given a bsize chunk of the allocated space for
     * hashnodes
     */
    bucket_params.elements = (void*)((uint8_t*)map->space.hashnodes + i * hn_space_per_bucket);
    RCSW_CHECK(darray_init(map->space.buckets + i, &bucket_params) != NULL);
  } /* for() */

  ER_DEBUG("max_elts=%zu n_buckets=%zu bsize=%zu sort_thresh=%d flags=0x%08x",
       map->max_elts,
       map->n_buckets,
       params->bsize,
       map->sort_thresh,
       map->flags);
  return map;

error:
  hashmap_destroy(map);
  errno = EAGAIN;
  return NULL;
} /* hashmap_init() */

void hashmap_destroy(struct hashmap* map) {
  RCSW_FPC_V(NULL != map);

  for (size_t i = 0; i < map->n_buckets; i++) {
    darray_destroy(map->space.buckets + i);
  }

  rcsw_free(map->space.elements, map->flags & RCSW_NOALLOC_DATA);
  rcsw_free(map->space.buckets, map->flags & RCSW_NOALLOC_META);
  rcsw_free(map, map->flags & RCSW_NOALLOC_HANDLE);
} /* hashmap_destroy() */

struct darray* hashmap_query(const struct hashmap* const map,
                             const void* const key,
                             uint32_t* const hash_out) {
  RCSW_FPC_NV(NULL, map != NULL, key != NULL);

  uint32_t hash = map->hash(key, RCSW_HASHMAP_KEYSIZE);
  uint32_t bucket_n = hash % map->n_buckets;

  if (hash_out != NULL) {
    *hash_out = hash;
  }
  return map->space.buckets + bucket_n;
} /* hashmap_query() */

void* hashmap_data_get(struct hashmap* const map, const void* const key) {
  RCSW_FPC_NV(NULL, map != NULL, key != NULL);

  uint32_t hash = 0;
  int node_index, bucket_index;
  struct hashnode node = { .hash = hash, .data = NULL };

  /* memset() needed to make hashnode_cmp() work */
  memset(node.key, 0, sizeof(node.key));
  memcpy(node.key, key, RCSW_HASHMAP_KEYSIZE);

  struct darray* bucket = hashmap_query(map, key, &hash);

  map->last_used = bucket;
  bucket_index = hashmap_bucket_index(map, bucket);

  node_index = darray_idx_query(bucket, &node);

  /*
   * If linear probing is not enabled, then we know for sure the element
   * isn't in the hashmap, because it wasn't in the bucket the key hashed to.
   */
  if (node_index == -1) {
    if (!(map->flags & RCSW_DS_HASHMAP_LINPROB)) {
      ER_DEBUG("Key not found in bucket %d", bucket_index);
      return NULL;
    }
    hashmap_linear_probe(map, &node, &bucket_index, &node_index);
    RCSW_CHECK(bucket_index != -1);
    RCSW_CHECK(node_index != -1);
    bucket = map->space.buckets + bucket_index;
  }

  /*
   * This is needed because the alignment for a void* on SPARC is 32 bits,
   * and if your data is not 32-bit aligned, you will get a unaligned address
   * trap. So use a uint8_t* to get the data, then copy to a local variable to
   * get correct alignment.
   */
  darray_idx_serve(bucket, &node, (size_t)node_index);

  return node.data;

error:
  return NULL;
} /* hashmap_data_get() */

status_t hashmap_add(struct hashmap* const map,
                     const void* const key,
                     const void* const data) {
  RCSW_FPC_NV(ERROR, map != NULL, key != NULL);

  uint32_t hash = 0;
  size_t bucket_index;
  int i;
  struct darray* bucket = hashmap_query(map, key, &hash);
  map->last_used = bucket;
  bucket_index = hashmap_bucket_index(map, bucket);

  if (darray_isfull(bucket)) {
    if (!(map->flags & RCSW_DS_HASHMAP_LINPROB)) {
      ER_DEBUG("Bucket %zu is full (%zu elements): cannot add new hashnode",
           bucket_index,
           bucket->current);
      map->stats.n_addfails++;
      return ERROR;
    }
    /* Loop through all buckets, starting from the one we originally hashed
     * to, incrementing indices, until a suitable bucket is found.
     */
    bucket = NULL;
    for (i = (int)(bucket_index + 1) % (int)map->n_buckets;
         i != (int)bucket_index;
         i++) {
      if (map->space.buckets[i].current <
          (size_t)map->space.buckets[i].max_elts) {
        bucket = map->space.buckets + i;
        break;
      }
      if (i + 1 == (int)map->n_buckets) {
        i = -1;
      }
    } /* for() */

    if (!bucket) {
      ER_DEBUG("All buckets full: Cannot add new hashnode");
      map->stats.n_addfails++;
      return ERROR;
    }
    ER_DEBUG("Linear probing found bucket %d", i);
  } /* if(bucket->current >= bucket->max_elts) */

  void* datablock = hashmap_db_alloc(map);
  RCSW_CHECK_PTR(datablock);
  ds_elt_copy(datablock, data, map->elt_size);

  struct hashnode node = { .data = datablock, .hash = hash };
  /* memset() needed to make hashnode_cmp() work */
  memset(node.key, 0, sizeof(node.key));
  memcpy(node.key, key, RCSW_HASHMAP_KEYSIZE);

  /* check for duplicates */
  if (darray_idx_query(bucket, &node) != -1) {
    errno = EAGAIN;
    ER_ERR("Node already exists in bucket");
    return ERROR;
  }

  ER_CHECK(darray_insert(bucket, &node, bucket->current) == OK,
                "could not append node to bucket");
  map->stats.n_collisions += (bucket->current != 1); /* if not 1, wasn't 0 before
                                                        (COLLISION) */
  map->stats.n_nodes++;
  map->stats.n_adds++;

  /*
   * Sort the hashmap if the following are met:
   *
   * - RCSW_DS_SORTED was passed
   * - The sort threshold has been reached
   */
  if ((map->flags & RCSW_DS_SORTED) && map->sort_thresh != -1 &&
      (map->stats.n_adds % (size_t)map->sort_thresh) == 0) {
    hashmap_sort(map);
  }
  map->sorted = bucket->sorted;

  return OK;

error:
  ++map->stats.n_addfails;
  return ERROR;
} /* hashmap_add() */

status_t hashmap_remove(struct hashmap* const map, const void* const key) {
  RCSW_FPC_NV(ERROR, map != NULL, key != NULL);

  uint32_t hash = 0;
  struct darray* bucket = hashmap_query(map, key, &hash);
  map->last_used = bucket;

  struct hashnode node = { .hash = hash, .data = NULL };
  /* memset() needed to make hashnode_cmp() work */
  memset(node.key, 0, sizeof(node.key));
  memcpy(&node.key, key, RCSW_HASHMAP_KEYSIZE);

  int node_index = darray_idx_query(bucket, &node);
  int bucket_index = hashmap_bucket_index(map, bucket);

  if (node_index == -1) {
    if (!(map->flags & RCSW_DS_HASHMAP_LINPROB)) {
      ER_DEBUG("No key found in bucket %d for removal (probing disabled)",
           bucket_index);
      goto error; /* normal return */
    }
    hashmap_linear_probe(map, &node, &bucket_index, &node_index);
    if (bucket_index == -1 || node_index == -1) {
      ER_DEBUG("No matching key found in hashmap");
      goto error; /* normal return */
    }
    bucket = map->space.buckets + bucket_index;
  } /* if (node_index == -1) */

  /* deallocate datablock */
  struct hashnode* node_p = darray_data_get(bucket, (size_t)node_index);
  hashmap_db_dealloc(map, node_p->data);

  /* remove hashnode */
  if (darray_remove(bucket, NULL, (size_t)node_index) != OK) {
    ER_ERR("Failed to remove node from bucket");
    errno = EAGAIN;
    return ERROR;
  }

  map->stats.n_nodes--;
  map->sorted = bucket->sorted;
error:
  return OK;
} /* hashmap_remove() */

status_t hashmap_sort(struct hashmap* const map) {
  RCSW_FPC_NV(ERROR, map != NULL);

  for (size_t i = 0; i < map->n_buckets; i++) {
    darray_sort(&map->space.buckets[i], ekEXEC_ITER);
  } /* for() */

  map->sorted = true;
  return OK;
} /* hashmap_sort() */

status_t hashmap_clear(const struct hashmap* const map) {
  RCSW_FPC_NV(ERROR, map != NULL);

  for (size_t i = 0; i < map->n_buckets; ++i) {
    RCSW_CHECK(darray_clear(map->space.buckets + i) == OK);
  }
  return OK;

error:
  return ERROR;
} /* hashmap_clear() */

status_t hashmap_gather(const struct hashmap* const map,
                        struct hashmap_stats* const stats) {
  RCSW_FPC_NV(ERROR, map != NULL, stats != NULL);

  /* copy over all current stats */
  *stats = map->stats;

  stats->n_buckets = map->n_buckets;
  stats->collision_ratio = ((double)stats->n_collisions / map->stats.n_adds);
  stats->sorted = map->sorted;

  /* get highest/lowest/average bucket utilization */
  size_t max = 0;
  size_t min = -1;
  double average = 0;
  size_t i;

  for (i = 0; i < map->n_buckets; i++) {
    max = RCSW_MAX(map->space.buckets[i].current, max);
    min = RCSW_MIN(map->space.buckets[i].current, min);
    average +=
        ((double)map->space.buckets[i].current) / map->space.buckets[0].max_elts;
  }

  stats->average_util = average / map->n_buckets;
  stats->max_util = (double)max / map->space.buckets[0].max_elts;
  stats->min_util = (double)min / map->space.buckets[0].max_elts;

  return OK;
} /* hashmap_gather() */

void hashmap_print(const struct hashmap* const map) {
  if (map == NULL) {
    DPRINTF(RCSW_ER_MODNAME " : < NULL >\n");
    return;
  }

  struct hashmap_stats stats;
  RCSW_CHECK(hashmap_gather(map, &stats) == OK);

  DPRINTF("\n******************** Hashmap Print ********************\n");
  DPRINTF("Total buckets   : %zu\n", stats.n_buckets);
  DPRINTF("Bucket capacity : %d\n", map->space.buckets[0].max_elts);
  DPRINTF("Total nodes     : %zu\n", stats.n_nodes);
  DPRINTF("Successful adds : %zu\n", stats.n_adds);
  DPRINTF("Failed adds     : %zu\n", stats.n_addfails);
  DPRINTF("Collisions      : %zu\n", stats.n_collisions);
  DPRINTF("Collision ratio : %.8f\n", stats.collision_ratio);
  DPRINTF("Map sorted      : %s\n", (stats.sorted) ? "yes" : "no");

  DPRINTF("Max bucket utilization     : %.8f\n", stats.max_util);
  DPRINTF("Min bucket utilization     : %.8f\n", stats.min_util);
  DPRINTF("Average bucket utilization : %.8f\n", stats.average_util);
  DPRINTF("\n");

error:
  return;
} /* hashmap_print() */

void hashmap_print_dist(const struct hashmap* const map) {
  if (NULL == map) {
    DPRINTF(RCSW_ER_MODNAME " : < NULL >\n");
    return;
  }
  DPRINTF("\n----------------------------------------\n");
  DPRINTF("Hashmap Utilization Distribution");
  DPRINTF("\n----------------------------------------\n\n");

  /* get maximum bucket node count */
  size_t max_node_count = 0;
  for (size_t i = 0; i < map->n_buckets; ++i) {
    max_node_count =
        RCSW_MAX(darray_size(&map->space.buckets[i]), max_node_count);
  }
  if (max_node_count == 0) {
    DPRINTF(RCSW_ER_MODNAME " : < empty >\n");
    return;
  }
  size_t xmax = RCSW_MIN(max_node_count, (size_t)80);

  for (size_t i = 0; i < map->n_buckets; ++i) {
    DPRINTF("Bucket %-4zu| ", i);
    if (darray_size(&map->space.buckets[i]) == 0) {
      DPRINTF("\n");
      continue;
    }

    double scale = darray_size(&map->space.buckets[i]) / (double)max_node_count;
    size_t fill = (size_t)(scale * xmax);
    for (size_t j = 0; j < fill; ++j) {
      DPRINTF("*");
    } /* for(j..) */

    DPRINTF("\n");
  }
  DPRINTF("\nHistogram normalized w.r.t. max bucket fill.\n");
} /* hashmap_print_dist() */


END_C_DECLS
