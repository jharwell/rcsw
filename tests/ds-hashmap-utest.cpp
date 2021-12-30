/**
 * @file hashmap-test.cpp
 *
 * Test of hashmap module
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
#include <stdio.h>
#include <stdlib.h>

extern "C" {
#include "rcsw/ds/hashmap.h"
#include "rcsw/utils/hash.h"
#include "rcsw/utils/utils.h"
#include "tests/ds_test.h"
}

#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_PREFIX_ALL
#include <catch.hpp>

/*******************************************************************************
 * Constant Definitions
 ******************************************************************************/
#define NUM_TESTS       3

/*******************************************************************************
 * Test Function Forward Declarations
 ******************************************************************************/
static void build_test( struct ds_params *  params);
static void linear_probing_test( struct ds_params *  params);
static void remove_test( struct ds_params * params);

/*******************************************************************************
 * Test Helpers
 ******************************************************************************/
static void test_runner(void (*test)(struct ds_params *params)) {
  /* dbg_init(); */
  /* dbg_insmod(M_TESTING,"Testing"); */
  /* dbg_insmod(M_DS_RBUFFER,"RBuffer"); */

  struct ds_params params;
  params.tag = DS_HASHMAP;
  params.flags = 0;
  params.cmpe = th_key_cmp;
  params.type.hm.key_cmp = th_key_cmp;
  params.type.hm.hash = hash_default;
  params.type.hm.sort_thresh = -1;
  params.type.hm.keysize = HASHMAP_MAX_KEYSIZE;
  params.printe = th_printe;
  params.el_size = sizeof(struct element);
  CATCH_REQUIRE(th_ds_init(&params) == OK);

  /* test with defined sizes */
  for (size_t j = 1; j < NUM_ITEMS; ++j) {
    for (size_t k = 1; k < NUM_ITEMS; ++k) {
      params.type.hm.bsize = j;
      params.type.hm.n_buckets = k;
      test(&params);
    } /* for(k..) */
  } /* for(j..) */
  th_ds_shutdown(&params);
} /* test_runner() */

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("hashmap Build Test", "[hashmap]") { test_runner(build_test); }
CATCH_TEST_CASE("hashmap Linear Probe Test", "[hashmap]") { test_runner(linear_probing_test); }
CATCH_TEST_CASE("hashmap Remove Test", "[hashmap]") { test_runner(remove_test); }

static void build_test(struct ds_params *  params) {
  struct hashmap *map;
  struct hashmap mymap;
  int i, j;
  int failed_count = 0;
  int len = params->type.hm.n_buckets * params->type.hm.bsize;
  struct hashnode nodes[len];
  struct element data[len];

  map = hashmap_init(&mymap, params);
  CATCH_REQUIRE(NULL != map);

  /* attempt to fill hashmap */
  for (i = 0; i < len; i++) {
    char rand_key[HASHMAP_MAX_KEYSIZE];
    string_gen(rand_key, HASHMAP_MAX_KEYSIZE);
    data[i].value1 = rand() % (i+1);
    data[i].value2 = rand() % (i+2);
    memcpy(nodes[i].key, rand_key, HASHMAP_MAX_KEYSIZE);

    int rval = hashmap_add(map, nodes[i].key, data+i);
    if (OK != rval) {
      failed_count++;
    }
    /* we cannot usually fill the hashmap, unless linear probing is enabled,
     * because buckets get full, so bail out after a couple failures to add
     */
    if (failed_count > 10) {
      break;
    }
    /* verify new element in hashmap, if the insertion succeeded */
    if (rval == OK) {
      struct element * el = (element*)hashmap_data_get(map, nodes[i].key);
      CATCH_REQUIRE(el != NULL);
      CATCH_REQUIRE(th_cmpe(data+i, el) == 0);
    }
  } /* for() */

  /* verify all elements */
  for (j = 0; j < i; ++j) {
    struct element * el = (element*)hashmap_data_get(map, nodes[j].key);
    if (el != NULL) {
      CATCH_REQUIRE(th_cmpe(data+j, el) == OK);
    }
  } /* for() */
  hashmap_destroy(map);
} /* build_test() */

static void linear_probing_test(struct ds_params *  params) {
  struct hashmap *map;
  struct hashmap mymap;
  int i;
  int len = params->type.hm.n_buckets * params->type.hm.bsize;
  struct hashnode nodes[len];
  struct element data[len];

  params->flags |= DS_HASHMAP_LINEAR_PROBING;
  map = hashmap_init(&mymap, params);
  CATCH_REQUIRE(map);

  /* fill hashmap */
  for (i = 0; i < len; i++) {
    char rand_key[HASHMAP_MAX_KEYSIZE];
    string_gen(rand_key, HASHMAP_MAX_KEYSIZE);
    data[i].value1 = rand() % (i+1);
    data[i].value2 = rand() % (i+2);
    memcpy(nodes[i].key, rand_key, HASHMAP_MAX_KEYSIZE);

    CATCH_REQUIRE(hashmap_add(map, nodes[i].key, data+i) == OK);

    /* verify new element in hashmap */
    struct element * el = (element*)hashmap_data_get(map, nodes[i].key);
    CATCH_REQUIRE(el != NULL);
    CATCH_REQUIRE(th_cmpe(data+i, el) == 0);
  } /* for() */

  /* verify all elements */
  for (i = 0; i < len; ++i) {
    struct element * el = (element*)hashmap_data_get(map, nodes[i].key);

    if (el != NULL) {
      CATCH_REQUIRE(th_cmpe(data+i, el) == OK);
    }
  } /* for() */
  hashmap_destroy(map);
} /* linear_probing_test() */

static void remove_test(struct ds_params * params) {
  struct hashmap *map;
  struct hashmap mymap;
  int i, j;
  int failed_count = 0;
  int len = params->type.hm.n_buckets * params->type.hm.bsize;
  struct hashnode nodes[len];
  struct element data[len];

  map = hashmap_init(&mymap, params);
  CATCH_REQUIRE(map);

  /* attempt to fill hashmap */
  for (i = 0; i < len; i++) {
    char rand_key[HASHMAP_MAX_KEYSIZE];
    string_gen(rand_key, HASHMAP_MAX_KEYSIZE);
    data[i].value1 = rand() % (i+1);
    data[i].value2 = rand() % (i+2);
    memcpy(nodes[i].key, rand_key, HASHMAP_MAX_KEYSIZE);

    int rval = hashmap_add(map, nodes[i].key, data+i);
    if (OK != rval) {
      failed_count++;
    }

    /* We cannot usually fill the hashmap, unless linear probing is enabled,
     * because buckets get full, so bail out after a couple failures to add
     */
    if (failed_count > 10) {
      break;
    }
  } /* for() */

  unsigned old_size;
  /* remove elements */
  for (j = 0; j < i; j++) {
    if (hashmap_data_get(map, nodes[j].key)) {
      old_size = map->n_nodes;
      CATCH_REQUIRE(hashmap_remove(map, nodes[j].key) == OK);

      /* verify key was removed */
      CATCH_REQUIRE(hashmap_data_get(map, nodes[j].key) == NULL);
      CATCH_REQUIRE(map->n_nodes == old_size -1);
    }
  } /* for() */

  hashmap_destroy(map);
  CATCH_REQUIRE(th_leak_check_data(params) == OK);
} /* remove_test() */
