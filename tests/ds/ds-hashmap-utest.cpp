/**
 * \file ds-hashmap-utest.cpp
 *
 * \copyright 2017 John Harwell, All rights reserved.
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/er/plugin/log4cl.h"
#define CATCH_CONFIG_PREFIX_ALL
#include <catch2/catch_test_macros.hpp>

#include <vector>

#include "rcsw/ds/hashmap.h"
#include "rcsw/utils/byteops.h"
#include "rcsw/utils/hash.h"
#include "tests/ds/ds_test.hpp"

/*******************************************************************************
 * Test Runner
 ******************************************************************************/
template <typename T>
static void run_test(void (*test)(struct hashmap_config* config)) {
  struct hashmap_config config;
  memset(&config, 0, sizeof(hashmap_config));
  config.hash        = utils_hash_default;
  config.sort_thresh = -1;
  config.elt_size    = sizeof(T);
  CATCH_REQUIRE(th::ds_init(&config) == OK);
  config.sort_thresh = -1;

  uint32_t flags[] = {
    RCSW_ZALLOC,
    RCSW_NOALLOC_HANDLE,
    RCSW_NOALLOC_DATA,
    RCSW_NOALLOC_META,
    RCSW_DS_SORTED,
    RCSW_DS_HASHMAP_LINPROB,
  };

  /* single flags */
  for (size_t i = 0; i < RCSW_ARRAY_ELTS(flags); ++i) {
    for (size_t m = 1; m < TH_NUM_ITEMS; ++m) {
      for (size_t k = 1; k < TH_NUM_BUCKETS; ++k) {
        config.flags     = flags[i];
        config.bsize     = m;
        config.n_buckets = k;
        test(&config);
      }
    }
  }
  /* pairwise */
  for (size_t i = 0; i < RCSW_ARRAY_ELTS(flags); ++i) {
    for (size_t j = i + 1; j < RCSW_ARRAY_ELTS(flags); ++j) {
      for (size_t m = 1; m < TH_NUM_ITEMS; ++m) {
        for (size_t k = 1; k < TH_NUM_BUCKETS; ++k) {
          config.flags     = flags[i] | flags[j];
          config.bsize     = m;
          config.n_buckets = k;
          test(&config);
        }
      }
    }
  }
  th::ds_shutdown(&config);
}

/*******************************************************************************
 * Test Functions
 ******************************************************************************/
template <typename T>
static void build_test(struct hashmap_config* config) {
  struct hashmap  mymap;
  size_t          attempts     = config->n_buckets * config->bsize;
  int             failed_count = 0;

  if (config->flags & RCSW_DS_SORTED) {
    config->sort_thresh = RCSW_MAX(attempts / 2, (size_t)1);
  }

  struct hashmap* map = hashmap_init(&mymap, config);
  CATCH_REQUIRE(nullptr != map);

  /* empty state */
  CATCH_REQUIRE(0 == map->stats.n_nodes);

  std::vector<struct hashnode> nodes(attempts);
  std::vector<T>               data(attempts);

  for (size_t i = 0; i < attempts; i++) {
    utils_string_gen((char*)nodes[i].key, RCSW_HASHMAP_KEYSIZE);
    data[i].value1 = (int)i;
    
    int rval = hashmap_add(map, nodes[i].key, &data[i]);
    if (rval == OK) {
      T* el = (T*)hashmap_data_get(map, nodes[i].key);
      CATCH_REQUIRE(el != nullptr);
      CATCH_REQUIRE(th::cmpe<T>(&data[i], el) == 0);
    } else {
      if (++failed_count > 10) {
        break;
      }
    }
  }
  /* clear: all keys gone */
  hashmap_clear(map);
  for (auto& n : nodes) {
    CATCH_REQUIRE(nullptr == hashmap_data_get(map, n.key));
  }

  hashmap_destroy(map);
  CATCH_REQUIRE(th::leak_check_data(config) == OK);
}

template <typename T>
static void remove_test(struct hashmap_config* config) {
  struct hashmap  mymap;
  size_t          len          = config->n_buckets * config->bsize;
  int             failed_count = 0;

  struct hashmap* map = hashmap_init(&mymap, config);
  CATCH_REQUIRE(map != nullptr);
  std::vector<struct hashnode> nodes(len);
  std::vector<T>               data(len);
  size_t                       n_inserted = 0;

  for (size_t i = 0; i < len; i++) {
    utils_string_gen((char*)nodes[i].key, RCSW_HASHMAP_KEYSIZE);
    data[i].value1 = rand() % ((int)i + 1);
    if (hashmap_add(map, nodes[i].key, &data[i]) == OK) {
      n_inserted++;
    } else if (++failed_count > 10) {
      break;
    }
  }

  /* remove each successfully-inserted element */
  for (size_t j = 0; j < n_inserted; j++) {
    if (hashmap_data_get(map, nodes[j].key)) {
      unsigned old_size = map->stats.n_nodes;
      CATCH_REQUIRE(hashmap_remove(map, nodes[j].key) == OK);

      /* key is gone */
      CATCH_REQUIRE(hashmap_data_get(map, nodes[j].key) == nullptr);
      CATCH_REQUIRE(map->stats.n_nodes == old_size - 1);

      /* second removal still returns OK */
      CATCH_REQUIRE(hashmap_remove(map, nodes[j].key) == OK);
    }
  }

  hashmap_destroy(map);
  CATCH_REQUIRE(th::leak_check_data(config) == OK);
}

template <typename T>
static void map_inject_test(struct hashmap_config* config) {
  struct hashmap  mymap;
  struct hashmap* map = hashmap_init(&mymap, config);
  CATCH_REQUIRE(nullptr != map);

  /* NULL callback rejected */
  CATCH_REQUIRE(ERROR == hashmap_map(map, nullptr));
  int dummy = 0;
  CATCH_REQUIRE(ERROR == hashmap_inject(map, nullptr, &dummy));

  /* insert some elements */
  size_t n = RCSW_MIN(config->n_buckets * config->bsize, (size_t)10);
  std::vector<struct hashnode> nodes(n);
  std::vector<T>               data(n);
  size_t                       n_ok = 0;
  for (size_t i = 0; i < n; i++) {
    utils_string_gen((char*)nodes[i].key, RCSW_HASHMAP_KEYSIZE);
    data[i].value1 = (int)i;
    if (hashmap_add(map, nodes[i].key, &data[i]) == OK) {
      n_ok++;
    }
  }

  /* map: decrement all values */
  CATCH_REQUIRE(OK == hashmap_map(map, th::map_func<T>));

  /* inject: sum all values */
  int total = 0;
  CATCH_REQUIRE(OK == hashmap_inject(map, th::inject_func<T>, &total));
  /* just verify it ran without crashing and total is reasonable */
  (void)total;

  hashmap_destroy(map);
}

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("hashmap Build Test", "[ds][hashmap]") {
  run_test<element8>(build_test<element8>);
  run_test<element4>(build_test<element4>);
  run_test<element2>(build_test<element2>);
  run_test<element1>(build_test<element1>);
}
CATCH_TEST_CASE("hashmap Remove Test", "[ds][hashmap]") {
  run_test<element8>(remove_test<element8>);
  run_test<element4>(remove_test<element4>);
  run_test<element2>(remove_test<element2>);
  run_test<element1>(remove_test<element1>);
}
CATCH_TEST_CASE("hashmap Map/Inject Test", "[ds][hashmap]") {
  run_test<element8>(map_inject_test<element8>);
  run_test<element4>(map_inject_test<element4>);
}
CATCH_TEST_CASE("hashmap Print Test", "[ds][hashmap]") {
  /* Coverage: verify print functions don't crash */
  struct hashmap_config config;
  memset(&config, 0, sizeof(hashmap_config));
  config.hash        = utils_hash_default;
  config.sort_thresh = -1;
  config.elt_size    = sizeof(element4);
  config.bsize       = TH_NUM_ITEMS;
  config.n_buckets   = TH_NUM_BUCKETS;
  config.flags       = RCSW_NONE;
  CATCH_REQUIRE(th::ds_init(&config) == OK);

  hashmap_print(nullptr);
  struct hashmap  mymap;
  struct hashmap* map = hashmap_init(&mymap, &config);
  CATCH_REQUIRE(map != nullptr);
  hashmap_print(map);
  hashmap_print_dist(map);
  hashmap_destroy(map);
  th::ds_shutdown(&config);
}
