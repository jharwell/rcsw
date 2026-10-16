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
#define CATCH_CONFIG_PREFIX_ALL
#include <algorithm>
#include <set>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "rcsw/ds/llist.h"
#include "rcsw/utils/hash.h"
#include "tests/unit/ds/ds_test.hpp"

/*******************************************************************************
 * Test Harness
 ******************************************************************************/
template <typename T>
static void run_test(void (*test)(int len, struct llist_config *config)) {
  struct llist_config config;
  memset(&config, 0, sizeof(llist_config));
  config.cmpe = th::cmpe<T>;
  config.printe = th::printe<T>;
  config.elt_size = sizeof(T);
  CATCH_REQUIRE(th::ds_init(&config) == OK);

  uint32_t flags[] = {
      RCSW_ZALLOC,
      RCSW_NOALLOC_HANDLE,
      RCSW_NOALLOC_DATA,
      RCSW_NOALLOC_META,
  };
  th::run_test_flags(config, flags, RCSW_ARRAY_ELTS(flags), TH_NUM_ITEMS, test);
  th::ds_shutdown(&config);
}

namespace {
struct key_buf {
  char bytes[RCSW_HASHMAP_KEYSIZE];
};

/* A zero-padded, full-width key: safe under any key semantics */
key_buf make_key(const char *s) {
  key_buf k;
  memset(&k, 0, sizeof(k));
  snprintf(k.bytes, sizeof(k.bytes), "%s", s);
  return k;
}

struct hashmap *make_map(size_t n_buckets, size_t bsize, uint32_t flags) {
  struct hashmap_config c;
  memset(&c, 0, sizeof(c));
  c.elt_size = sizeof(int);
  c.n_buckets = n_buckets;
  c.bsize = bsize;
  c.hash = utils_hash_fnv1a;
  c.flags = flags;
  return hashmap_init(nullptr, &c);
}

size_t bucket_of(const key_buf &k, size_t n_buckets) {
  uint32_t h = 0;
  utils_hash_fnv1a(k.bytes, sizeof(k.bytes), &h);
  return h % n_buckets;
}

/* Two distinct keys that hash to the same bucket */
std::pair<key_buf, key_buf> colliding_keys(size_t n_buckets) {
  key_buf first = make_key("k0");
  size_t b = bucket_of(first, n_buckets);
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

template <typename T>
static void insert_test(int len, struct llist_config *config) {
  struct llist *list;
  struct llist mylist;

  if (config->flags & RCSW_NOALLOC_HANDLE) {
    CATCH_REQUIRE(nullptr == llist_init(nullptr, config));
    list = llist_init(&mylist, config);
  } else {
    list = llist_init(nullptr, config);
  }
  CATCH_REQUIRE(nullptr != list);

  /* empty state */
  CATCH_REQUIRE(llist_isempty(list));
  CATCH_REQUIRE(llist_size(list) == 0);

  th::element_generator<T> g(th::gen_elt_type::RAND_VALS, config->max_elts);
  std::vector<T> inserted;

  auto dist = std::uniform_int_distribution<size_t>(0, 1);
  for (int i = 0; i < len; i++) {
    T e = g.next();
    if (dist(th::make_rng())) {
      CATCH_REQUIRE(llist_append(list, &e) == OK);
    } else {
      CATCH_REQUIRE(llist_prepend(list, &e) == OK);
    }
    inserted.push_back(e);
    for (auto &el : inserted) {
      CATCH_REQUIRE(llist_data_query(list, &el));
    }
  }

  /* full: append/prepend nullptr rejected */
  if (len == config->max_elts) {
    CATCH_REQUIRE(llist_isfull(list));
  }
  CATCH_REQUIRE(llist_append(list, nullptr) == ERROR);
  CATCH_REQUIRE(llist_prepend(list, nullptr) == ERROR);

  llist_destroy(list);
  CATCH_REQUIRE(th::leak_check_data(config) == 0);
  CATCH_REQUIRE(th::leak_check_nodes(config) == 0);
}

template <typename T>
static void remove_if_test(int len, struct llist_config *config) {
  struct llist *list;
  struct llist mylist;

  if (config->flags & RCSW_NOALLOC_HANDLE) {
    list = llist_init(&mylist, config);
  } else {
    list = llist_init(nullptr, config);
  }
  CATCH_REQUIRE(nullptr != list);

  th::element_generator<T> g(th::gen_elt_type::RAND_VALS, config->max_elts);
  std::vector<T> inserted;
  for (int i = 0; i < len; i++) {
    T e = g.next();
    CATCH_REQUIRE(llist_append(list, &e) == OK);
    inserted.push_back(e);
  }

  CATCH_REQUIRE(llist_remove_if(list, th::filter_func<T>) == OK);

  /* matching elements removed, non-matching remain */
  for (auto &e : inserted) {
    if (th::filter_func<T>(&e)) {
      CATCH_REQUIRE(llist_node_query(list, &e) == nullptr);
    } else {
      CATCH_REQUIRE(llist_node_query(list, &e) != nullptr);
    }
  }

  llist_destroy(list);
  CATCH_REQUIRE(th::leak_check_data(config) == 0);
  CATCH_REQUIRE(th::leak_check_nodes(config) == 0);
}

template <typename T>
static void copy_test(int len, struct llist_config *config) {
  struct llist *list1;
  struct llist *list2;
  struct llist mylist;

  if (config->flags & RCSW_NOALLOC_HANDLE) {
    list1 = llist_init(&mylist, config);
  } else {
    list1 = llist_init(nullptr, config);
  }
  CATCH_REQUIRE(nullptr != list1);

  th::element_generator<T> g(th::gen_elt_type::RAND_VALS, config->max_elts);
  std::vector<T> inserted;
  for (int i = 0; i < len; i++) {
    T e = g.next();
    CATCH_REQUIRE(llist_append(list1, &e) == OK);
    inserted.push_back(e);
  }

  /* copy_if: only elements satisfying filter_func */
  list2 = llist_copy_if(list1, th::filter_func<T>, 0, nullptr, nullptr);
  CATCH_REQUIRE(nullptr != list2);

  for (auto &e : inserted) {
    if (th::filter_func<T>(&e)) {
      CATCH_REQUIRE(llist_node_query(list2, &e) != nullptr);
    }
    /* original list unchanged */
    CATCH_REQUIRE(llist_node_query(list1, &e) != nullptr);
  }
  llist_destroy(list2);

  /* full copy: all elements present in both */
  list2 = llist_copy(list1, 0, nullptr, nullptr);
  CATCH_REQUIRE(nullptr != list2);
  for (auto &e : inserted) {
    CATCH_REQUIRE(llist_node_query(list1, &e) != nullptr);
    CATCH_REQUIRE(llist_node_query(list2, &e) != nullptr);
  }
  llist_destroy(list2);

  llist_destroy(list1);
  CATCH_REQUIRE(th::leak_check_data(config) == 0);
  CATCH_REQUIRE(th::leak_check_nodes(config) == 0);
}

template <typename T>
static void sort_test(int len, struct llist_config *config) {
  struct llist *list;
  struct llist mylist;

  if (config->flags & RCSW_NOALLOC_HANDLE) {
    list = llist_init(&mylist, config);
  } else {
    list = llist_init(nullptr, config);
  }
  CATCH_REQUIRE(nullptr != list);

  for (int i = 0; i < len; i++) {
    T e;
    auto dist = std::uniform_int_distribution<size_t>(0, i);
    e.value1 = dist(th::make_rng()) + i;
    CATCH_REQUIRE(llist_append(list, &e) == OK);
  }

  auto dist2 = std::uniform_int_distribution<size_t>(0, 1);
  CATCH_REQUIRE(OK == llist_sort(list, (exec_type)(dist2(th::make_rng()))));

  /* verify sorted */
  int val = -1;
  LLIST_FOREACH(list, next, curr) {
    T *e = (T *)curr->data;
    CATCH_REQUIRE(val <= e->value1);
    val = (unsigned char)e->value1;
  }

  llist_destroy(list);
  CATCH_REQUIRE(th::leak_check_data(config) == 0);
  CATCH_REQUIRE(th::leak_check_nodes(config) == 0);
}

template <typename T>
static void inject_test(int len, struct llist_config *config) {
  struct llist *list;
  struct llist mylist;

  if (config->flags & RCSW_NOALLOC_HANDLE) {
    list = llist_init(&mylist, config);
  } else {
    list = llist_init(nullptr, config);
  }
  CATCH_REQUIRE(nullptr != list);

  /* NULL callback rejected */
  int dummy = 0;
  CATCH_REQUIRE(ERROR == llist_inject(list, nullptr, &dummy));

  th::element_generator<T> g(th::gen_elt_type::INC_VALS, config->max_elts);
  int expected_sum = 0;
  for (int i = 0; i < len; i++) {
    T e = g.next();
    expected_sum += i;
    CATCH_REQUIRE(llist_append(list, &e) == OK);
  }

  int total = 0;
  CATCH_REQUIRE(llist_inject(list, th::inject_func<T>, &total) == OK);
  CATCH_REQUIRE(total == expected_sum);

  llist_destroy(list);
}

template <typename T>
// NOLINTNEXTLINE(readability-function-size)
static void iter_test(int len, struct llist_config *config) {
  struct llist *list;
  struct llist mylist;

  if (config->flags & RCSW_NOALLOC_HANDLE) {
    list = llist_init(&mylist, config);
  } else {
    list = llist_init(nullptr, config);
  }
  CATCH_REQUIRE(nullptr != list);

  th::element_generator<T> g(th::gen_elt_type::INC_VALS, config->max_elts);
  for (int i = 0; i < len; i++) {
    T e = g.next();
    CATCH_REQUIRE(llist_append(list, &e) == OK);
  }

  T *e;
  struct ds_iterator iter;

  /* filtered forward */
  CATCH_REQUIRE(nullptr != llist_iter_init(&iter, list, ITER_FORWARD,
                                           th::iter_func_even<T>));
  while ((e = (T *)ds_iter_next(&iter)) != nullptr) {
    CATCH_REQUIRE(e->value1 % 2 == 0);
  }

  /* unfiltered forward */
  CATCH_REQUIRE(nullptr != llist_iter_init(&iter, list, ITER_FORWARD,
                                           th::iter_func_all<T>));
  size_t count = 0;
  while ((e = (T *)ds_iter_next(&iter)) != nullptr) {
    CATCH_REQUIRE((size_t)e->value1 == count);
    count++;
  }
  CATCH_REQUIRE(count == list->current);

  /* unfiltered backward: values in reverse order */
  CATCH_REQUIRE(nullptr != llist_iter_init(&iter, list, ITER_BACKWARD,
                                           th::iter_func_all<T>));
  count = 0;
  while ((e = (T *)ds_iter_next(&iter)) != nullptr) {
    CATCH_REQUIRE((size_t)e->value1 == (size_t)len - count - 1);
    count++;
  }
  CATCH_REQUIRE(count == list->current);

  /* two independent iterators */
  struct ds_iterator iter2;
  CATCH_REQUIRE(nullptr != llist_iter_init(&iter, list, ITER_FORWARD,
                                           th::iter_func_all<T>));
  CATCH_REQUIRE(nullptr != llist_iter_init(&iter2, list, ITER_FORWARD,
                                           th::iter_func_all<T>));
  T *a = (T *)ds_iter_next(&iter);
  T *b = (T *)ds_iter_next(&iter2);
  if (len > 0) {
    CATCH_REQUIRE(a != nullptr);
    CATCH_REQUIRE(b != nullptr);
    CATCH_REQUIRE(th::cmpe<T>(a, b) == 0);
  }

  llist_destroy(list);
}

/* key/tag pair: equality (cmpe) is on key only, so two elements can compare
 * equal while being distinguishable by tag. */
struct kv {
  int key;
  int tag;
};

int cmp_key(const void *const a, const void *const b) {
  const auto *x = static_cast<const kv *>(a);
  const auto *y = static_cast<const kv *>(b);
  return (x->key > y->key) - (x->key < y->key);
}

bool_t tag_is_odd(const void *const e) {
  return (static_cast<const kv *>(e)->tag % 2) != 0;
}

bool_t key_is_even(const void *const e) {
  return (static_cast<const kv *>(e)->key % 2) == 0;
}

static struct llist *make_list(int max_elts, uint32_t flags = RCSW_NONE) {
  struct llist_config c;
  memset(&c, 0, sizeof(c));
  c.cmpe = cmp_key;
  c.elt_size = sizeof(kv);
  c.max_elts = max_elts;
  c.flags = flags;
  return llist_init(nullptr, &c);
}

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("llist Insert Test", "[ds][llist]") {
  run_test<element8>(insert_test<element8>);
  run_test<element4>(insert_test<element4>);
  run_test<element2>(insert_test<element2>);
  run_test<element1>(insert_test<element1>);
}
CATCH_TEST_CASE("llist Remove_if Test", "[ds][llist]") {
  run_test<element8>(remove_if_test<element8>);
  run_test<element4>(remove_if_test<element4>);
  run_test<element2>(remove_if_test<element2>);
  run_test<element1>(remove_if_test<element1>);
}
CATCH_TEST_CASE("llist Copy Test", "[ds][llist]") {
  run_test<element8>(copy_test<element8>);
  run_test<element4>(copy_test<element4>);
  run_test<element2>(copy_test<element2>);
  run_test<element1>(copy_test<element1>);
}
CATCH_TEST_CASE("llist Sort Test", "[ds][llist]") {
  run_test<element8>(sort_test<element8>);
  run_test<element4>(sort_test<element4>);
  run_test<element2>(sort_test<element2>);
  run_test<element1>(sort_test<element1>);
}
CATCH_TEST_CASE("llist Inject Test", "[ds][llist]") {
  run_test<element8>(inject_test<element8>);
  run_test<element4>(inject_test<element4>);
  run_test<element2>(inject_test<element2>);
  run_test<element1>(inject_test<element1>);
}
CATCH_TEST_CASE("llist Iterator Test", "[ds][llist]") {
  run_test<element8>(iter_test<element8>);
  run_test<element4>(iter_test<element4>);
  run_test<element2>(iter_test<element2>);
  run_test<element1>(iter_test<element1>);
}
CATCH_TEST_CASE("llist_filter places nodes in the nodes buffer and data in "
                "the elements buffer",
                "[ds][llist]") {
  struct llist *src = make_list(8);
  for (int i = 0; i < 6; ++i) {
    kv e = {i, i};
    CATCH_REQUIRE(OK == llist_append(src, &e));
  }
  /* Exactly-sized heap buffers so ASan catches any overrun */
  size_t elt_bytes = llist_element_space(8, sizeof(kv));
  size_t node_bytes = llist_meta_space(8);
  auto *elements = static_cast<unsigned char *>(malloc(elt_bytes));
  auto *nodes = static_cast<unsigned char *>(malloc(node_bytes));

  struct llist *out = llist_filter(
      src, key_is_even, RCSW_NOALLOC_DATA | RCSW_NOALLOC_META, elements, nodes);
  CATCH_REQUIRE(nullptr != out);
  CATCH_REQUIRE(nullptr != out->first);

  auto *first = reinterpret_cast<unsigned char *>(out->first);
  auto *data = reinterpret_cast<unsigned char *>(out->first->data);
  CATCH_REQUIRE(first >= nodes);
  CATCH_REQUIRE(first < nodes + node_bytes);
  CATCH_REQUIRE(data >= elements);
  CATCH_REQUIRE(data < elements + elt_bytes);

  llist_destroy(out);
  llist_destroy(src);
  free(elements);
  free(nodes);
}

CATCH_TEST_CASE("llist: append after llist_sort() invalidates the cached "
                "sorted state",
                "[ds][llist]") {
  struct llist *l = make_list(-1);
  for (int k : {3, 1, 2}) {
    kv e = {k, 0};
    llist_append(l, &e);
  }
  CATCH_REQUIRE(OK == llist_sort(l, EXEC_REC));
  kv zero = {0, 0};
  llist_append(l, &zero);
  CATCH_REQUIRE(OK == llist_sort(l, EXEC_REC));
  CATCH_REQUIRE(0 == reinterpret_cast<kv *>(l->first->data)->key);
  llist_destroy(l);
}

CATCH_TEST_CASE("llist_remove_if removes the matching NODE even when an "
                "earlier node compares equal",
                "[ds][llist]") {
  struct llist *l = make_list(-1);
  kv a = {1, 0}; /* same key, tag even -> keep */
  kv b = {1, 1}; /* same key, tag odd  -> remove */
  llist_append(l, &a);
  llist_append(l, &b);

  CATCH_REQUIRE(OK == llist_remove_if(l, tag_is_odd));
  CATCH_REQUIRE(1 == llist_size(l));
  CATCH_REQUIRE(0 == reinterpret_cast<kv *>(l->first->data)->tag);
  llist_destroy(l);
}

CATCH_TEST_CASE("llist_filter moves the matching NODE even when an earlier "
                "node compares equal",
                "[ds][llist]") {
  struct llist *l = make_list(-1);
  kv a = {1, 0};
  kv b = {1, 1};
  llist_append(l, &a);
  llist_append(l, &b);

  struct llist *out = llist_filter(l, tag_is_odd, RCSW_NONE, nullptr, nullptr);
  CATCH_REQUIRE(nullptr != out);
  CATCH_REQUIRE(1 == llist_size(out));
  CATCH_REQUIRE(1 == reinterpret_cast<kv *>(out->first->data)->tag);
  CATCH_REQUIRE(1 == llist_size(l));
  CATCH_REQUIRE(0 == reinterpret_cast<kv *>(l->first->data)->tag);
  llist_destroy(out);
  llist_destroy(l);
}

CATCH_TEST_CASE("llist_splice refuses lists backed by caller-provided pools",
                "[ds][llist]") {
  struct llist *heap = make_list(16);
  kv e = {1, 1};
  CATCH_REQUIRE(OK == llist_append(heap, &e));

  static dptr_t elements[512];
  static dptr_t nodes[512];
  struct llist_config c;
  memset(&c, 0, sizeof(c));
  c.cmpe = cmp_key;
  c.elt_size = sizeof(kv);
  c.max_elts = 8;
  c.elements = elements;
  c.meta = nodes;
  c.flags = RCSW_NOALLOC_DATA | RCSW_NOALLOC_META;
  struct llist *pooled = llist_init(nullptr, &c);
  CATCH_REQUIRE(nullptr != pooled);
  kv f = {2, 2};
  CATCH_REQUIRE(OK == llist_append(pooled, &f));

  /* Spliced nodes would later be returned to the wrong allocator */
  errno = 0;
  CATCH_REQUIRE(ERROR == llist_splice(heap, pooled, heap->last));
  CATCH_REQUIRE(EINVAL == errno);

  /* Both lists are left untouched */
  CATCH_REQUIRE(1 == llist_size(heap));
  CATCH_REQUIRE(1 == llist_size(pooled));
  llist_destroy(pooled);
  llist_destroy(heap);
}

CATCH_TEST_CASE("llist_remove: empty list and missing element are ENOENT",
                "[ds][llist]") {
  struct llist *l = make_list(-1);
  kv probe = {42, 0};
  errno = 0;
  status_t empty_rc = llist_remove(l, &probe);
  int empty_err = errno;

  kv other = {1, 0};
  llist_append(l, &other);
  errno = 0;
  status_t missing_rc = llist_remove(l, &probe);
  int miss_err = errno;

  CATCH_REQUIRE(ERROR == empty_rc);
  CATCH_REQUIRE(ENOENT == empty_err);
  CATCH_REQUIRE(ERROR == missing_rc);
  CATCH_REQUIRE(ENOENT == miss_err);
  llist_destroy(l);
}

CATCH_TEST_CASE("a successful llist_init does not modify errno",
                "[ds][llist]") {
  /*
   * Module registration (RCSW_ER_MODULE_INIT()) may fail, e.g. before the
   * logging plugin is initialized; that must not leak into errno.
   */
  errno = 0;
  struct llist *l = make_list(4);
  CATCH_REQUIRE(nullptr != l);
  CATCH_REQUIRE(0 == errno);
  llist_destroy(l);
}

CATCH_TEST_CASE("caller-provided llist pools are reused in any order",
                "[ds][llist][noalloc]") {
  constexpr int kMax = 8;
  std::vector<dptr_t> elements(llist_element_space(kMax, sizeof(kv)) /
                               sizeof(dptr_t));
  std::vector<dptr_t> nodes(llist_meta_space(kMax) / sizeof(dptr_t));
  struct llist_config c;
  struct llist handle;
  memset(&c, 0, sizeof(c));
  c.cmpe = cmp_key;
  c.elt_size = sizeof(kv);
  c.max_elts = kMax;
  c.elements = elements.data();
  c.meta = nodes.data();
                  c.flags = RCSW_NOALLOC_ALL;
  struct llist *l = llist_init(&handle, &c);
  CATCH_REQUIRE(nullptr != l);

  for (int round = 0; round < 3; ++round) {
    for (int i = 0; i < kMax; ++i) {
      kv e = {i, round};
      CATCH_REQUIRE(OK == llist_append(l, &e));
    }
    /* full: the pools are exhausted */
    kv extra = {99, 0};
    CATCH_REQUIRE(ERROR == llist_append(l, &extra));

    /* every live element has its own datablock and node */
    std::set<const void *> blocks;
    std::set<const void *> addrs;
    LLIST_FOREACH(l, next, node) {
      blocks.insert(node->data);
      addrs.insert(node);
    }
    CATCH_REQUIRE(kMax == blocks.size());
    CATCH_REQUIRE(kMax == addrs.size());

    /* free the odd keys, then the even ones: out of allocation order */
    for (int parity = 1; parity >= 0; --parity) {
      for (int i = parity; i < kMax; i += 2) {
        kv e = {i, 0};
        CATCH_REQUIRE(OK == llist_remove(l, &e));
      }
    }
    CATCH_REQUIRE(0 == llist_size(l));
  }
  llist_destroy(l);
}

CATCH_TEST_CASE("RCSW_DS_EXTFLAGS_SHIFT is past every ds flag",
                "[ds][noalloc]") {
  constexpr uint32_t kAll = RCSW_DS_SORTED | RCSW_DS_ORDERED |
                            RCSW_DS_HASHMAP_LINPROB | RCSW_DS_RBUFFER_AS_FIFO |
                            RCSW_DS_LLIST_DB_DISOWN | RCSW_DS_LLIST_DB_PTR |
                            RCSW_DS_BSTREE_RB | RCSW_DS_BSTREE_INT |
                            RCSW_DS_BSTREE_OS | RCSW_DS_BINHEAP_MIN;
  static_assert(0 == (kAll & ~((1U << RCSW_DS_EXTFLAGS_SHIFT) - 1)),
                "a ds flag is at or above RCSW_DS_EXTFLAGS_SHIFT");
  static_assert(0 == (kAll & (RCSW_MODFLAGS_BASE - 1)),
                "a ds flag overlaps the common flags");
  CATCH_SUCCEED();
}

namespace {
/*
 * The keys of \p l in order, front to back. Fails the test if the prev links,
 * the last pointer, or the size disagree with the next links.
 */
std::vector<int> keys_of(const struct llist *l) {
  std::vector<int> keys;
  const struct llist_node *prev = nullptr;
  for (const struct llist_node *n = l->first; n != nullptr; n = n->next) {
    CATCH_REQUIRE(prev == n->prev);
    keys.push_back(reinterpret_cast<const kv *>(n->data)->key);
    prev = n;
  }
  CATCH_REQUIRE(prev == l->last);
  CATCH_REQUIRE(keys.size() == llist_size(l));
  return keys;
}

struct llist *list_of(std::initializer_list<int> keys,
                      uint32_t flags = RCSW_NONE,
                      int max_elts = -1) {
  struct llist *l = make_list(max_elts, flags);
  CATCH_REQUIRE(nullptr != l);
  for (int k : keys) {
    kv e = {k, 0};
    CATCH_REQUIRE(OK == llist_append(l, &e));
  }
  return l;
}
} /* namespace */

CATCH_TEST_CASE("llist_sort keeps the prev links and last pointer consistent",
                "[ds][llist]") {
  std::uniform_int_distribution<int> len_dist(0, 50);
  std::uniform_int_distribution<int> key_dist(0, 19); /* plenty of ties */
  for (int trial = 0; trial < 200; ++trial) {
    for (exec_type how : {EXEC_REC, EXEC_ITER}) {
      struct llist *l = make_list(-1);
      int len = len_dist(th::make_rng());
      for (int i = 0; i < len; ++i) {
        kv e = {key_dist(th::make_rng()), i};
        CATCH_REQUIRE(OK == llist_append(l, &e));
      }
      CATCH_REQUIRE(OK == llist_sort(l, how));
      std::vector<int> keys = keys_of(l);
      CATCH_REQUIRE(std::is_sorted(keys.begin(), keys.end()));
      CATCH_REQUIRE(len == static_cast<int>(keys.size()));
      llist_destroy(l);
    }
  }
}

CATCH_TEST_CASE("llist_splice inserts list2 before the given node",
                "[ds][llist]") {
  struct llist *l1 = list_of({1, 2, 3});

  /* before a middle node */
  CATCH_REQUIRE(OK == llist_splice(l1, list_of({7, 8}), l1->first->next));
  CATCH_REQUIRE(std::vector<int>({1, 7, 8, 2, 3}) == keys_of(l1));

  /* before the first node: prepend */
  CATCH_REQUIRE(OK == llist_splice(l1, list_of({5}), l1->first));
  CATCH_REQUIRE(std::vector<int>({5, 1, 7, 8, 2, 3}) == keys_of(l1));

  /* NULL: append */
  CATCH_REQUIRE(OK == llist_splice(l1, list_of({9, 4}), nullptr));
  CATCH_REQUIRE(std::vector<int>({5, 1, 7, 8, 2, 3, 9, 4}) == keys_of(l1));
  llist_destroy(l1);
}

CATCH_TEST_CASE("llist_splice into a one-element list: first and NULL differ",
                "[ds][llist]") {
  struct llist *l1 = list_of({1});
  CATCH_REQUIRE(OK == llist_splice(l1, list_of({7, 8}), l1->first));
  CATCH_REQUIRE(std::vector<int>({7, 8, 1}) == keys_of(l1));
  llist_destroy(l1);

  l1 = list_of({1});
  CATCH_REQUIRE(OK == llist_splice(l1, list_of({7, 8}), nullptr));
  CATCH_REQUIRE(std::vector<int>({1, 7, 8}) == keys_of(l1));
  llist_destroy(l1);
}

CATCH_TEST_CASE("llist_splice accepts empty lists", "[ds][llist]") {
  struct llist *l1 = list_of({});
  CATCH_REQUIRE(OK == llist_splice(l1, list_of({7, 8}), nullptr));
  CATCH_REQUIRE(std::vector<int>({7, 8}) == keys_of(l1));

  CATCH_REQUIRE(OK == llist_splice(l1, list_of({}), nullptr));
  CATCH_REQUIRE(std::vector<int>({7, 8}) == keys_of(l1));
  llist_destroy(l1);
}

CATCH_TEST_CASE("llist_splice refuses a node that is not in list1",
                "[ds][llist]") {
  struct llist *l1 = list_of({1, 2, 3});
  struct llist *l2 = list_of({7, 8});
  struct llist *other = list_of({7, 8});

  errno = 0;
  CATCH_REQUIRE(ERROR == llist_splice(l1, l2, other->first));
  CATCH_REQUIRE(ENOENT == errno);

  /* neither list changed */
  CATCH_REQUIRE(std::vector<int>({1, 2, 3}) == keys_of(l1));
  CATCH_REQUIRE(std::vector<int>({7, 8}) == keys_of(l2));
  llist_destroy(other);
  llist_destroy(l2);
  llist_destroy(l1);
}

CATCH_TEST_CASE("llist_splice refuses lists that disagree on data ownership",
                "[ds][llist]") {
  struct llist *owning = list_of({1, 2, 3});

  /* a list storing pointers to caller-owned data */
  std::vector<kv> store = {{7, 0}, {8, 0}};
  struct llist *ptrs = make_list(-1, RCSW_DS_LLIST_DB_DISOWN);
  for (auto &e : store) {
    CATCH_REQUIRE(OK == llist_append(ptrs, &e));
  }

  /* list1 would free (or fail to free) list2's datablocks */
  errno = 0;
  CATCH_REQUIRE(ERROR == llist_splice(owning, ptrs, nullptr));
  CATCH_REQUIRE(EINVAL == errno);
  errno = 0;
  CATCH_REQUIRE(ERROR == llist_splice(ptrs, owning, nullptr));
  CATCH_REQUIRE(EINVAL == errno);

  CATCH_REQUIRE(std::vector<int>({1, 2, 3}) == keys_of(owning));
  CATCH_REQUIRE(std::vector<int>({7, 8}) == keys_of(ptrs));
  llist_destroy(ptrs);
  llist_destroy(owning);
}

CATCH_TEST_CASE("llist_splice keeps a sorted list1 sorted", "[ds][llist]") {
  struct llist *l1 = list_of({1, 4, 6}, RCSW_DS_SORTED);
  CATCH_REQUIRE(OK == llist_splice(l1, list_of({5, 2}), l1->first));
  CATCH_REQUIRE(std::vector<int>({1, 2, 4, 5, 6}) == keys_of(l1));
  llist_destroy(l1);
}

CATCH_TEST_CASE("llist_splice respects list1's max_elts", "[ds][llist]") {
  struct llist *l1 = list_of({1, 2, 3}, RCSW_NONE, 4);
  struct llist *l2 = list_of({7, 8});

  errno = 0;
  CATCH_REQUIRE(ERROR == llist_splice(l1, l2, nullptr));
  CATCH_REQUIRE(ENOSPC == errno);
  CATCH_REQUIRE(std::vector<int>({1, 2, 3}) == keys_of(l1));
  CATCH_REQUIRE(std::vector<int>({7, 8}) == keys_of(l2));
  llist_destroy(l2);
  llist_destroy(l1);
}

CATCH_TEST_CASE("llist_filter refuses a pointer-storing result for a list "
                "that owns its data",
                "[ds][llist]") {
  struct llist *src = list_of({1, 2, 3, 4});

  /* the result would point at datablocks src frees as it removes them */
  errno = 0;
  CATCH_REQUIRE(nullptr == llist_filter(src,
                                        key_is_even,
                                        RCSW_DS_LLIST_DB_DISOWN,
                                        nullptr,
                                        nullptr));
  CATCH_REQUIRE(EINVAL == errno);
  CATCH_REQUIRE(std::vector<int>({1, 2, 3, 4}) == keys_of(src));

  /* a copying result is fine */
  struct llist *out =
      llist_filter(src, key_is_even, RCSW_NONE, nullptr, nullptr);
  CATCH_REQUIRE(nullptr != out);
  CATCH_REQUIRE(std::vector<int>({2, 4}) == keys_of(out));
  CATCH_REQUIRE(std::vector<int>({1, 3}) == keys_of(src));
  llist_destroy(out);
  llist_destroy(src);
}

CATCH_TEST_CASE("llist_filter between pointer-storing lists moves the pointers",
                "[ds][llist]") {
  std::vector<kv> store = {{1, 0}, {2, 0}, {3, 0}, {4, 0}};
  struct llist *src = make_list(-1, RCSW_DS_LLIST_DB_DISOWN);
  for (auto &e : store) {
    CATCH_REQUIRE(OK == llist_append(src, &e));
  }
  struct llist *out = llist_filter(src,
                                   key_is_even,
                                   RCSW_DS_LLIST_DB_DISOWN,
                                   nullptr,
                                   nullptr);
  CATCH_REQUIRE(nullptr != out);
  CATCH_REQUIRE(reinterpret_cast<dptr_t *>(&store[1]) == out->first->data);
  CATCH_REQUIRE(reinterpret_cast<dptr_t *>(&store[3]) == out->last->data);
  llist_destroy(out);
  llist_destroy(src);
}

CATCH_TEST_CASE("LLIST_FOREACH scopes its variable to the loop",
                "[ds][llist]") {
  struct llist *l = list_of({1, 2, 3, 4});
  int sum = 0;
  int digits = 0;
  /* the same variable name twice in one block: each loop declares its own */
  LLIST_FOREACH(l, next, node) { sum += reinterpret_cast<kv *>(node->data)->key; }
  LLIST_FOREACH(l, prev, node) { (void)node; }
  LLIST_ITER(l, l->last, prev, node) {
    digits = digits * 10 + reinterpret_cast<kv *>(node->data)->key;
  }
  CATCH_REQUIRE(10 == sum);
  CATCH_REQUIRE(4321 == digits);
  llist_destroy(l);
}

CATCH_TEST_CASE("llist keeps its configuration, and copies inherit it",
                "[ds][llist]") {
  struct llist *l = make_list(8, RCSW_DS_SORTED);
  CATCH_REQUIRE(nullptr != l);
  CATCH_REQUIRE(cmp_key == l->cfg.cmpe);
  CATCH_REQUIRE(sizeof(kv) == l->cfg.elt_size);
  CATCH_REQUIRE(8 == l->cfg.max_elts);
  CATCH_REQUIRE(RCSW_DS_SORTED == l->cfg.flags);
  CATCH_REQUIRE(nullptr == l->cfg.elements); /* heap storage */

  kv e = {1, 0};
  CATCH_REQUIRE(OK == llist_append(l, &e));

  /* the copy takes the element configuration from the list, flags from us */
  struct llist *c = llist_copy(l, RCSW_NONE, nullptr, nullptr);
  CATCH_REQUIRE(nullptr != c);
  CATCH_REQUIRE(l->cfg.cmpe == c->cfg.cmpe);
  CATCH_REQUIRE(l->cfg.printe == c->cfg.printe);
  CATCH_REQUIRE(l->cfg.elt_size == c->cfg.elt_size);
  CATCH_REQUIRE(l->cfg.max_elts == c->cfg.max_elts);
  CATCH_REQUIRE(RCSW_NONE == c->cfg.flags);
  CATCH_REQUIRE(std::vector<int>({1}) == keys_of(c));
  llist_destroy(c);
  llist_destroy(l);
}
