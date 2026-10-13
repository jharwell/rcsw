/**
 * \file
 *
 * \copyright 2017 John Harwell
 *
 * SPDX-License-Identifier: MIT
 */
/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/hashmap.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "rcsw/ds/allocm.h"

#define RCSW_ER_MODNAME RCSW_ER_MODNAME_BUILDER("rcsw", "ds", "hashmap")
#define RCSW_ER_MODID LOG4CL_DS_HASHMAP
#include "rcsw/algorithm/sort.h"
#include "rcsw/core/alloc.h"
#include "rcsw/core/fpc.h"
#include "rcsw/ds/darray.h"
#include "rcsw/er/client.h"

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
                                   const struct darray* const  bucket) {
  return (size_t)(bucket - map->space.buckets) % map->cfg.n_buckets;
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
static struct darray* hashmap_query(const struct hashmap* const map,
                                    const void* const           key,
                                    uint32_t* const             hash_out) {
  RCSW_FPC_NV(NULL, map != NULL, key != NULL);

  uint32_t hash = 0;
  map->cfg.hash(key, map->cfg.key_len, &hash);
  uint32_t bucket_n = (uint32_t)(hash % map->cfg.n_buckets);

  if (hash_out != NULL) {
    *hash_out = hash;
  }
  return map->space.buckets + bucket_n;
} /* hashmap_query() */

/**
 * \brief Allocate a datablock.
 *
 * \param map The hashmap handle.
 *
 * \return The allocated datablock, or NULL if no valid block could be found.
 *
 */
static dptr_t* hashmap_db_alloc(const struct hashmap* const map) {
  int alloc_idx = allocm_alloc(map->space.db_map);
  RCSW_CHECK(-1 != alloc_idx);
  dptr_t* datablock = (void*)((uint8_t*)map->space.datablocks +
                              ((size_t)alloc_idx * map->cfg.elt_size));

  ER_TRACE("Allocated data block %d/%zu", alloc_idx, map->max_elts);

  return datablock;

error:
  return NULL;
}

/**
 * \brief Deallocate a datablock.
 *
 * \param map The hashmap handle.
 * \param datablock The datablock to deallocate.
 *
 */
static void hashmap_db_dealloc(const struct hashmap* const map,
                               const void* const           datablock) {
  if (NULL == datablock) {
    return;
  }
  size_t block_index =
    (size_t)((const uint8_t*)datablock - (uint8_t*)map->space.datablocks) /
    (map->cfg.elt_size);

  allocm_free(map->space.db_map, block_index);

  ER_TRACE("Deallocated data block %zu/%zu", block_index, map->max_elts);
}

/**
 * \brief Locate the hashnode for a key.
 *
 * Looks in the bucket the key hashes to and, if linear probing is enabled, in
 * the next max_probe buckets: no node has ever been placed further from its
 * home bucket than that.
 *
 * \param map The hashmap handle.
 * \param node A hashnode carrying the (zero-padded) key.
 * \param home Index of the bucket the key hashes to.
 * \param bucket_index Set to the bucket containing the key, or -1.
 * \param node_index Set to the key's index in that bucket, or -1.
 */
static void hashmap_find(const struct hashmap* const  map,
                         const struct hashnode* const node,
                         size_t                       home,
                         int*                         bucket_index,
                         int*                         node_index) {
  size_t n_probe = map->max_probe + 1; /* max_probe is 0 without LINPROB */

  for (size_t k = 0; k < n_probe; ++k) {
    size_t b   = (home + k) % map->cfg.n_buckets;
    int    idx = darray_idx_query(map->space.buckets + b, node);
    if (-1 != idx) {
      *bucket_index = (int)b;
      *node_index   = idx;
      return;
    }
  } /* for(k..) */
  *bucket_index = -1;
  *node_index   = -1;
} /* hashmap_find() */

/**
 * \brief Build the lookup hashnode for \p key.
 *
 * Only key_len bytes are read from \p key; the rest of the stored key is
 * zeroed, so comparing the full RCSW_HASHMAP_KEYSIZE bytes is exact.
 */
static void hashmap_make_node(const struct hashmap* const map,
                              struct hashnode*            node,
                              const void*                 key,
                              uint32_t                    hash) {
  memset(node, 0, sizeof(*node));
  memcpy(node->key, key, map->cfg.key_len);
  node->hash = hash;
} /* hashmap_make_node() */

/**
 * \brief Release everything a (possibly partially) initialized hashmap owns.
 *
 * \param map The hashmap handle, or NULL.
 * \param n_buckets_init How many buckets were successfully initialized.
 */
static void hashmap_teardown(struct hashmap* const map, size_t n_buckets_init) {
  if (NULL == map) {
    return;
  }
  if (NULL != map->space.buckets) {
    for (size_t i = 0; i < n_buckets_init; i++) {
      darray_destroy(map->space.buckets + i);
    } /* for(i..) */
  }
  rcsw_free(map->space.elements, map->cfg.flags & RCSW_NOALLOC_DATA);
  rcsw_free(map->space.buckets, map->cfg.flags & RCSW_NOALLOC_META);
  rcsw_free(map, map->cfg.flags & RCSW_NOALLOC_HANDLE);
} /* hashmap_teardown() */

/**
 * \brief Compare hashnodes for equality
 *
 * \param n1 hashnode #1
 * \param n2 hashnode #2
 *
 * \return true if n1 = n2, false otherwise
 */
static int hashnode_cmp(const void* const n1, const void* const n2) {
  /*
   * Compare the key bytes by offset rather than through struct pointers, so
   * this does not depend on how the bucket storage is laid out.
   */
  return memcmp((const uint8_t*)n1 + offsetof(struct hashnode, key),
                (const uint8_t*)n2 + offsetof(struct hashnode, key),
                RCSW_HASHMAP_KEYSIZE);
}

/*******************************************************************************
 * Public API
 ******************************************************************************/
/* NOLINTNEXTLINE(readability-function-size) */
struct hashmap* hashmap_init(struct hashmap*                    map_in,
                             const struct hashmap_config* const params) {
  RCSW_FPC_NV(NULL,
              params != NULL,
              params->elt_size > 0,
              params->n_buckets > 0,
              params->hash != NULL,
              params->bsize > 0,
              params->key_len <= RCSW_HASHMAP_KEYSIZE);
  /* Every operation hashes a key */
  ER_ASSERT(NULL != params->hash, "hashmap requires hash()");
  RCSW_ER_MODULE_INIT();

  size_t          n_buckets_init = 0;
  struct hashmap* map =
    rcsw_alloc(map_in,
               sizeof(struct hashmap),
               params->flags & (RCSW_NOALLOC_HANDLE | RCSW_ZALLOC));
  RCSW_CHECK_PTR(map);

  map->cfg = *params;
  map->cfg.key_len =
    (0 == params->key_len) ? RCSW_HASHMAP_KEYSIZE : params->key_len;
  map->max_elts           = params->bsize * params->n_buckets;
  map->stats.n_nodes      = 0;
  map->stats.n_collisions = 0;
  map->stats.n_adds       = 0;
  map->stats.n_addfails   = 0;
  map->max_probe          = 0;
  map->sorted             = false;
  map->space.elements     = NULL;
  map->space.buckets      = NULL;

  /* Allocate space for hashmap buckets */
  map->space.buckets =
    rcsw_alloc(params->meta,
               params->n_buckets * sizeof(struct darray),
               params->flags & (RCSW_NOALLOC_META | RCSW_ZALLOC));

  RCSW_CHECK_PTR(map->space.buckets);

  /* Allocate space for hashnodes+datablocks+datablock alloc map */
  size_t element_bytes =
    hashmap_element_space(map->cfg.n_buckets, params->bsize, map->cfg.elt_size);
  map->space.elements =
    rcsw_alloc(params->elements,
               element_bytes,
               params->flags & (RCSW_NOALLOC_DATA | RCSW_ZALLOC));

  RCSW_CHECK_PTR(map->space.elements);

  /* initialize free pool of hashnodes */
  map->space.db_map = (struct allocm_entry*)map->space.elements;
  allocm_init(map->space.db_map, map->max_elts);

  /* initialize buckets */
  struct darray_config bucket_params = {
    .init_size = params->bsize,
    .cmpe      = hashnode_cmp,
    .printe    = NULL,
    .max_elts  = (int)params->bsize,
    .elt_size  = sizeof(struct hashnode),

    .flags = RCSW_NOALLOC_HANDLE | RCSW_NOALLOC_DATA};

  if (params->flags & RCSW_DS_SORTED) {
    bucket_params.flags |= RCSW_DS_SORTED;
  }

  map->space.datablocks =
    (void*)((uint8_t*)map->space.elements +
            ds_meta_space(map->cfg.n_buckets * params->bsize));
  size_t db_space_per_bucket =
    darray_element_space(params->bsize, params->elt_size);
  /* padded so the hashnodes start dptr_t-aligned, matching
   * hashmap_element_space() */
  map->space.hashnodes =
    (void*)((uint8_t*)map->space.datablocks +
            utils_align_size(db_space_per_bucket * map->cfg.n_buckets,
                             sizeof(dptr_t)));

  size_t hn_space_per_bucket =
    darray_element_space(params->bsize, sizeof(struct hashnode));

  for (size_t i = 0; i < map->cfg.n_buckets; i++) {
    /*
     * Each bucket is given a bsize chunk of the allocated space for
     * hashnodes
     */
    bucket_params.elements =
      (void*)((uint8_t*)map->space.hashnodes + (i * hn_space_per_bucket));
    RCSW_CHECK(darray_init(map->space.buckets + i, &bucket_params) != NULL);
    ++n_buckets_init;
  } /* for() */

  ER_DEBUG("max_elts=%zu n_buckets=%zu bsize=%zu flags=0x%08x",
           map->max_elts,
           map->cfg.n_buckets,
           params->bsize,
           map->cfg.flags);
  return map;

error:
  /* Only undo what was set up: buckets may be NULL or partially initialized */
  hashmap_teardown(map, n_buckets_init);
  errno = ENOMEM;
  return NULL;
} /* hashmap_init() */

void hashmap_destroy(struct hashmap* map) {
  RCSW_FPC_V(NULL != map);
  if (map->cfg.flags & RCSW_NOALLOC_DATA) {
    allocm_init(map->space.db_map, map->max_elts);
  }
  hashmap_teardown(map, map->cfg.n_buckets);
} /* hashmap_destroy() */

void* hashmap_data_get(struct hashmap* const map, const void* const key) {
  RCSW_FPC_NV(NULL, map != NULL, key != NULL);

  uint32_t        hash   = 0;
  struct darray*  bucket = hashmap_query(map, key, &hash);
  struct hashnode node;
  hashmap_make_node(map, &node, key, hash);

  int bucket_index = -1;
  int node_index   = -1;
  hashmap_find(map,
               &node,
               hashmap_bucket_index(map, bucket),
               &bucket_index,
               &node_index);
  if (-1 == node_index) {
    return NULL; /* not found: not an error */
  }

  /*
   * Copy the hashnode out rather than dereferencing it in place, so this does
   * not depend on how the bucket storage is laid out.
   */
  darray_idx_serve(map->space.buckets + bucket_index, &node, (size_t)node_index);
  return node.data;
}

status_t hashmap_add(struct hashmap* const map,
                     const void* const     key,
                     const void* const     data) {
  RCSW_FPC_NV(ERROR, map != NULL, key != NULL, data != NULL);

  uint32_t        hash   = 0;
  struct darray*  bucket = hashmap_query(map, key, &hash);
  size_t          home   = hashmap_bucket_index(map, bucket);
  struct hashnode node;
  hashmap_make_node(map, &node, key, hash);

  /* Reject duplicates in every bucket the key could occupy */
  int found_bucket = -1;
  int found_node   = -1;
  hashmap_find(map, &node, home, &found_bucket, &found_node);
  if (-1 != found_node) {
    ER_ERR("Key already exists in bucket %d with %zu elements",
           found_bucket,
           darray_size(map->space.buckets + found_bucket));
    errno = EEXIST;
    goto error;
  }

  if (darray_isfull(bucket)) {
    bucket = NULL;
    if (map->cfg.flags & RCSW_DS_HASHMAP_LINPROB) {
      /* Linear probe for the next bucket with space */
      for (size_t k = 1; k < map->cfg.n_buckets; ++k) {
        struct darray* candidate =
          map->space.buckets + ((home + k) % map->cfg.n_buckets);
        if (!darray_isfull(candidate)) {
          bucket         = candidate;
          map->max_probe = RCSW_MAX(map->max_probe, k);
          break;
        }
      } /* for(k..) */
    }
    if (NULL == bucket) {
      ER_DEBUG("No bucket has space: cannot add new hashnode");
      errno = ENOSPC;
      goto error;
    }
  }
  void* datablock = hashmap_db_alloc(map);
  if (NULL == datablock) {
    errno = ENOSPC;
    goto error;
  }
  node.data = datablock;
  memcpy(datablock, data, map->cfg.elt_size);

  if (darray_insert(bucket, &node, bucket->current) != OK) {
    ER_ERR("Bucket insertion failed!");
    hashmap_db_dealloc(map, datablock);
    goto error;
  }
  /* if the bucket was not empty before, this is a collision */
  map->stats.n_collisions += (bucket->current != 1);
  map->stats.n_nodes++;
  map->stats.n_adds++;
  map->sorted = bucket->sorted;
  return OK;

error:
  ++map->stats.n_addfails;
  return ERROR;
}

status_t hashmap_remove(struct hashmap* const map, const void* const key) {
  RCSW_FPC_NV(ERROR, map != NULL, key != NULL);

  uint32_t        hash   = 0;
  struct darray*  bucket = hashmap_query(map, key, &hash);
  struct hashnode node;
  hashmap_make_node(map, &node, key, hash);

  int bucket_index = -1;
  int node_index   = -1;
  hashmap_find(map,
               &node,
               hashmap_bucket_index(map, bucket),
               &bucket_index,
               &node_index);
  if (-1 == node_index) {
    ER_DEBUG("No matching key found in hashmap");
    errno = ENOENT;
    return ERROR;
  }
  bucket = map->space.buckets + bucket_index;

  /* deallocate datablock */
  struct hashnode victim;
  darray_idx_serve(bucket, &victim, (size_t)node_index);
  hashmap_db_dealloc(map, victim.data);

  /* remove hashnode */
  RCSW_CHECK(OK == darray_remove(bucket, NULL, (size_t)node_index));
  map->stats.n_nodes--;
  map->sorted = bucket->sorted;
  return OK;

error:
  return ERROR;
}

status_t hashmap_sort(struct hashmap* const map) {
  RCSW_FPC_NV(ERROR, map != NULL);

  for (size_t i = 0; i < map->cfg.n_buckets; i++) {
    RCSW_CHECK(OK == darray_sort(&map->space.buckets[i], EXEC_ITER));
  } /* for() */

  map->sorted = true;
  return OK;

error:
  return ERROR;
} /* hashmap_sort() */

status_t hashmap_map(struct hashmap* const map, void (*f)(void* e)) {
  RCSW_FPC_NV(ERROR, map != NULL, f != NULL);

  for (size_t i = 0; i < map->cfg.n_buckets; i++) {
    darray_map(&map->space.buckets[i], f);
  }
  return OK;
} /* hashmap_map() */

status_t hashmap_inject(struct hashmap* const map,
                        void (*f)(void* e, void* result),
                        void* result) {
  RCSW_FPC_NV(ERROR, map != NULL, f != NULL, result != NULL);

  for (size_t i = 0; i < map->cfg.n_buckets; i++) {
    darray_inject(&map->space.buckets[i], f, result);
  }
  return OK;
} /* hashmap_inject() */

status_t hashmap_clear(struct hashmap* const map) {
  RCSW_FPC_NV(ERROR, map != NULL);

  for (size_t i = 0; i < map->cfg.n_buckets; ++i) {
    RCSW_CHECK(darray_clear(map->space.buckets + i) == OK);
  }
  allocm_init(map->space.db_map, map->max_elts);
  map->stats.n_nodes = 0;
  map->max_probe     = 0;
  map->sorted        = false;
  return OK;

error:
  return ERROR;
} /* hashmap_clear() */

/**
 * \brief Expected lookup cost of the current keys relative to a random hash.
 *
 * With b_j keys hashing to bucket j, n keys and m buckets, this is
 *
 *   sum_j b_j (b_j + 1) / 2  /  ((n / 2m) (n + 2m - 1))
 *
 * The numerator is the total number of comparisons needed to look up every
 * key in a chained table; the denominator is its expected value for a
 * uniformly random hash. The keys' home buckets are counted, not where linear
 * probing placed them, so the counts are recomputed from the stored hashes in
 * windows of a fixed number of buckets to avoid allocating.
 */
static double hashmap_hash_quality(const struct hashmap* const map) {
  double n = (double)map->stats.n_nodes;
  double m = (double)map->cfg.n_buckets;
  if (0 == map->stats.n_nodes) {
    return 0.0;
  }
  double sum = 0.0;

  if (!(map->cfg.flags & RCSW_DS_HASHMAP_LINPROB)) {
    /* Every key is in its home bucket */
    for (size_t j = 0; j < map->cfg.n_buckets; ++j) {
      double b = (double)darray_size(&map->space.buckets[j]);
      sum += b * (b + 1.0) / 2.0;
    } /* for(j..) */
  } else {
    size_t counts[256];
    for (size_t base = 0; base < map->cfg.n_buckets;
         base += RCSW_ARRAY_ELTS(counts)) {
      size_t width = RCSW_MIN(RCSW_ARRAY_ELTS(counts), map->cfg.n_buckets - base);
      memset(counts, 0, sizeof(counts));
      for (size_t i = 0; i < map->cfg.n_buckets; ++i) {
        const struct darray* bucket = &map->space.buckets[i];
        for (size_t k = 0; k < darray_size(bucket); ++k) {
          const struct hashnode* node = darray_data_get(bucket, k);
          size_t                 home = node->hash % map->cfg.n_buckets;
          if (home >= base && home < base + width) {
            ++counts[home - base];
          }
        } /* for(k..) */
      } /* for(i..) */
      for (size_t j = 0; j < width; ++j) {
        double b = (double)counts[j];
        sum += b * (b + 1.0) / 2.0;
      } /* for(j..) */
    } /* for(base..) */
  }
  return sum / ((n / (2.0 * m)) * (n + 2.0 * m - 1.0));
} /* hashmap_hash_quality() */

status_t hashmap_gather(const struct hashmap* const map,
                        struct hashmap_stats* const stats) {
  RCSW_FPC_NV(ERROR, map != NULL, stats != NULL);

  /* copy over all current stats */
  *stats = map->stats;

  stats->collision_ratio =
    (0 == map->stats.n_adds)
      ? 0.0
      : ((double)stats->n_collisions / (double)map->stats.n_adds);
  stats->sorted = map->sorted;

  /* get highest/lowest/average bucket utilization */
  size_t max     = 0;
  size_t min     = SIZE_MAX;
  double average = 0;
  double bsize   = (double)map->space.buckets[0].max_elts;

  for (size_t i = 0; i < map->cfg.n_buckets; i++) {
    size_t n = map->space.buckets[i].current;
    max      = RCSW_MAX(n, max);
    min      = RCSW_MIN(n, min);
    average += (double)n / bsize;
  } /* for(i..) */

  stats->average_util = average / (double)map->cfg.n_buckets;
  stats->max_util     = (double)max / bsize;
  stats->min_util     = (double)min / bsize;
  stats->hash_quality = hashmap_hash_quality(map);
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
  DPRINTF("Total buckets   : %zu\n", map->cfg.n_buckets);
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
  DPRINTF("Hash quality (1.0 = random) : %.4f\n", stats.hash_quality);
  DPRINTF("\n");

error:
  return;
} /* hashmap_print() */

END_C_DECLS
