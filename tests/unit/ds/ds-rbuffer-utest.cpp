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

#include "rcsw/ds/rbuffer.h"
#include "tests/unit/ds/ds_test.hpp"

/*******************************************************************************
 * Test Harness
 ******************************************************************************/
template <typename T>
static void run_test(void (*test)(int len, struct rbuffer_config* config)) {
  struct rbuffer_config config;
  memset(&config, 0, sizeof(rbuffer_config));
  config.cmpe     = th::cmpe<T>;
  config.printe   = th::printe<T>;
  config.elt_size = sizeof(T);
  CATCH_REQUIRE(th::ds_init(&config) == OK);

  uint32_t flags[] = {
    RCSW_ZALLOC,
    RCSW_NOALLOC_HANDLE,
    RCSW_NOALLOC_DATA,
    RCSW_DS_RBUFFER_AS_FIFO,
  };
  /* start at len=3 -- tests need at least 3 elements to be meaningful */
  for (int k = 3; k < TH_NUM_ITEMS; ++k) {
    config.max_elts = k;
    config.flags    = RCSW_NONE;
    test(k, &config);
  }
  for (size_t i = 0; i < RCSW_ARRAY_ELTS(flags); ++i) {
    for (size_t j = i + 1; j < RCSW_ARRAY_ELTS(flags); ++j) {
      uint32_t combo = flags[i] | flags[j];
      for (int k = 3; k < TH_NUM_ITEMS; ++k) {
        config.max_elts = k;
        config.flags    = combo;
        test(k, &config);
      }
    }
  }
  th::ds_shutdown(&config);
}

struct rbuffer* make_rb(size_t max_elts, uint32_t flags = RCSW_NONE) {
  struct rbuffer_config c;
  memset(&c, 0, sizeof(c));
  c.cmpe     = th::cmp_int;
  c.elt_size = sizeof(int);
  c.max_elts = max_elts;
  c.flags    = flags;
  return rbuffer_init(nullptr, &c);
}

/*******************************************************************************
 * Test Functions
 ******************************************************************************/
template <typename T>
static void rdwr_test(int len, struct rbuffer_config* config) {
  struct rbuffer myrb;
  config->flags &= ~RCSW_DS_RBUFFER_AS_FIFO;
  struct rbuffer* rb = rbuffer_init(&myrb, config);
  CATCH_REQUIRE(rb != nullptr);

  /* empty state */
  CATCH_REQUIRE(rbuffer_isempty(rb));
  T dummy{};
  CATCH_REQUIRE(ERROR == rbuffer_remove(rb, &dummy));

  th::element_generator<T> g(th::gen_elt_type::INC_VALS, config->max_elts);
  for (int i = 0; i < len; i++) {
    T e = g.next();
    CATCH_REQUIRE(rbuffer_add(rb, &e) == OK);
  }

  for (int i = 0; i < len; ++i) {
    T* e = (T*)rbuffer_data_get(rb, i);
    CATCH_REQUIRE(i == e->value1);
    CATCH_REQUIRE(i == rbuffer_index_query(rb, e));
  }

  for (int i = 0; i < len; ++i) {
    T e;
    CATCH_REQUIRE(rbuffer_remove(rb, &e) == OK);
    CATCH_REQUIRE((int)i == e.value1);
  }
  CATCH_REQUIRE(rbuffer_isempty(rb));

  rbuffer_destroy(rb);
}

template <typename T>
static void overwrite_test(int len, struct rbuffer_config* config) {
  struct rbuffer myrb;
  config->flags &= ~RCSW_DS_RBUFFER_AS_FIFO;
  struct rbuffer* rb = rbuffer_init(&myrb, config);
  CATCH_REQUIRE(rb != nullptr);

  std::vector<T>           arr;
  size_t                   tail = 0;
  th::element_generator<T> g(th::gen_elt_type::INC_VALS, config->max_elts);

  for (int i = 0; i < len * len; i++) {
    T e = g.next();
    CATCH_REQUIRE(rbuffer_add(rb, &e) == OK);
    arr.push_back(e);
    if (i >= len) {
      tail++;
    }
    size_t             count = 0;
    T*                 ep    = nullptr;
    struct ds_iterator iter;
    CATCH_REQUIRE(nullptr != rbuffer_iter_init(&iter, rb, nullptr));
    while ((ep = (T*)ds_iter_next(&iter)) != nullptr) {
      CATCH_REQUIRE(memcmp(ep, &arr[tail + count], sizeof(T)) == 0);
      count++;
    }
  }
  rbuffer_destroy(rb);
}

template <typename T>
static void map_test(int len, struct rbuffer_config* config) {
  struct rbuffer  myrb;
  struct rbuffer* rb = rbuffer_init(&myrb, config);
  CATCH_REQUIRE(rb != nullptr);

  /* NULL callback rejected */
  CATCH_REQUIRE(ERROR == rbuffer_map(rb, nullptr));

  th::element_generator<T> g(th::gen_elt_type::INC_VALS, config->max_elts);
  std::vector<int>         expected;
  for (int i = 0; i < len; i++) {
    T e = g.next();
    expected.push_back(i);
    CATCH_REQUIRE(rbuffer_add(rb, &e) == OK);
  }

  /* map_func decrements value1 */
  CATCH_REQUIRE(rbuffer_map(rb, th::map_func<T>) == OK);
  for (int i = 0; i < len; ++i) {
    T e;
    CATCH_REQUIRE(rbuffer_remove(rb, &e) == OK);
    CATCH_REQUIRE(expected[i] == e.value1 + 1);
  }
  rbuffer_destroy(rb);
}

template <typename T>
static void inject_test(int len, struct rbuffer_config* config) {
  struct rbuffer  myrb;
  struct rbuffer* rb = rbuffer_init(&myrb, config);
  CATCH_REQUIRE(nullptr != rb);

  /* NULL callback rejected */
  int dummy = 0;
  CATCH_REQUIRE(ERROR == rbuffer_inject(rb, nullptr, &dummy));

  th::element_generator<T> g(th::gen_elt_type::INC_VALS, config->max_elts);
  int                      expected_sum = 0;
  for (int i = 0; i < len; i++) {
    T e = g.next();
    expected_sum += i;
    CATCH_REQUIRE(rbuffer_add(rb, &e) == OK);
  }

  int total = 0;
  CATCH_REQUIRE(rbuffer_inject(rb, th::inject_func<T>, &total) == OK);
  CATCH_REQUIRE(total == expected_sum);

  rbuffer_destroy(rb);
}

template <typename T>
static void fifo_test(int len, struct rbuffer_config* config) {
  if (!(config->flags & RCSW_DS_RBUFFER_AS_FIFO)) {
    return;
  }
  struct rbuffer  myrb;
  struct rbuffer* rb = rbuffer_init(&myrb, config);
  CATCH_REQUIRE(nullptr != rb);

  th::element_generator<T> g(th::gen_elt_type::INC_VALS, config->max_elts);
  std::vector<T>           arr;

  for (int i = 0; i < len * 2; ++i) {
    T e = g.next();
    if (i < len) {
      arr.push_back(e);
      CATCH_REQUIRE(rbuffer_add(rb, &e) == OK);
    } else {
      /* FIFO full: must reject */
      unsigned curr_old  = rb->current;
      unsigned start_old = rb->start;
      CATCH_REQUIRE(rbuffer_add(rb, &e) == ERROR);
      CATCH_REQUIRE(errno == ENOSPC);
      CATCH_REQUIRE(curr_old == rb->current);
      CATCH_REQUIRE(start_old == rb->start);
      T* front = (T*)rbuffer_front(rb);
      CATCH_REQUIRE(front->value1 == arr[0].value1);
    }
  }
  rbuffer_destroy(rb);
}

template <typename T>
// NOLINTNEXTLINE(readability-function-size)
static void iter_test(int len, struct rbuffer_config* config) {
  struct rbuffer myrb;
  config->flags &= ~RCSW_DS_RBUFFER_AS_FIFO;
  struct rbuffer* rb = rbuffer_init(&myrb, config);
  CATCH_REQUIRE(rb != nullptr);

  th::element_generator<T> g(th::gen_elt_type::INC_VALS, config->max_elts);
  for (int i = 0; i < len; i++) {
    T e = g.next();
    CATCH_REQUIRE(rbuffer_add(rb, &e) == OK);
  }

  T*                 e;
  struct ds_iterator iter;

  /* filtered forward */
  CATCH_REQUIRE(nullptr != rbuffer_iter_init(&iter, rb, th::iter_func_even<T>));
  while ((e = (T*)ds_iter_next(&iter)) != nullptr) {
    CATCH_REQUIRE(e->value1 % 2 == 0);
  }

  /* unfiltered forward */
  CATCH_REQUIRE(nullptr != rbuffer_iter_init(&iter, rb, th::iter_func_all<T>));
  size_t count = 0;
  while ((e = (T*)ds_iter_next(&iter)) != nullptr) {
    CATCH_REQUIRE((size_t)e->value1 == count);
    count++;
  }
  CATCH_REQUIRE(count == rbuffer_size(rb));

  /* two independent iterators */
  struct ds_iterator iter2;
  CATCH_REQUIRE(nullptr != rbuffer_iter_init(&iter, rb, th::iter_func_all<T>));
  CATCH_REQUIRE(nullptr != rbuffer_iter_init(&iter2, rb, th::iter_func_all<T>));
  T* a = (T*)ds_iter_next(&iter);
  T* b = (T*)ds_iter_next(&iter2);
  if (len > 0) {
    CATCH_REQUIRE(a != nullptr);
    CATCH_REQUIRE(b != nullptr);
    CATCH_REQUIRE(th::cmpe<T>(a, b) == 0);
  }

  /* overwrite iteration */
  CATCH_REQUIRE(rbuffer_clear(rb) == OK);
  std::vector<T> arr;
  size_t         tail = 0;
  g.reset();
  for (int i = 0; i < len * len; i++) {
    T e2 = g.next();
    CATCH_REQUIRE(rbuffer_add(rb, &e2) == OK);
    arr.push_back(e2);
    if (i >= len) {
      tail++;
    }
    count = 0;
    T* ep = nullptr;
    CATCH_REQUIRE(nullptr != rbuffer_iter_init(&iter, rb, th::iter_func_all<T>));
    while ((ep = (T*)ds_iter_next(&iter)) != nullptr) {
      CATCH_REQUIRE(ep->value1 == arr[tail + count].value1);
      count++;
    }
  }
  rbuffer_destroy(rb);
}

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("rbuffer RDWR Test", "[ds][rbuffer][noalloc]") {
  run_test<element4>(rdwr_test<element4>);
  run_test<element8>(rdwr_test<element8>);
}
CATCH_TEST_CASE("rbuffer Overwrite Test", "[ds][rbuffer][noalloc]") {
  run_test<element4>(overwrite_test<element4>);
  run_test<element8>(overwrite_test<element8>);
}
CATCH_TEST_CASE("rbuffer Map Test", "[ds][rbuffer][noalloc]") {
  run_test<element4>(map_test<element4>);
  run_test<element8>(map_test<element8>);
}
CATCH_TEST_CASE("rbuffer Inject Test", "[ds][rbuffer][noalloc]") {
  run_test<element4>(inject_test<element4>);
  run_test<element8>(inject_test<element8>);
}
CATCH_TEST_CASE("rbuffer FIFO Test", "[ds][rbuffer][noalloc]") {
  run_test<element4>(fifo_test<element4>);
  run_test<element8>(fifo_test<element8>);
}
CATCH_TEST_CASE("rbuffer Iterator Test", "[ds][rbuffer][noalloc]") {
  run_test<element4>(iter_test<element4>);
  run_test<element8>(iter_test<element8>);
}
CATCH_TEST_CASE("rbuffer_clear zeroes the managed memory after wrap-around",
                "[ds][rbuffer]") {
  struct rbuffer* rb = make_rb(4);
  for (int i = 1; i <= 6; ++i) { /* overwrite mode: wraps, start != 0 */
    CATCH_REQUIRE(OK == rbuffer_add(rb, &i));
  }
  for (int i = 0; i < 3; ++i) {
    CATCH_REQUIRE(OK == rbuffer_remove(rb, nullptr));
  }
  CATCH_REQUIRE(OK == rbuffer_clear(rb));
  CATCH_REQUIRE(rbuffer_isempty(rb));

  const int* slots = reinterpret_cast<const int*>(rb->elements);
  for (size_t i = 0; i < 4; ++i) {
    CATCH_REQUIRE(0 == slots[i]);
  }
  rbuffer_destroy(rb);
}

CATCH_TEST_CASE("rbuffer_index_query does not match removed (stale) elements",
                "[ds][rbuffer]") {
  struct rbuffer* rb = make_rb(4);
  int             a  = 11;
  int             b  = 22;
  CATCH_REQUIRE(OK == rbuffer_add(rb, &a));
  CATCH_REQUIRE(OK == rbuffer_add(rb, &b));
  CATCH_REQUIRE(OK == rbuffer_remove(rb, nullptr)); /* removes 11 */

  CATCH_REQUIRE(-1 == rbuffer_index_query(rb, &a));
  CATCH_REQUIRE(-1 != rbuffer_index_query(rb, &b));

  CATCH_REQUIRE(OK == rbuffer_remove(rb, nullptr)); /* now empty */
  CATCH_REQUIRE(-1 == rbuffer_index_query(rb, &b));
  rbuffer_destroy(rb);
}

CATCH_TEST_CASE("rbuffer indices count from the front after wrap-around",
                "[ds][rbuffer]") {
  struct rbuffer* rb = make_rb(3);
  for (int i = 1; i <= 5; ++i) { /* overwrite mode: holds 3,4,5 */
    CATCH_REQUIRE(OK == rbuffer_add(rb, &i));
  }
  CATCH_REQUIRE(0 != rb->start); /* the oldest element is not in slot 0 */

  for (int i = 0; i < 3; ++i) {
    int val = 3 + i;
    CATCH_REQUIRE(val == *static_cast<int*>(rbuffer_data_get(rb, i)));
    CATCH_REQUIRE(i == rbuffer_index_query(rb, &val));
  }
  /* Past the live elements: rejected, not wrapped */
  CATCH_REQUIRE(nullptr == rbuffer_data_get(rb, 3));

  CATCH_REQUIRE(OK == rbuffer_remove(rb, nullptr)); /* removes 3 */
  CATCH_REQUIRE(4 == *static_cast<int*>(rbuffer_data_get(rb, 0)));
  CATCH_REQUIRE(nullptr == rbuffer_data_get(rb, 2));
  rbuffer_destroy(rb);
}

CATCH_TEST_CASE("rbuffer honors RCSW_ZALLOC for caller-provided element space",
                "[ds][rbuffer]") {
  static int space[8];
  memset(space, 0xAB, sizeof(space));

  struct rbuffer_config c;
  memset(&c, 0, sizeof(c));
  c.cmpe             = th::cmp_int;
  c.elt_size         = sizeof(int);
  c.max_elts         = 8;
  c.elements         = (dptr_t*)space;
  c.flags            = RCSW_NOALLOC_DATA | RCSW_ZALLOC;
  struct rbuffer* rb = rbuffer_init(nullptr, &c);
  CATCH_REQUIRE(nullptr != rb);

  const unsigned char* bytes = reinterpret_cast<const unsigned char*>(space);
  /* the element storage, not the (possibly larger) buffer, is zeroed */
  for (size_t i = 0; i < 8 * sizeof(int); ++i) {
    CATCH_REQUIRE(0 == bytes[i]);
  }
  rbuffer_destroy(rb);
}
