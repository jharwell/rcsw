/**
 * @file hashmap.c
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
#include "rcsw/ds/hashmap.h"
#include <math.h>
#include "rcsw/algorithm/sort.h"
#include "rcsw/common/dbg.h"
#include "rcsw/common/fpc.h"
#include "rcsw/ds/darray.h"
#include "rcsw/utils/hash.h"

/*******************************************************************************
 * Constant Definitions
 ******************************************************************************/
#define MODULE_ID M_DS_HASHMAP

/*******************************************************************************
 * Forward Declarations
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * @brief Allocate a datablock.
 *
 * @param map The hashmap handle.
 *
 * @return The allocated datablock, or NULL if no valid block could be found.
 *
 */
static void* datablock_alloc(const struct hashmap* map);

/**
 * @brief Deallocate a datablock.
 *
 * @param map The hashmap handle.
 * @param datablock The datablock to deallocate.
 *
 */
static void datablock_dealloc(const struct hashmap* map,
                              const uint8_t* datablock);
/**
 * @brief Use linear probing, starting at the specified bucket, to
 * find a hashnode
 *
 * @param map The hashmap handle.
 * @param node The node to try to find.
 * @param bucket_index Set to hashed bucket index at start; filled with the
 * actual bucket.
 * @param node_index Filled with node index within the bucket the hashnode was
 * found in.
 */
static void linear_probe(const struct hashmap* map,
                         const struct hashnode* node,
                         int* bucket_index,
                         int* node_index);

/**
 * @brief Compare hashnodes for equality
 *
 * @param n1 hashnode #1
 * @param n2 hashnode #2
 *
 * @return TRUE if n1 = n2, FALSE otherwise
 */
static int hashnode_cmp(const void* n1, const void* n2);

/*******************************************************************************
 * API Functions
 ******************************************************************************/
struct hashmap* hashmap_init(struct hashmap* map_in,
                             const struct ds_params* const params) {
  RCSW_FPC_NV(NULL,
            params != NULL,
            params->type.hm.hash != NULL,
            params->el_size > 0,
            params->type.hm.sort_thresh != 0,
            params->type.hm.n_buckets > 0,
            params->type.hm.n_buckets > 0);

  struct hashmap* map = NULL;
  size_t i;

  if (params->flags & DS_APP_DOMAIN_HANDLE) {
    RCSW_CHECK_PTR(map_in);
    map = map_in;
  } else {
    map = malloc(sizeof(struct hashmap));
    RCSW_CHECK_PTR(map);
  }
  map->flags = params->flags;

  if (params->flags & DS_APP_DOMAIN_NODES) {
    RCSW_CHECK_PTR(params->nodes);
    map->buckets = (struct darray*)params->nodes;
  } else {
    map->buckets = calloc(params->type.hm.n_buckets, sizeof(struct darray));
    RCSW_CHECK_PTR(map->buckets);
  }

  /* validate keysize */
  SOFT_ASSERT(params->type.hm.keysize <= HASHMAP_MAX_KEYSIZE,
              "ERROR: Keysize (%zu) > HASHMAP_MAX_KEYSIZE (%d)\n",
              params->type.hm.keysize,
              HASHMAP_MAX_KEYSIZE);
  map->keysize = params->type.hm.keysize;

  map->hash = params->type.hm.hash;
  map->n_buckets = params->type.hm.n_buckets;
  map->el_size = params->el_size;
  map->max_elts = params->type.hm.bsize * params->type.hm.n_buckets;
  map->n_nodes = 0;
  map->n_collisions = 0;
  map->n_adds = 0;
  map->n_addfails = 0;
  map->sort_thresh = params->type.hm.sort_thresh;
  map->sorted = FALSE;

  /* allocate space for hashnodes (the elements in this data structure) */
  if (params->flags & DS_APP_DOMAIN_DATA) {
    map->elements = params->elements;
  } else {
    map->elements = malloc(hashmap_element_space(
        params->type.hm.n_buckets * params->type.hm.bsize, params->el_size));
  }
  RCSW_CHECK_PTR(map->elements);

  /* initialize free pool of hashnodes */
  for (i = 0; i < map->max_elts; ++i) {
    ((int*)(map->elements))[i] = -1;
  } /* for() */

  /* initialize buckets */
  struct ds_params da_params = {
      .type = {.da =
                   {
                       .init_size = params->type.hm.bsize,
                   }},
      .cmpe = hashnode_cmp,
      .printe = NULL,
      .max_elts = (int)params->type.hm.bsize,
      .el_size = sizeof(struct hashnode),
      .tag = DS_DARRAY,
      .flags = DS_APP_DOMAIN_HANDLE | DS_APP_DOMAIN_DATA};
  if (params->flags & DS_KEEP_SORTED) {
    da_params.flags |= DS_KEEP_SORTED;
  }

  uint8_t* db_start =
      map->elements + ds_calc_meta_space(map->n_buckets * params->type.hm.bsize);
  uint8_t* hashnode_start =
      db_start + map->n_buckets * params->type.hm.bsize * map->el_size;

  for (i = 0; i < map->n_buckets; i++) {
    /*
     * Each bucket is given a bsize chunk of the allocated space for
     * hashnodes
     */
    da_params.elements =
        hashnode_start + i * (params->type.hm.bsize * sizeof(struct hashnode));
    RCSW_CHECK(darray_init(map->buckets + i, &da_params) != NULL);
  } /* for() */

  DBGD("max_elts=%zu n_buckets=%zu bsize=%zu sort_thresh=%d flags=0x%08x\n",
       map->max_elts,
       map->n_buckets,
       params->type.hm.bsize,
       map->sort_thresh,
       map->flags);
  return map;

error:
  hashmap_destroy(map);
  errno = EAGAIN;
  return NULL;
} /* hashmap_init() */

void hashmap_destroy(struct hashmap* map) {
  RCSW_FPC_V(NULL != map, NULL != map->buckets);

  size_t i;
  for (i = 0; i < map->n_buckets; i++) {
    darray_destroy(map->buckets + i);
  }

  if (!(map->flags & DS_APP_DOMAIN_DATA)) {
    free(map->elements);
  }

  if (!(map->flags & DS_APP_DOMAIN_NODES)) {
    free(map->buckets);
  }
  if (!(map->flags & DS_APP_DOMAIN_HANDLE)) {
    free(map);
  }
} /* hashmap_destroy() */

struct darray* hashmap_query(const struct hashmap* const map,
                             const void* const key,
                             uint32_t* const hash_out) {
  RCSW_FPC_NV(NULL, map != NULL, key != NULL);

  uint32_t hash = map->hash(key, map->keysize);
  uint32_t bucket_n = hash % map->n_buckets;

  if (hash_out != NULL) {
    *hash_out = hash;
  }
  return map->buckets + bucket_n;
} /* hashmap_query() */

void* hashmap_data_get(struct hashmap* const map, const void* const key) {
  RCSW_FPC_NV(NULL, map != NULL, key != NULL);

  uint32_t hash = 0;
  int node_index, bucket_index;
  struct hashnode node = {.hash = hash, .data = NULL};

  /* memset() needed to make hashnode_cmp() work */
  memset(node.key, 0, sizeof(node.key));
  memcpy(node.key, key, map->keysize);

  struct darray* bucket = hashmap_query(map, key, &hash);

  map->last_used = bucket;
  bucket_index = hashmap_bucket_index(map, bucket);

  node_index = darray_index_query(bucket, &node);

  /* If linear probing is not enabled, then we know for sure the element
   * isn't in the hashmap, because it wasn't in the bucket the key hashed to.
   */
  if (node_index == -1) {
    if (!(map->flags & DS_HASHMAP_LINEAR_PROBING)) {
      DBGD("Key not found in bucket %d\n", bucket_index);
      return NULL;
    }
    linear_probe(map, &node, &bucket_index, &node_index);
    RCSW_CHECK(bucket_index != -1);
    RCSW_CHECK(node_index != -1);
    bucket = map->buckets + bucket_index;
  }

  /*
   * This is needed because the alignment for a void* on SPARC is 32 bits,
   * and if your data is not 32-bit aligned, you will get a unaligned address
   * trap. So use a uint8_t* to get the data, then copy to a local variable to
   * get correct alignment.
   */
  darray_index_serve(bucket, &node, (size_t)node_index);

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
    if (!(map->flags & DS_HASHMAP_LINEAR_PROBING)) {
      DBGD("Bucket %zu is full (%zu elements): cannot add new hashnode\n",
           bucket_index,
           bucket->current);
      map->n_addfails++;
      return ERROR;
    }
    /* Loop through all buckets, starting from the one we originally hashed
     * to, incrementing indices, until a suitable bucket is found.
     */
    bucket = NULL;
    for (i = (int)(bucket_index + 1) % (int)map->n_buckets;
         i != (int)bucket_index;
         i++) {
      if (map->buckets[i].current < (size_t)map->buckets[i].max_elts) {
        bucket = map->buckets + i;
        break;
      }
      if (i + 1 == (int)map->n_buckets) {
        i = -1;
      }
    } /* for() */

    if (!bucket) {
      DBGD("All buckets full: Cannot add new hashnode\n");
      map->n_addfails++;
      return ERROR;
    }
    DBGD("Linear probing found bucket %d\n", i);
  } /* if(bucket->current >= bucket->max_elts) */

  void* datablock = datablock_alloc(map);
  RCSW_CHECK_PTR(datablock);
  ds_elt_copy(datablock, data, map->el_size);

  struct hashnode node = {.data = datablock, .hash = hash};
  /* memset() needed to make hashnode_cmp() work */
  memset(node.key, 0, sizeof(node.key));
  memcpy(node.key, key, map->keysize);

  /* RCSW_CHECK for duplicates */
  if (darray_index_query(bucket, &node) != -1) {
    errno = EAGAIN;
    DBGE("ERROR: Node already exists in bucket\n");
    return ERROR;
  }

  SOFT_ASSERT(darray_insert(bucket, &node, bucket->current) == OK,
              "ERROR: could not append node to bucket");
  map->n_collisions +=
      (bucket->current != 1); /* if not 1, wasn't 0 before (COLLISION) */
  map->n_nodes++;
  map->n_adds++;

  /*
   * Sort the hashmap if:
   * -> DS_KEEP_SORTED was not passed (the map is always sorted)
   * -> The sort threshold has been reached
   */
  if (!(map->flags & DS_KEEP_SORTED) && map->sort_thresh != -1 &&
      map->n_adds % (size_t)map->sort_thresh == 0) {
    hashmap_sort(map);
  }
  map->sorted = bucket->sorted;

  return OK;

error:
  map->n_addfails++;
  return ERROR;
} /* hashmap_add() */

status_t hashmap_remove(struct hashmap* const map, const void* const key) {
  RCSW_FPC_NV(ERROR, map != NULL, key != NULL);

  uint32_t hash = 0;
  struct darray* bucket = hashmap_query(map, key, &hash);
  map->last_used = bucket;

  struct hashnode node = {.hash = hash, .data = NULL};
  /* memset() needed to make hashnode_cmp() work */
  memset(node.key, 0, sizeof(node.key));
  memcpy(&node.key, key, map->keysize);

  int node_index = darray_index_query(bucket, &node);
  int bucket_index = hashmap_bucket_index(map, bucket);

  if (node_index == -1) {
    if (!(map->flags & DS_HASHMAP_LINEAR_PROBING)) {
      DBGD("No key found in bucket %d for removal (probing disabled)\n",
           bucket_index);
      goto error; /* normal return */
    }
    linear_probe(map, &node, &bucket_index, &node_index);
    if (bucket_index == -1 || node_index == -1) {
      DBGD("No matching key found in hashmap\n");
      goto error; /* normal return */
    }
    bucket = map->buckets + bucket_index;
  } /* if (node_index == -1) */

  /* deallocate datablock */
  struct hashnode* node_p = darray_data_get(bucket, (size_t)node_index);
  datablock_dealloc(map, node_p->data);

  /* remove hashnode */
  if (darray_remove(bucket, NULL, (size_t)node_index) != OK) {
    DBGE("ERROR: Failed to remove node from bucket");
    errno = EAGAIN;
    return ERROR;
  }

  map->n_nodes--;
  map->sorted = bucket->sorted;
error:
  return OK;
} /* hashmap_remove() */

status_t hashmap_sort(struct hashmap* const map) {
  RCSW_FPC_NV(ERROR, map != NULL);

  size_t i;
  for (i = 0; i < map->n_buckets; i++) {
    darray_sort(&map->buckets[i], QSORT_ITER);
  } /* for() */

  map->sorted = TRUE;
  return OK;
} /* hashmap_sort() */

status_t hashmap_clear(const struct hashmap* const map) {
  RCSW_FPC_NV(ERROR, map != NULL);

  size_t i;
  for (i = 0; i < map->n_buckets; ++i) {
    RCSW_CHECK(darray_clear(map->buckets + i) == OK);
  }
  return OK;

error:
  return ERROR;
} /* hashmap_clear() */

status_t hashmap_gather(const struct hashmap* const map,
                        struct hashmap_stats* const stats) {
  RCSW_FPC_NV(ERROR, map != NULL, stats != NULL);

  stats->n_buckets = map->n_buckets;
  stats->n_nodes = map->n_nodes;
  stats->n_adds = map->n_adds;
  stats->n_addfails = map->n_addfails;
  stats->n_collisions = map->n_collisions;
  stats->collision_ratio = ((double)stats->n_collisions / map->n_adds);
  stats->sorted = map->sorted;

  /* get highest/lowest/average bucket utilization */
  double max = 0;
  double min = 1000000000;
  double average = 0;
  size_t i;

  for (i = 0; i < map->n_buckets; i++) {
    max = (map->buckets[i].current > max) ? map->buckets[i].current : max;
    min = (map->buckets[i].current < min) ? map->buckets[i].current : min;
    average += ((double)map->buckets[i].current) / map->buckets[0].max_elts;
  }

  stats->average_util = average / map->n_buckets;
  stats->max_util = max / map->buckets[0].max_elts;
  stats->min_util = min / map->buckets[0].max_elts;

  return OK;
} /* hashmap_gather() */

void hashmap_print(const struct hashmap* const map) {
  if (map == NULL) {
    DPRINTF("Hashmap: < NULL hashmap >\n");
    return;
  }

  struct hashmap_stats stats;
  RCSW_CHECK(hashmap_gather(map, &stats) == OK);

  DPRINTF("\n******************** Hashmap Print ********************\n");
  DPRINTF("Total buckets   : %zu\n", stats.n_buckets);
  DPRINTF("Total nodes     : %zu\n", stats.n_nodes);
  DPRINTF("Successful adds : %zu\n", stats.n_adds);
  DPRINTF("Failed adds     : %zu\n", stats.n_addfails);
  DPRINTF("Collisions      : %zu\n", stats.n_collisions);
  DPRINTF_FLOAT("Collision ratio : %.8f\n", stats.collision_ratio);
  DPRINTF("Map sorted      : %s\n", (stats.sorted) ? "yes" : "no");

  DPRINTF_FLOAT("Max bucket utilization     : %.8f\n", stats.max_util);
  DPRINTF_FLOAT("Min bucket utilization     : %.8f\n", stats.min_util);
  DPRINTF_FLOAT("Average bucket utilization : %.8f\n", stats.average_util);
  DPRINTF("\n");

error:
  return;
} /* hashmap_print() */

void hashmap_print_distribution(const struct hashmap* const map) {
  if (NULL == map) {
    DPRINTF("Hashmap: < NULL hashmap >\n");
    return;
  }
  size_t i, j, k;
  DPRINTF("\n----------------------------------------\n");
  DPRINTF("Hashmap Utilization Distribution");
  DPRINTF("\n----------------------------------------\n\n");

  /* get maximum bucket node count */
  size_t max_node_count = 0;
  for (i = 0; i < map->n_buckets; ++i) {
    max_node_count = RCSW_MAX(map->buckets[i].current - 1, max_node_count);
  }
  size_t y_scale = (max_node_count > 100) ? max_node_count / 100 : 1;
  size_t chunk_size = 100;

  for (i = 0; i < map->n_buckets; i += chunk_size) {
    /* display graph */
    for (j = 0; j < max_node_count * y_scale; ++j) {
      DPRINTF("%8zu | ", max_node_count - y_scale * j);
      for (k = i; k < i + chunk_size; k++) {
        if (k == map->n_buckets) {
          break;
        }
        if (map->buckets[k].current - 1 >= max_node_count - y_scale * j) {
          DPRINTF("*");
        } else {
          DPRINTF(" ");
        }
      }
      DPRINTF("\n");
    }

    /* display X-axis */
    for (j = 0; j < chunk_size + 13; ++j) {
      if (j == map->n_buckets + 13) {
        break;
      }
      DPRINTF("-");
    }
    DPRINTF("\nBucket # | ");

    /* display X-axis markers */
    for (j = 0; j <= i + chunk_size; ++j) {
      if (j % 10 == 0) {
        DPRINTF("|%-9zu", j);
      }
      if (j == map->n_buckets) {
        break;
      }
    }
    DPRINTF("\n\n");
  }
} /* hashmap_print_distribution() */

/*******************************************************************************
 * Static Functions
 ******************************************************************************/
static int hashnode_cmp(const void* const n1, const void* const n2) {
  return memcmp(((const struct hashnode*)n1)->key,
                ((const struct hashnode*)n2)->key,
                HASHMAP_MAX_KEYSIZE);
} /* hashnode_cmp() */

static void linear_probe(const struct hashmap* const map,
                         const struct hashnode* const node,
                         int* bucket_index,
                         int* node_index) {
  for (int i = (*bucket_index + 1) % (int)map->n_buckets; i != *bucket_index;
       i++) {
    *node_index = darray_index_query(map->buckets + i, node);
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

static void datablock_dealloc(const struct hashmap* const map,
                              const uint8_t* const datablock) {
  if (datablock == NULL) {
    return;
  }
  uint8_t* db_start = map->elements + ds_calc_meta_space(map->max_elts);
  size_t block_index = (size_t)(datablock - db_start) / (map->el_size);
  ((int*)(map->elements))[block_index] = -1; /* mark data block as available */
  DBGV("Dellocated data block %zu/%zu\n", block_index + 1, map->max_elts);
} /* datablock_dealloc() */

static void* datablock_alloc(const struct hashmap* const map) {
  void* datablock = NULL;

  /*
   * Try to find an available data block. Using hashing/linear probing instead
   * of linear scan. This reduces startup times if initializing/building a
   * large hashmap.
   */

  /* make sure that we have 32 bits of randomness */
  uint32_t val =
      (uint32_t)(random() & 0xff) | (uint32_t)((random() & 0xff) << 8) |
      (uint32_t)((random() & 0xff) << 16) | (uint32_t)((random() & 0xff) << 24);

  size_t index = hash_fnv1a(&val, 4) % map->max_elts;

  datablock = ds_meta_probe(map->elements, map->el_size, map->max_elts, &index);
  RCSW_CHECK_PTR(datablock);

  DBGV("Allocated data block %zu/%zu\n", index + 1, map->max_elts);
  ((int*)(map->elements))[index] = 0; /* mark data block as in use */
  return datablock;

error:
  return NULL;
} /* datablock_alloc() */

END_C_DECLS
