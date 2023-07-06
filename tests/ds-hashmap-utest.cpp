/**
 * \file hashmap-test.cpp
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_PREFIX_ALL
#include <catch.hpp>

#include "rcsw/ds/hashmap.h"
#include "rcsw/utils/hash.h"
#include "rcsw/utils/utils.h"
#include "tests/ds_test.h"
#include "tests/ds_test.hpp"

/*******************************************************************************
 * Test Helpers
 ******************************************************************************/
template<typename T>
static void run_test(hashmap_test_t test) {
  /* dbg_init(); */
  /* dbg_insmod(M_TESTING,"Testing"); */
  /* dbg_insmod(M_DS_RBUFFER,"RBuffer"); */

  struct hashmap_params params;
  params.flags = 0;
  params.cmpe = th_cmpe<T>;
  params.hash = hash_default;
  params.sort_thresh = -1;
  params.keysize = RCSW_HASHMAP_MAX_KEYSIZE;
  params.printe = th_printe<T>;
  params.elt_size = sizeof(T);
  CATCH_REQUIRE(th_ds_init(&params) == OK);

  uint32_t flags[] = {
    RCSW_NOALLOC_HANDLE,
    RCSW_NOALLOC_DATA,
    RCSW_NOALLOC_META,
    RCSW_DS_SORTED,
    RCSW_DS_HASHMAP_LINPROB
  };

  for (size_t i = 0; i < RCSW_ARRAY_ELTS(flags); ++i) {
    for (size_t j = 1; j < TH_NUM_ITEMS; ++j) {
      for (size_t k = 1; k < TH_NUM_BUCKETS; ++k) {
        params.flags = flags[i];
        params.bsize = j;
        params.n_buckets = k;
        test(&params);
      } /* for(k..) */
    } /* for(j..) */
  } /* for(i..) */
  th_ds_shutdown(&params);
} /* run_test() */

/*******************************************************************************
 * Test Functions
 ******************************************************************************/
template<typename T>
static void build_test(struct hashmap_params *  params) {
  struct hashmap *map;
  struct hashmap mymap;
  size_t i, j;
  int failed_count = 0;
  size_t attempts = params->n_buckets * params->bsize;

  struct hashnode nodes[TH_NUM_ITEMS* TH_NUM_ITEMS];
  T data[TH_NUM_ITEMS * TH_NUM_ITEMS];

  if (params->flags & RCSW_DS_SORTED) {
    params->sort_thresh = RCSW_MAX(attempts / 2, 1UL);
  }

  map = hashmap_init(&mymap, params);
  CATCH_REQUIRE(nullptr != map);

  /* attempt to fill hashmap */
  for (i = 0; i < attempts; i++) {
    char rand_key[RCSW_HASHMAP_MAX_KEYSIZE];
    string_gen(rand_key, RCSW_HASHMAP_MAX_KEYSIZE);
    data[i].value1 = i;
    memcpy(nodes[i].key, rand_key, RCSW_HASHMAP_MAX_KEYSIZE);

    int rval = hashmap_add(map, nodes[i].key, data+i);

    /* verify new element in hashmap, if the insertion succeeded */
    if (rval == OK) {
      T * el = (T*)hashmap_data_get(map, nodes[i].key);
      CATCH_REQUIRE(el != nullptr);
      CATCH_REQUIRE(th_cmpe<T>(data+i, el) == 0);


    } else {
      failed_count++;
      /*
       * We cannot usually fill the hashmap, unless linear probing is enabled,
       * because buckets get full, so bail out after a couple failures to add
       */
      if (failed_count > 10) {
        break;
      }
    }
  } /* for() */

  /* verify all elements */
  for (j = 0; j < i; ++j) {
    T * el = (T*)hashmap_data_get(map, nodes[j].key);
    if (el != nullptr) {
      CATCH_REQUIRE(th_cmpe<T>(data+j, el) == OK);
    }
  } /* for() */
  hashmap_clear(map);
  for (j = 0; j < i; ++j) {
    CATCH_REQUIRE(nullptr == hashmap_data_get(map, nodes[j].key));
  } /* for() */
  hashmap_destroy(map);
} /* build_test() */

template<typename T>
static void stats_test(struct hashmap_params *  params) {
  struct hashmap *map;
  struct hashmap mymap;
  size_t i, j;
  int failed_count = 0;

  params->bsize = TH_NUM_ITEMS * TH_NUM_ITEMS;

  map = hashmap_init(&mymap, params);
  CATCH_REQUIRE(nullptr != map);

  /* fill hashmap */
  element_generator<T> g(gen_elt_type::ekINC_VALS, TH_NUM_ITEMS*10);
  for (i = 0; i < TH_NUM_ITEMS * 10; ++i) {
    char rand_key[RCSW_HASHMAP_MAX_KEYSIZE];
    string_gen(rand_key, RCSW_HASHMAP_MAX_KEYSIZE);
    T e = g.next();
    struct hashnode n;
    memcpy(n.key, rand_key, RCSW_HASHMAP_MAX_KEYSIZE);

    /* we don't care how many things are in the hashmap */
    hashmap_add(map, n.key, &e);
  }

  hashmap_destroy(map);
} /* stats_test() */

template<typename T>
static void linear_probing_test(struct hashmap_params *  params) {
  struct hashmap *map;
  struct hashmap mymap;
  int len = params->n_buckets * params->bsize;
  struct hashnode nodes[TH_NUM_ITEMS * TH_NUM_ITEMS];
  T data[TH_NUM_ITEMS * TH_NUM_ITEMS];

  params->flags |= RCSW_DS_HASHMAP_LINPROB;
  map = hashmap_init(&mymap, params);
  CATCH_REQUIRE(map);

  /* fill hashmap */
  for (int i = 0; i < len; i++) {
    char rand_key[RCSW_HASHMAP_MAX_KEYSIZE];
    string_gen(rand_key, RCSW_HASHMAP_MAX_KEYSIZE);
    data[i].value1 = rand() % (i+1);
    memcpy(nodes[i].key, rand_key, RCSW_HASHMAP_MAX_KEYSIZE);

    CATCH_REQUIRE(hashmap_add(map, nodes[i].key, data+i) == OK);

    /* verify new element in hashmap */
    T * el = (T*)hashmap_data_get(map, nodes[i].key);
    CATCH_REQUIRE(el != nullptr);
    CATCH_REQUIRE(th_cmpe<T>(data+i, el) == 0);
  } /* for() */

  /* verify all elements */
  for (int i = 0; i < len; ++i) {
    T * el = (T*)hashmap_data_get(map, nodes[i].key);

    if (el != nullptr) {
      CATCH_REQUIRE(th_cmpe<T>(data+i, el) == OK);
    }
  } /* for() */
  hashmap_destroy(map);
} /* linear_probing_test() */

template<typename T>
static void remove_test(struct hashmap_params * params) {
  struct hashmap *map;
  struct hashmap mymap;
  int i, j;
  int failed_count = 0;
  int len = params->n_buckets * params->bsize;
  struct hashnode nodes[TH_NUM_ITEMS * TH_NUM_ITEMS];
  T data[TH_NUM_ITEMS * TH_NUM_ITEMS];

  map = hashmap_init(&mymap, params);
  CATCH_REQUIRE(map);

  /* attempt to fill hashmap */
  for (i = 0; i < len; i++) {
    char rand_key[RCSW_HASHMAP_MAX_KEYSIZE];
    string_gen(rand_key, RCSW_HASHMAP_MAX_KEYSIZE);
    data[i].value1 = rand() % (i+1);

    memcpy(nodes[i].key, rand_key, RCSW_HASHMAP_MAX_KEYSIZE);

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
      old_size = map->stats.n_nodes;
      CATCH_REQUIRE(hashmap_remove(map, nodes[j].key) == OK);

      /* verify key was removed */
      CATCH_REQUIRE(hashmap_data_get(map, nodes[j].key) == nullptr);
      CATCH_REQUIRE(map->stats.n_nodes == old_size -1);

      /* verify 2nd removal fails */
      CATCH_REQUIRE(hashmap_remove(map, nodes[j].key) == OK);
    }
  } /* for() */

  hashmap_destroy(map);
  CATCH_REQUIRE(th_leak_check_data(params) == OK);
} /* remove_test() */

template<typename T>
static void print_test(struct hashmap_params *  params) {
  struct hashmap *map;
  struct hashmap mymap;
  size_t i, j;
  int failed_count = 0;

  params->bsize = TH_NUM_ITEMS * TH_NUM_ITEMS;

  hashmap_print(nullptr);
  map = hashmap_init(&mymap, params);

  CATCH_REQUIRE(nullptr != map);
  hashmap_print(map);
  hashmap_print_dist(map);

  /* fill hashmap */
  element_generator<T> g(gen_elt_type::ekINC_VALS, TH_NUM_ITEMS*10);
  for (i = 0; i < TH_NUM_ITEMS * 10; ++i) {
    char rand_key[RCSW_HASHMAP_MAX_KEYSIZE];
    string_gen(rand_key, RCSW_HASHMAP_MAX_KEYSIZE);
    T e = g.next();
    struct hashnode n;
    memcpy(n.key, rand_key, RCSW_HASHMAP_MAX_KEYSIZE);

    /* we don't care how many things are in the hashmap */
    hashmap_add(map, n.key, &e);
  }

  hashmap_print(map);
  hashmap_print_dist(map);
  hashmap_destroy(map);
} /* print_test() */

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("Build Test", "[ds][hashmap]") {
  run_test<element8>(build_test<element8>);
  run_test<element4>(build_test<element4>);
  run_test<element2>(build_test<element2>);
  run_test<element1>(build_test<element1>);
}
CATCH_TEST_CASE("Linear Probe Test", "[ds][hashmap]") {
  run_test<element8>(linear_probing_test<element8>);
  run_test<element4>(linear_probing_test<element4>);
  run_test<element2>(linear_probing_test<element2>);
  run_test<element1>(linear_probing_test<element1>);
}
CATCH_TEST_CASE("Remove Test", "[ds][hashmap]") {
  run_test<element8>(remove_test<element8>);
  run_test<element4>(remove_test<element4>);
  run_test<element2>(remove_test<element2>);
  run_test<element1>(remove_test<element1>);
}
CATCH_TEST_CASE("Stats Test", "[ds][hashmap]") {
  run_test<element8>(stats_test<element8>);
  run_test<element4>(stats_test<element4>);
  run_test<element2>(stats_test<element2>);
  run_test<element1>(stats_test<element1>);
}

CATCH_TEST_CASE("Print Test", "[ds][hashmap]") {
  run_test<element8>(print_test<element8>);
}
