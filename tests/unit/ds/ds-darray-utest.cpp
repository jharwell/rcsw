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
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "rcsw/ds/darray.h"
#include "tests/unit/ds/ds_test.hpp"

/*******************************************************************************
 * Test Harness
 ******************************************************************************/
template <typename T>
static void run_test(void (*test)(int len, struct darray_config* config)) {
  struct darray_config config;
  memset(&config, 0, sizeof(darray_config));
  config.cmpe     = th::cmpe<T>;
  config.printe   = th::printe<T>;
  config.elt_size = sizeof(T);
  CATCH_REQUIRE(th::ds_init(&config) == OK);

  uint32_t flags[] = {
    RCSW_ZALLOC,
    RCSW_DS_SORTED,
    RCSW_DS_ORDERED,
    RCSW_NOALLOC_HANDLE,
    RCSW_NOALLOC_DATA,
  };
  th::run_test_flags(config, flags, RCSW_ARRAY_ELTS(flags), TH_NUM_ITEMS, test);
  th::ds_shutdown(&config);
}

namespace {
int cmp_int(const void* const a, const void* const b) {
  int x, y;
  memcpy(&x, a, sizeof(x));
  memcpy(&y, b, sizeof(y));
  return (x > y) - (x < y);
}

/* Compares only the first int of an 80-byte element */
int cmp_big(const void* const a, const void* const b) { return cmp_int(a, b); }

struct darray* make_int_array(int max_elts, size_t init_size, uint32_t flags) {
  struct darray_config c;
  memset(&c, 0, sizeof(c));
  c.cmpe      = cmp_int;
  c.elt_size  = sizeof(int);
  c.max_elts  = max_elts;
  c.init_size = init_size;
  c.flags     = flags;
  return darray_init(nullptr, &c);
}

void append(struct darray* arr, int v) {
  CATCH_REQUIRE(OK == darray_insert(arr, &v, darray_size(arr)));
}

int at(const struct darray* arr, size_t i) {
  int v;
  memcpy(&v, darray_data_get(arr, i), sizeof(v));
  return v;
}
} /* namespace */

template <typename T>
static void addremove_test(int len, struct darray_config* config) {
  struct darray* arr;
  struct darray  myarr;

  /* verify NULL handle rejected when NOALLOC_HANDLE set */
  if (config->flags & RCSW_NOALLOC_HANDLE) {
    CATCH_REQUIRE(nullptr == darray_init(nullptr, config));
    arr = darray_init(&myarr, config);
  } else {
    arr = darray_init(nullptr, config);
  }
  CATCH_REQUIRE(nullptr != arr);

  /* empty state assertions */
  CATCH_REQUIRE(darray_isempty(arr));
  CATCH_REQUIRE(darray_size(arr) == 0);
  T dummy{};
  CATCH_REQUIRE(ERROR == darray_remove(arr, &dummy, 0));

  th::element_generator<T> g(th::gen_elt_type::INC_VALS, config->max_elts);
  std::vector<T>           inserted;

  auto dist = std::uniform_int_distribution<size_t>(0, 1);

  for (int i = 0; i < len; i++) {
    T e = g.next();
    inserted.push_back(e);
    if (dist(th::make_rng())) {
      CATCH_REQUIRE(darray_insert(arr, &e, 0) == OK);
    } else {
      CATCH_REQUIRE(darray_insert(arr, &e, arr->current) == OK);
    }
    /* every inserted element must be findable */
    for (auto& el : inserted) {
      CATCH_REQUIRE(darray_idx_query(arr, &el) != -1);
    }
  }
  CATCH_REQUIRE(darray_size(arr) == (size_t)len);

  /* resize up */
  if (!(arr->flags & RCSW_NOALLOC_DATA)) {
    CATCH_REQUIRE(OK == darray_resize(arr, darray_size(arr) * 2));
  }

  /* remove all and verify each removal */
  for (int i = 0; i < len; i++) {
    T e;
    CATCH_REQUIRE(OK == darray_remove(arr, &e, 0));
    CATCH_REQUIRE(darray_idx_query(arr, &e) == -1);
  }
  CATCH_REQUIRE(darray_isempty(arr));
  darray_destroy(arr);
}

template <typename T>
static void sort_test(int len, struct darray_config* config) {
  struct darray* arr;
  struct darray  myarr;

  if (config->flags & RCSW_NOALLOC_HANDLE) {
    arr = darray_init(&myarr, config);
  } else {
    arr = darray_init(nullptr, config);
  }
  CATCH_REQUIRE(nullptr != arr);

  th::element_generator<T> g(th::gen_elt_type::RAND_VALS, config->max_elts);
  std::vector<T>           original;

  for (int i = 0; i < len; i++) {
    T e = g.next();
    original.push_back(e);
    CATCH_REQUIRE(darray_insert(arr, &e, arr->current) == OK);
  }

  auto dist = std::uniform_int_distribution<size_t>(0, 1);

  darray_sort(arr, (dist(th::make_rng())) ? EXEC_ITER : EXEC_REC);

  /* verify sorted order AND permutation */
  std::vector<T> sorted;
  sorted.reserve(len);
  for (int i = 0; i < len; i++) {
    sorted.emplace_back(*reinterpret_cast<T*>(darray_data_get(arr, i)));
  }
  th::verify_sort_permutation(original, sorted.data(), len);

  darray_destroy(arr);
}

template <typename T>
static void copy_test(int len, struct darray_config* config) {
  struct darray* arr1;
  struct darray* arr2;
  struct darray  myarr;

  if (config->flags & RCSW_NOALLOC_HANDLE) {
    arr1 = darray_init(&myarr, config);
  } else {
    arr1 = darray_init(nullptr, config);
  }
  CATCH_REQUIRE(nullptr != arr1);

  th::element_generator<T> g(th::gen_elt_type::RAND_VALS, config->max_elts);
  std::vector<T>           inserted;
  for (int i = 0; i < len; i++) {
    T e = g.next();
    inserted.push_back(e);
    CATCH_REQUIRE(darray_insert(arr1, &e, arr1->current) == OK);
  }

  /* copy using heap allocation regardless of source config */
  arr2 = darray_copy(arr1, 0x0, nullptr);
  CATCH_REQUIRE(nullptr != arr2);

  /* both copies contain all elements */
  for (auto& e : inserted) {
    CATCH_REQUIRE(darray_idx_query(arr1, &e) != -1);
    CATCH_REQUIRE(darray_idx_query(arr2, &e) != -1);
  }

  /* copy preserves cmpe and printe */
  CATCH_REQUIRE(arr2->cmpe == arr1->cmpe);

  darray_destroy(arr1);
  darray_destroy(arr2);
}

template <typename T>
static void map_test(int len, struct darray_config* config) {
  struct darray* arr;
  struct darray  myarr;

  if (config->flags & RCSW_NOALLOC_HANDLE) {
    arr = darray_init(&myarr, config);
  } else {
    arr = darray_init(nullptr, config);
  }
  CATCH_REQUIRE(nullptr != arr);

  /* NULL callback rejected */
  CATCH_REQUIRE(ERROR == darray_map(arr, nullptr));

  th::element_generator<T> g(th::gen_elt_type::INC_VALS, config->max_elts);
  for (int i = 0; i < len; i++) {
    T e = g.next();
    CATCH_REQUIRE(darray_insert(arr, &e, arr->current) == OK);
  }

  /* map_func decrements value1 by 1 */
  CATCH_REQUIRE(darray_map(arr, th::map_func<T>) == OK);
  for (int i = 0; i < len; i++) {
    T e;
    CATCH_REQUIRE(darray_idx_serve(arr, &e, i) == OK);
    CATCH_REQUIRE(e.value1 == i - 1);
  }
  darray_destroy(arr);
}

template <typename T>
static void inject_test(int len, struct darray_config* config) {
  struct darray* arr;
  struct darray  myarr;

  if (config->flags & RCSW_NOALLOC_HANDLE) {
    arr = darray_init(&myarr, config);
  } else {
    arr = darray_init(nullptr, config);
  }
  CATCH_REQUIRE(nullptr != arr);

  /* NULL callback rejected */
  int dummy = 0;
  CATCH_REQUIRE(ERROR == darray_inject(arr, nullptr, &dummy));

  th::element_generator<T> g(th::gen_elt_type::INC_VALS, config->max_elts);
  int                      expected_sum = 0;
  for (int i = 0; i < len; i++) {
    T e = g.next();
    expected_sum += i;
    CATCH_REQUIRE(darray_insert(arr, &e, arr->current) == OK);
  }

  int total = 0;
  CATCH_REQUIRE(darray_inject(arr, th::inject_func<T>, &total) == OK);
  CATCH_REQUIRE(total == expected_sum);

  darray_destroy(arr);
}

template <typename T>
// NOLINTNEXTLINE(readability-function-size)
static void iter_test(int len, struct darray_config* config) {
  struct darray* arr;
  struct darray  myarr;

  if (config->flags & RCSW_NOALLOC_HANDLE) {
    arr = darray_init(&myarr, config);
  } else {
    arr = darray_init(nullptr, config);
  }
  CATCH_REQUIRE(nullptr != arr);

  th::element_generator<T> g(th::gen_elt_type::INC_VALS, config->max_elts);
  for (int i = 0; i < len; i++) {
    T e = g.next();
    CATCH_REQUIRE(darray_insert(arr, &e, arr->current) == OK);
  }

  T*                 e;
  struct ds_iterator iter;

  /* filtered forward: only even values */
  CATCH_REQUIRE(
    nullptr != darray_iter_init(&iter, arr, ITER_FORWARD, th::iter_func_even<T>));
  while ((e = (T*)ds_iter_next(&iter)) != nullptr) {
    CATCH_REQUIRE(e->value1 % 2 == 0);
  }

  /* unfiltered forward: all values in order */
  CATCH_REQUIRE(nullptr !=
                darray_iter_init(&iter, arr, ITER_FORWARD, th::iter_func_all<T>));
  size_t count = 0;
  while ((e = (T*)ds_iter_next(&iter)) != nullptr) {
    CATCH_REQUIRE(e->value1 == (decltype(T::value1))count);
    count++;
  }
  CATCH_REQUIRE(count == darray_size(arr));

  /* unfiltered backward: values in reverse order */
  CATCH_REQUIRE(
    nullptr != darray_iter_init(&iter, arr, ITER_BACKWARD, th::iter_func_all<T>));
  count = 0;
  while ((e = (T*)ds_iter_next(&iter)) != nullptr) {
    CATCH_REQUIRE(e->value1 == len - (decltype(T::value1))count - 1);
    count++;
  }
  CATCH_REQUIRE(count == darray_size(arr));

  /* two independent iterators over the same array */
  struct ds_iterator iter2;
  CATCH_REQUIRE(nullptr !=
                darray_iter_init(&iter, arr, ITER_FORWARD, th::iter_func_all<T>));
  CATCH_REQUIRE(
    nullptr != darray_iter_init(&iter2, arr, ITER_FORWARD, th::iter_func_all<T>));
  T* e1 = (T*)ds_iter_next(&iter);
  T* e2 = (T*)ds_iter_next(&iter2);
  CATCH_REQUIRE(e1 != nullptr);
  CATCH_REQUIRE(e2 != nullptr);
  /* both point at the same first element */
  CATCH_REQUIRE(th::cmpe<T>(e1, e2) == 0);

  darray_destroy(arr);
}

template <typename T>
static void filter_test(int len, struct darray_config* config) {
  struct darray* arr1;
  struct darray* arr2;
  struct darray  myarr;

  if (config->flags & RCSW_NOALLOC_HANDLE) {
    arr1 = darray_init(&myarr, config);
  } else {
    arr1 = darray_init(nullptr, config);
  }
  CATCH_REQUIRE(nullptr != arr1);

  th::element_generator<T> g(th::gen_elt_type::INC_VALS, config->max_elts);
  std::vector<T>           inserted;
  for (int i = 0; i < len; i++) {
    T e = g.next();
    inserted.push_back(e);
    CATCH_REQUIRE(darray_insert(arr1, &e, arr1->current) == OK);
  }

  if (config->flags & (RCSW_NOALLOC_DATA | RCSW_NOALLOC_HANDLE)) {
    arr2 = darray_filter(arr1, th::filter_func<T>, 0, nullptr);
    CATCH_REQUIRE(nullptr != arr2);
    for (auto& el : inserted) {
      if (th::filter_func<T>(&el)) {
        CATCH_REQUIRE(darray_idx_query(arr2, &el) != -1);
        CATCH_REQUIRE(darray_idx_query(arr1, &el) == -1);
      } else {
        CATCH_REQUIRE(darray_idx_query(arr2, &el) == -1);
        CATCH_REQUIRE(darray_idx_query(arr1, &el) != -1);
      }
    }
    darray_destroy(arr2);
  }
  darray_destroy(arr1);
}

template <typename T>
static void binarysearch_test(int len, struct darray_config* config) {
  struct darray* arr;
  struct darray  myarr;
  std::vector<T> inserted;

  if (config->flags & RCSW_NOALLOC_HANDLE) {
    arr = darray_init(&myarr, config);
  } else {
    arr = darray_init(nullptr, config);
  }
  CATCH_REQUIRE(nullptr != arr);

  th::element_generator<T> g(th::gen_elt_type::RAND_VALS, config->max_elts);
  for (int i = 0; i < len; i++) {
    T e = g.next();
    inserted.push_back(e);
    CATCH_REQUIRE(darray_insert(arr, &e, arr->current) == OK);
  }

  darray_sort(arr, EXEC_ITER);

  /* every inserted element must be found */
  for (auto& e : inserted) {
    CATCH_REQUIRE(darray_idx_query(arr, &e) != -1);
  }

  darray_destroy(arr);
}

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("darray Add/Remove Test", "[ds][darray]") {
  run_test<element8>(addremove_test<element8>);
  run_test<element4>(addremove_test<element4>);
  run_test<element2>(addremove_test<element2>);
  run_test<element1>(addremove_test<element1>);
}
CATCH_TEST_CASE("darray Sort Test", "[ds][darray]") {
  run_test<element8>(sort_test<element8>);
  run_test<element4>(sort_test<element4>);
  run_test<element2>(sort_test<element2>);
  run_test<element1>(sort_test<element1>);
}
CATCH_TEST_CASE("darray Copy Test", "[ds][darray]") {
  run_test<element8>(copy_test<element8>);
  run_test<element4>(copy_test<element4>);
  run_test<element2>(copy_test<element2>);
  run_test<element1>(copy_test<element1>);
}
CATCH_TEST_CASE("darray Map Test", "[ds][darray]") {
  run_test<element8>(map_test<element8>);
  run_test<element4>(map_test<element4>);
  run_test<element2>(map_test<element2>);
  run_test<element1>(map_test<element1>);
}
CATCH_TEST_CASE("darray Inject Test", "[ds][darray]") {
  run_test<element8>(inject_test<element8>);
  run_test<element4>(inject_test<element4>);
  run_test<element2>(inject_test<element2>);
  run_test<element1>(inject_test<element1>);
}
CATCH_TEST_CASE("darray Iterator Test", "[ds][darray]") {
  run_test<element8>(iter_test<element8>);
  run_test<element4>(iter_test<element4>);
  run_test<element2>(iter_test<element2>);
  run_test<element1>(iter_test<element1>);
}
CATCH_TEST_CASE("darray Filter Test", "[ds][darray]") {
  run_test<element8>(filter_test<element8>);
  run_test<element4>(filter_test<element4>);
  run_test<element2>(filter_test<element2>);
  run_test<element1>(filter_test<element1>);
}
CATCH_TEST_CASE("darray Binary Search Test", "[ds][darray]") {
  run_test<element8>(binarysearch_test<element8>);
  run_test<element4>(binarysearch_test<element4>);
  run_test<element2>(binarysearch_test<element2>);
  run_test<element1>(binarysearch_test<element1>);
}
CATCH_TEST_CASE("darray Print Test", "[ds][darray]") {
  /* Coverage test: verify print doesn't crash on NULL or populated arrays */
  struct darray_config config;
  memset(&config, 0, sizeof(darray_config));
  config.cmpe     = th::cmpe<element4>;
  config.printe   = th::printe<element4>;
  config.elt_size = sizeof(element4);
  config.max_elts = TH_NUM_ITEMS;
  CATCH_REQUIRE(th::ds_init(&config) == OK);
  struct darray* arr = darray_init(nullptr, &config);
  CATCH_REQUIRE(arr != nullptr);
  darray_print(nullptr); /* must not crash */
  darray_print(arr);     /* empty */
  element4 e{};
  e.value1 = 1;
  darray_insert(arr, &e, 0);
  darray_print(arr); /* one element */
  darray_destroy(arr);
  th::ds_shutdown(&config);
}

CATCH_TEST_CASE("darray_idx_query returns the FIRST matching index",
                "[ds][darray]") {
  struct darray* arr = make_int_array(-1, 4, RCSW_NONE);
  for (int v : {5, 7, 5}) {
    append(arr, v);
  }
  int key = 5;
  CATCH_REQUIRE(0 == darray_idx_query(arr, &key));
  darray_destroy(arr);
}

CATCH_TEST_CASE(
  "darray: unsorted insert after darray_sort() invalidates "
  "the cached sorted state",
  "[ds][darray]") {
  struct darray* arr = make_int_array(-1, 4, RCSW_NONE);
  for (int v : {5, 7, 5}) {
    append(arr, v);
  }
  CATCH_REQUIRE(OK == darray_sort(arr, EXEC_REC));

  /* Inserting out of order must not leave arr->sorted == true */
  int nine = 9;
  int one  = 1;
  CATCH_REQUIRE(OK == darray_insert(arr, &nine, 0));
  CATCH_REQUIRE(OK == darray_insert(arr, &one, darray_size(arr)));

  CATCH_REQUIRE(-1 != darray_idx_query(arr, &one));
  CATCH_REQUIRE(-1 != darray_idx_query(arr, &nine));
  darray_destroy(arr);
}

CATCH_TEST_CASE(
  "darray: unordered remove after darray_sort() invalidates "
  "the cached sorted state",
  "[ds][darray]") {
  struct darray* arr = make_int_array(-1, 8, RCSW_NONE);
  for (int v : {1, 2, 3, 4, 5, 6}) {
    append(arr, v);
  }
  CATCH_REQUIRE(OK == darray_sort(arr, EXEC_ITER));
  /* Without RCSW_DS_SORTED/ORDERED, remove moves the last element into the
   * hole, so the array is no longer sorted. */
  CATCH_REQUIRE(OK == darray_remove(arr, nullptr, 0));
  for (int v : {2, 3, 4, 5, 6}) {
    CATCH_REQUIRE(-1 != darray_idx_query(arr, &v));
  }
  darray_destroy(arr);
}

CATCH_TEST_CASE("darray_remove preserves relative order with RCSW_DS_ORDERED",
                "[ds][darray]") {
  struct darray* arr = make_int_array(-1, 8, RCSW_DS_ORDERED);
  for (int v : {10, 20, 30, 40, 50}) {
    append(arr, v);
  }
  CATCH_REQUIRE(OK == darray_remove(arr, nullptr, 1));
  std::vector<int> expected = {10, 30, 40, 50};
  CATCH_REQUIRE(darray_size(arr) == expected.size());
  for (size_t i = 0; i < expected.size(); ++i) {
    CATCH_REQUIRE(at(arr, i) == expected[i]);
  }
  darray_destroy(arr);
}

CATCH_TEST_CASE(
  "darray_resize to exactly the current size keeps every "
  "element",
  "[ds][darray]") {
  struct darray* arr = make_int_array(-1, 4, RCSW_NONE);
  for (int v : {0, 1, 2, 3}) {
    append(arr, v);
  }
  CATCH_REQUIRE(OK == darray_resize(arr, 8));
  CATCH_REQUIRE(OK == darray_resize(arr, 4));
  CATCH_REQUIRE(4 == darray_size(arr));
  for (int i = 0; i < 4; ++i) {
    CATCH_REQUIRE(at(arr, i) == i);
  }
  darray_destroy(arr);
}

CATCH_TEST_CASE("darray_resize rejects sizes whose byte count overflows",
                "[ds][darray]") {
  struct darray* arr     = make_int_array(-1, 4, RCSW_NONE);
  size_t         old_cap = darray_capacity(arr);
  /* (SIZE_MAX/4 + 2) * sizeof(int) wraps around to a tiny allocation */
  size_t bad_size = (SIZE_MAX / sizeof(int)) + 2;
  CATCH_REQUIRE(ERROR == darray_resize(arr, bad_size));
  CATCH_REQUIRE(old_cap == darray_capacity(arr));
  darray_destroy(arr);
}

CATCH_TEST_CASE("darray_sort sorts elements larger than 64 bytes",
                "[ds][darray]") {
  for (auto type : {EXEC_REC, EXEC_ITER}) {
    struct darray_config c;
    memset(&c, 0, sizeof(c));
    c.cmpe             = cmp_big;
    c.elt_size         = 80;
    c.max_elts         = -1;
    c.init_size        = 4;
    struct darray* arr = darray_init(nullptr, &c);
    CATCH_REQUIRE(nullptr != arr);

    unsigned char buf[80];
    for (int v = 3; v >= 0; --v) {
      memset(buf, 0, sizeof(buf));
      memcpy(buf, &v, sizeof(v));
      CATCH_REQUIRE(OK == darray_insert(arr, buf, darray_size(arr)));
    }
    CATCH_REQUIRE(OK == darray_sort(arr, type));
    for (int i = 0; i < 4; ++i) {
      CATCH_REQUIRE(at(arr, i) == i);
    }
    darray_destroy(arr);
  }
}

CATCH_TEST_CASE("darray_copy of a full, bounded array succeeds", "[ds][darray]") {
  struct darray* arr = make_int_array(4, 2, RCSW_NONE);
  for (int v : {0, 1, 2, 3}) {
    append(arr, v);
  }
  CATCH_REQUIRE(darray_isfull(arr));

  struct darray* copy = darray_copy(arr, RCSW_NONE, nullptr);
  CATCH_REQUIRE(nullptr != copy);
  CATCH_REQUIRE(4 == darray_size(copy));
  for (int i = 0; i < 4; ++i) {
    CATCH_REQUIRE(at(copy, i) == i);
  }
  darray_destroy(copy);
  darray_destroy(arr);
}

CATCH_TEST_CASE(
  "darray_init rejects RCSW_NOALLOC_DATA with an unbounded "
  "max_elts",
  "[ds][darray][noalloc]") {
  static dptr_t        space[64];
  struct darray_config c;
  memset(&c, 0, sizeof(c));
  c.cmpe     = cmp_int;
  c.elt_size = sizeof(int);
  c.max_elts = -1;
  c.elements = space;
  c.flags    = RCSW_NOALLOC_DATA;
  /* Caller-provided space has a fixed size, so max_elts must be bounded */
  CATCH_REQUIRE(nullptr == darray_init(nullptr, &c));
}

CATCH_TEST_CASE("darray honors RCSW_ZALLOC for caller-provided element space",
                "[ds][darray]") {
  static dptr_t space[16];
  memset(space, 0xAB, sizeof(space));

  struct darray_config c;
  memset(&c, 0, sizeof(c));
  c.cmpe             = cmp_int;
  c.elt_size         = sizeof(int);
  c.max_elts         = 16;
  c.init_size        = 16;
  c.elements         = space;
  c.flags            = RCSW_NOALLOC_DATA | RCSW_ZALLOC;
  struct darray* arr = darray_init(nullptr, &c);
  CATCH_REQUIRE(nullptr != arr);

  const unsigned char* bytes = reinterpret_cast<const unsigned char*>(space);
  /* the element storage, not the (possibly larger) buffer, is zeroed */
  for (size_t i = 0; i < 16 * sizeof(int); ++i) {
    CATCH_REQUIRE(0 == bytes[i]);
  }
  darray_destroy(arr);
}

CATCH_TEST_CASE(
  "darray: 8-byte elements in 4-byte-aligned caller space are "
  "copied without misaligned access",
  "[ds][darray]") {
  /*
   * RCSW_CONFIG_PTR_ALIGN=4 promises 4-byte alignment. Element 1 of an
   * 8-byte-element array then sits at offset 8 from a 4-aligned base, which
   * ds_elt_copy() loads through a double*. Run with
   * -fno-sanitize-recover=alignment (or UBSAN_OPTIONS=halt_on_error=1) for
   * this test to fail on the current code.
   */
  alignas(8) static unsigned char raw[8 * 8 + 4];
  auto*                           base = reinterpret_cast<dptr_t*>(raw + 4);

  struct darray_config c;
  memset(&c, 0, sizeof(c));
  c.cmpe             = cmp_int;
  c.elt_size         = 8;
  c.max_elts         = 8;
  c.init_size        = 8;
  c.elements         = base;
  c.flags            = RCSW_NOALLOC_DATA;
  struct darray* arr = darray_init(nullptr, &c);
  CATCH_REQUIRE(nullptr != arr);

  uint64_t v = 0x1122334455667788ULL;
  for (int i = 0; i < 4; ++i) {
    CATCH_REQUIRE(OK == darray_insert(arr, &v, darray_size(arr)));
  }
  uint64_t out = 0;
  CATCH_REQUIRE(OK == darray_idx_serve(arr, &out, 3));
  CATCH_REQUIRE(out == v);
  darray_destroy(arr);
}
