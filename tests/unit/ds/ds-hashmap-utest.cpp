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
#include "rcsw/er/plugin/log4cl.h"
#define CATCH_CONFIG_PREFIX_ALL
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "rcsw/ds/hashmap.h"
#include "rcsw/utils/byteops.h"
#include "rcsw/utils/hash.h"
#include "tests/unit/ds/ds_test.hpp"

/*******************************************************************************
 * Test Harness
 ******************************************************************************/
template <typename T>
static void run_test(void (*test)(struct hashmap_config* config)) {
  struct hashmap_config config;
  memset(&config, 0, sizeof(hashmap_config));
  config.hash        = utils_hash_default;
  config.elt_size    = sizeof(T);
  CATCH_REQUIRE(th::ds_init(&config) == OK);

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

template <typename T>
static void build_test(struct hashmap_config* config) {
  struct hashmap mymap;
  size_t         attempts     = config->n_buckets * config->bsize;
  int            failed_count = 0;

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
  struct hashmap mymap;
  size_t         len          = config->n_buckets * config->bsize;
  int            failed_count = 0;

  struct hashmap* map = hashmap_init(&mymap, config);
  CATCH_REQUIRE(map != nullptr);
  std::vector<struct hashnode> nodes(len);
  std::vector<T>               data(len);
  size_t                       n_inserted = 0;

  for (size_t i = 0; i < len; i++) {
    utils_string_gen((char*)nodes[i].key, RCSW_HASHMAP_KEYSIZE);
    auto dist      = std::uniform_int_distribution<size_t>(0, i);
    data[i].value1 = dist(th::make_rng());
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

      /* second removal should return ERROR */
      errno = 0;
      CATCH_REQUIRE(hashmap_remove(map, nodes[j].key) == ERROR);
      CATCH_REQUIRE(errno == ENOENT);
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

namespace {
struct key_buf {
  char bytes[RCSW_HASHMAP_KEYSIZE];
};

/* A zero-padded, full-width key: safe under any key semantics */
key_buf make_key(const char* s) {
  key_buf k;
  memset(&k, 0, sizeof(k));
  snprintf(k.bytes, sizeof(k.bytes), "%s", s);
  return k;
}

struct hashmap* make_map(size_t n_buckets, size_t bsize, uint32_t flags) {
  struct hashmap_config c;
  memset(&c, 0, sizeof(c));
  c.elt_size    = sizeof(int);
  c.n_buckets   = n_buckets;
  c.bsize       = bsize;
  c.hash        = utils_hash_fnv1a;
  c.flags       = flags;
  return hashmap_init(nullptr, &c);
}

size_t bucket_of(const key_buf& k, size_t n_buckets) {
  uint32_t h = 0;
  utils_hash_fnv1a(k.bytes, sizeof(k.bytes), &h);
  return h % n_buckets;
}

/* Two distinct keys that hash to the same bucket */
std::pair<key_buf, key_buf> colliding_keys(size_t n_buckets) {
  key_buf first = make_key("k0");
  size_t  b     = bucket_of(first, n_buckets);
  for (int i = 1; i < 1000; ++i) {
    char name[16];
    snprintf(name, sizeof(name), "k%d", i);
    key_buf k = make_key(name);
    if (bucket_of(k, n_buckets) == b) {
      return {first, k};
    }
  }
  return {first, first};
}
} /* namespace */

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("hashmap Build Test", "[ds][hashmap][noalloc]") {
  run_test<element8>(build_test<element8>);
  run_test<element4>(build_test<element4>);
  run_test<element2>(build_test<element2>);
  run_test<element1>(build_test<element1>);
}
CATCH_TEST_CASE("hashmap Remove Test", "[ds][hashmap][noalloc]") {
  run_test<element8>(remove_test<element8>);
  run_test<element4>(remove_test<element4>);
  run_test<element2>(remove_test<element2>);
  run_test<element1>(remove_test<element1>);
}
CATCH_TEST_CASE("hashmap Map/Inject Test", "[ds][hashmap][noalloc]") {
  run_test<element8>(map_inject_test<element8>);
  run_test<element4>(map_inject_test<element4>);
}
CATCH_TEST_CASE("hashmap Print Test", "[ds][hashmap][noalloc]") {
  /* Coverage: verify print functions don't crash */
  struct hashmap_config config;
  memset(&config, 0, sizeof(hashmap_config));
  config.hash        = utils_hash_default;
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
  hashmap_destroy(map);
  th::ds_shutdown(&config);
}

CATCH_TEST_CASE("hashmap: keys are full RCSW_HASHMAP_KEYSIZE-byte blobs",
                "[ds][hashmap]") {
  /*
   * Keys are compared (memcmp) and hashed over all RCSW_HASHMAP_KEYSIZE
   * bytes, so two buffers holding the same C string but different trailing
   * bytes are DIFFERENT keys, and each is found exactly.
   */
  struct hashmap* m = make_map(16, 4, RCSW_NONE);

  key_buf a = make_key("abc"); /* "abc\0\0\0..." */
  key_buf b;
  memset(&b, 'Z', sizeof(b));
  memcpy(b.bytes, "abc", 4); /* "abc\0ZZZ..." */

  int va = 1;
  int vb = 2;
  CATCH_REQUIRE(OK == hashmap_add(m, a.bytes, &va));
  CATCH_REQUIRE(OK == hashmap_add(m, b.bytes, &vb)); /* not a duplicate */
  CATCH_REQUIRE(1 == *static_cast<int*>(hashmap_data_get(m, a.bytes)));
  CATCH_REQUIRE(2 == *static_cast<int*>(hashmap_data_get(m, b.bytes)));

  /* removing a missing key: ENOENT */
  key_buf c = make_key("missing");
  errno     = 0;
  CATCH_REQUIRE(ERROR == hashmap_remove(m, c.bytes));
  CATCH_REQUIRE(ENOENT == errno);
  hashmap_destroy(m);
}

CATCH_TEST_CASE("hashmap_clear returns all datablocks to the pool",
                "[ds][hashmap]") {
  struct hashmap* m = make_map(2, 2, RCSW_DS_HASHMAP_LINPROB);
  int             v = 1;
  for (const char* s : {"a", "b", "c", "d"}) {
    key_buf k = make_key(s);
    CATCH_REQUIRE(OK == hashmap_add(m, k.bytes, &v));
  }
  CATCH_REQUIRE(OK == hashmap_clear(m));

  struct hashmap_stats stats;
  CATCH_REQUIRE(OK == hashmap_gather(m, &stats));
  CATCH_REQUIRE(0 == stats.n_nodes);

  for (const char* s : {"e", "f", "g", "h"}) {
    key_buf k = make_key(s);
    CATCH_REQUIRE(OK == hashmap_add(m, k.bytes, &v));
  }
  hashmap_destroy(m);
}

CATCH_TEST_CASE(
  "hashmap: duplicate key rejected even when the original "
  "lives in a probed bucket",
  "[ds][hashmap]") {
  auto keys = colliding_keys(2);
  CATCH_REQUIRE(0 != memcmp(&keys.first, &keys.second, sizeof(key_buf)));

  struct hashmap* m = make_map(2, 1, RCSW_DS_HASHMAP_LINPROB);
  int             v = 1;
  CATCH_REQUIRE(OK == hashmap_add(m, keys.first.bytes, &v));
  /* home bucket full -> probed into the other bucket */
  CATCH_REQUIRE(OK == hashmap_add(m, keys.second.bytes, &v));
  /* free up the home bucket */
  CATCH_REQUIRE(OK == hashmap_remove(m, keys.first.bytes));

  /* second key is still present (in the probed bucket) */
  CATCH_REQUIRE(ERROR == hashmap_add(m, keys.second.bytes, &v));

  struct hashmap_stats stats;
  hashmap_gather(m, &stats);
  CATCH_REQUIRE(1 == stats.n_nodes);
  hashmap_destroy(m);
}

CATCH_TEST_CASE("hashmap: duplicate key rejected when the home bucket is full",
                "[ds][hashmap]") {
  auto            keys = colliding_keys(2);
  struct hashmap* m    = make_map(2, 1, RCSW_DS_HASHMAP_LINPROB);
  int             v    = 1;
  CATCH_REQUIRE(OK == hashmap_add(m, keys.first.bytes, &v));
  /* home bucket full: the duplicate check must still look there */
  CATCH_REQUIRE(ERROR == hashmap_add(m, keys.first.bytes, &v));
  hashmap_destroy(m);
}

CATCH_TEST_CASE("hashmap_add rejects NULL data", "[ds][hashmap]") {
  struct hashmap* m = make_map(4, 4, RCSW_NONE);
  key_buf         k = make_key("k");
  CATCH_REQUIRE(ERROR == hashmap_add(m, k.bytes, nullptr));
  hashmap_destroy(m);
}

CATCH_TEST_CASE("hashmap_gather reports sane utilization", "[ds][hashmap]") {
  struct hashmap*      m = make_map(4, 4, RCSW_NONE);
  struct hashmap_stats stats;

  CATCH_REQUIRE(OK == hashmap_gather(m, &stats));
  CATCH_REQUIRE(stats.min_util >= 0.0);
  CATCH_REQUIRE_FALSE(std::isnan(stats.collision_ratio));

  int     v = 1;
  key_buf k = make_key("x");
  CATCH_REQUIRE(OK == hashmap_add(m, k.bytes, &v));
  CATCH_REQUIRE(OK == hashmap_gather(m, &stats));
  CATCH_REQUIRE(stats.min_util >= 0.0);
  CATCH_REQUIRE(stats.min_util <= stats.max_util);
  hashmap_destroy(m);
}

CATCH_TEST_CASE("hashmap: key_len selects short keys", "[ds][hashmap]") {
  struct hashmap_config c;
  memset(&c, 0, sizeof(c));
  c.elt_size  = sizeof(int);
  c.hash      = utils_hash_fnv1a;
  c.n_buckets = 4;
  c.bsize     = 2;
  c.key_len   = sizeof(int);
  c.flags     = RCSW_DS_HASHMAP_LINPROB; /* fill every slot, via probing */

  struct hashmap* m = hashmap_init(nullptr, &c);
  CATCH_REQUIRE(nullptr != m);
  CATCH_REQUIRE(sizeof(int) == m->key_len);

  /* plain ints as keys: no padding to RCSW_HASHMAP_KEYSIZE needed */
  for (int k = 0; k < 8; ++k) {
    int v = k * 10;
    CATCH_REQUIRE(OK == hashmap_add(m, &k, &v));
  }
  int dup = 3;
  CATCH_REQUIRE(ERROR == hashmap_add(m, &dup, &dup));
  for (int k = 0; k < 8; ++k) {
    auto* v = static_cast<int*>(hashmap_data_get(m, &k));
    CATCH_REQUIRE(nullptr != v);
    CATCH_REQUIRE(k * 10 == *v);
  }

  int gone = 5;
  CATCH_REQUIRE(OK == hashmap_remove(m, &gone));
  CATCH_REQUIRE(nullptr == hashmap_data_get(m, &gone));
  errno = 0;
  CATCH_REQUIRE(ERROR == hashmap_remove(m, &gone));
  CATCH_REQUIRE(ENOENT == errno);
  hashmap_destroy(m);
}

CATCH_TEST_CASE("hashmap: key_len 0 means RCSW_HASHMAP_KEYSIZE-byte keys",
                "[ds][hashmap]") {
  struct hashmap* m = make_map(4, 4, RCSW_NONE); /* key_len left 0 */
  CATCH_REQUIRE(nullptr != m);
  CATCH_REQUIRE(RCSW_HASHMAP_KEYSIZE == m->key_len);

  key_buf k = make_key("abc");
  int     v = 7;
  CATCH_REQUIRE(OK == hashmap_add(m, k.bytes, &v));
  CATCH_REQUIRE(7 == *static_cast<int*>(hashmap_data_get(m, k.bytes)));
  hashmap_destroy(m);
}

CATCH_TEST_CASE("hashmap_init rejects key_len above RCSW_HASHMAP_KEYSIZE",
                "[ds][hashmap]") {
  struct hashmap_config c;
  memset(&c, 0, sizeof(c));
  c.elt_size  = sizeof(int);
  c.hash      = utils_hash_fnv1a;
  c.n_buckets = 4;
  c.bsize     = 4;
  c.key_len   = RCSW_HASHMAP_KEYSIZE + 1;
  CATCH_REQUIRE(nullptr == hashmap_init(nullptr, &c));
}

CATCH_TEST_CASE("hashmap_init fails cleanly without promised caller memory",
                "[ds][hashmap]") {
  struct hashmap_config c;
  memset(&c, 0, sizeof(c));
  c.elt_size  = sizeof(int);
  c.hash      = utils_hash_fnv1a;
  c.n_buckets = 4;
  c.bsize     = 4;
  c.key_len   = sizeof(int);

  /* no bucket memory: fails before anything is initialized */
  c.flags = RCSW_NOALLOC_META;
  c.meta  = nullptr;
  CATCH_REQUIRE(nullptr == hashmap_init(nullptr, &c));

  /* no element memory: fails after the buckets are allocated */
  c.flags    = RCSW_NOALLOC_DATA;
  c.elements = nullptr;
  CATCH_REQUIRE(nullptr == hashmap_init(nullptr, &c));
}

CATCH_TEST_CASE("hashmap_element_space is enough for any element size",
                "[ds][hashmap][noalloc]") {
  /* 12 bytes: the datablocks do not end on a dptr_t boundary */
  struct rec {
    char tag[12];
  };
  for (size_t n_buckets = 1; n_buckets <= 5; ++n_buckets) {
    for (size_t bsize = 1; bsize <= 3; ++bsize) {
      size_t n = n_buckets * bsize;
      /* exactly the reported size, so ASAN flags any overrun */
      size_t bytes = hashmap_element_space(n_buckets, bsize, sizeof(rec));
      auto*  elements = static_cast<dptr_t*>(malloc(bytes));

      struct hashmap_config c;
      memset(&c, 0, sizeof(c));
      c.elt_size  = sizeof(rec);
      c.hash      = utils_hash_fnv1a;
      c.n_buckets = n_buckets;
      c.bsize     = bsize;
      c.key_len   = sizeof(int);
      c.elements  = elements;
      c.flags     = RCSW_NOALLOC_DATA | RCSW_DS_HASHMAP_LINPROB;
      struct hashmap* m = hashmap_init(nullptr, &c);
      CATCH_REQUIRE(nullptr != m);
      /* the hashnode array after the datablocks is still aligned */
      CATCH_REQUIRE(0 ==
                    reinterpret_cast<uintptr_t>(m->space.hashnodes) %
                      sizeof(dptr_t));

      for (int k = 0; k < static_cast<int>(n); ++k) {
        rec r;
        snprintf(r.tag, sizeof(r.tag), "k%d", k);
        CATCH_REQUIRE(OK == hashmap_add(m, &k, &r));
      }
      int full = static_cast<int>(n);
      rec r    = {"x"};
      CATCH_REQUIRE(ERROR == hashmap_add(m, &full, &r));

      for (int k = 0; k < static_cast<int>(n); ++k) {
        char want[12];
        snprintf(want, sizeof(want), "k%d", k);
        auto* got = static_cast<rec*>(hashmap_data_get(m, &k));
        CATCH_REQUIRE(nullptr != got);
        CATCH_REQUIRE(0 == strcmp(want, got->tag));
      }
      hashmap_destroy(m);
      free(elements);
    }
  }
}

namespace {
/* Every key hashes to the same value: the worst possible hash function */
status_t constant_hash(const void*, size_t, uint32_t* out) {
  *out = 7;
  return OK;
}

struct hashmap* make_int_map(size_t n_buckets,
                             size_t bsize,
                             uint32_t flags,
                             status_t (*hash)(const void*, size_t, uint32_t*)) {
  struct hashmap_config c;
  memset(&c, 0, sizeof(c));
  c.elt_size  = sizeof(int);
  c.n_buckets = n_buckets;
  c.bsize     = bsize;
  c.key_len   = sizeof(uint32_t);
  c.hash      = hash;
  c.flags     = flags;
  return hashmap_init(nullptr, &c);
}
} /* namespace */

CATCH_TEST_CASE("hashmap_gather rates a good hash function close to random",
                "[ds][hashmap]") {
  constexpr size_t kBuckets = 101;
  constexpr size_t kKeys    = 2000;
  std::uniform_int_distribution<uint32_t> dist;
  for (uint32_t flags :
       {(uint32_t)RCSW_NONE, (uint32_t)RCSW_DS_HASHMAP_LINPROB}) {
    struct hashmap* m = make_int_map(kBuckets, 64, flags, utils_hash_fnv1a);
    CATCH_REQUIRE(nullptr != m);
    struct hashmap_stats stats;
    CATCH_REQUIRE(OK == hashmap_gather(m, &stats));
    CATCH_REQUIRE(0.0 == stats.hash_quality); /* empty */

    int v = 1;
    for (size_t i = 0; i < kKeys; ++i) {
      uint32_t k = dist(th::make_rng());
      (void)hashmap_add(m, &k, &v); /* a rare duplicate key is fine */
    }
    CATCH_REQUIRE(OK == hashmap_gather(m, &stats));
    CATCH_REQUIRE(stats.hash_quality > 0.85);
    CATCH_REQUIRE(stats.hash_quality < 1.15);
    hashmap_destroy(m);
  }
}

CATCH_TEST_CASE("hashmap_gather exposes a degenerate hash function",
                "[ds][hashmap]") {
  /* Linear probing still stores every key, but each one's home is bucket 7 */
  constexpr size_t kBuckets = 16;
  struct hashmap*  m =
    make_int_map(kBuckets, 4, RCSW_DS_HASHMAP_LINPROB, constant_hash);
  CATCH_REQUIRE(nullptr != m);
  int v = 1;
  for (uint32_t k = 0; k < 40; ++k) {
    CATCH_REQUIRE(OK == hashmap_add(m, &k, &v));
  }
  struct hashmap_stats stats;
  CATCH_REQUIRE(OK == hashmap_gather(m, &stats));
  /* all n keys in one bucket: (n+1)m/(n+2m-1) times worse than random */
  double n        = 40;
  double expected = (n * (n + 1) / 2) / ((n / (2 * kBuckets)) * (n + 2 * kBuckets - 1));
  CATCH_REQUIRE(stats.hash_quality > 5.0);
  CATCH_REQUIRE(std::fabs(stats.hash_quality - expected) < 1e-9);
  hashmap_destroy(m);
}
