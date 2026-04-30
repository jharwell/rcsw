/**
 * \file ds-rbuffer-utest.cpp
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#define CATCH_CONFIG_PREFIX_ALL
#include <catch2/catch_test_macros.hpp>

#include "rcsw/ds/rbuffer.h"
#include "tests/ds_test.h"
#include "tests/ds_test.hpp"

/*******************************************************************************
 * Namespaces/Decls
 ******************************************************************************/
using rbuffer_test_t = void (*)(int len, struct rbuffer_config *config);

/*******************************************************************************
 * Test Helper Functions
 ******************************************************************************/
template <typename T>
static void run_test(rbuffer_test_t test) {
  RCSW_ER_INIT(TH_ZLOG_CONF);
  struct rbuffer_config config;
  memset(&config, 0, sizeof(rbuffer_config));
  config.flags    = 0;
  config.cmpe     = th::cmpe<T>;
  config.printe   = th::printe<T>;
  config.elt_size = sizeof(T);
  CATCH_REQUIRE(th::ds_init(&config) == OK);

  uint32_t flags[] = {RCSW_NONE,
                      RCSW_ZALLOC,
                      RCSW_NOALLOC_HANDLE,
                      RCSW_NOALLOC_DATA,
                      RCSW_DS_RBUFFER_AS_FIFO};

  uint32_t applied = 0;
  for (size_t i = 0; i < RCSW_ARRAY_ELTS(flags); ++i) {
    applied |= flags[i];
    for (size_t j = i + 1; j < RCSW_ARRAY_ELTS(flags); ++j) {
      applied |= flags[j];
      for (size_t k = 3; k < TH_NUM_ITEMS; ++k) {
        config.max_elts = k;
        test(k, &config);
      } /* for(k..) */
      applied &= ~flags[j];
    } /* for(j..) */
  } /* for(i..) */

  th::ds_shutdown(&config);
  RCSW_ER_DEINIT();
} /* test_runner() */

/*******************************************************************************
 * Test Functions
 ******************************************************************************/
template <typename T>
static void rdwr_test(int len, struct rbuffer_config *config) {
  struct rbuffer *rb;
  struct rbuffer  myrb;

  config->flags &= ~RCSW_DS_RBUFFER_AS_FIFO;
  rb = rbuffer_init(&myrb, config);
  CATCH_REQUIRE(rb != nullptr);
  rbuffer_print(rb);

  th::element_generator<T> g(gen_elt_type::ekINC_VALS, config->max_elts);

  for (int i = 0; i < len; i++) {
    T e = g.next();
    CATCH_REQUIRE(rbuffer_add(rb, &e) == OK);
  } /* for() */
  rbuffer_print(rb);

  /* verify rbuffer contents */
  for (int i = 0; i < len; ++i) {
    T *e = (T *)rbuffer_data_get(rb, i);
    CATCH_REQUIRE(i == e->value1);
    CATCH_REQUIRE(i == rbuffer_index_query(rb, e));
  } /* for() */

  /* test reading out rbuffer contents */
  for (int i = 0; i < len; ++i) {
    T e;
    CATCH_REQUIRE(rbuffer_remove(rb, &e) == OK);
    CATCH_REQUIRE((int)i == e.value1);
  } /* for() */

  rbuffer_destroy(rb);
} /* rdwr_test() */

template <typename T>
static void overwrite_test(int len, struct rbuffer_config *config) {
  struct rbuffer *rb;
  struct rbuffer  myrb;
  T               arr[TH_NUM_ITEMS * TH_NUM_ITEMS];

  config->flags &= ~RCSW_DS_RBUFFER_AS_FIFO;
  rb = rbuffer_init(&myrb, config);

  unsigned                 tail  = 0;
  unsigned                 count = 0;
  th::element_generator<T> g(gen_elt_type::ekINC_VALS, config->max_elts);

  /* test overwriting */
  for (int i = 0; i < len * len; i++) {
    T e = g.next();
    CATCH_REQUIRE(rbuffer_add(rb, &e) == OK);
    arr[i] = e;
    if (i >= len) {
      tail++;
    }
    count                  = 0;
    T                  *ep = nullptr;
    struct ds_iterator iter;
    CATCH_REQUIRE(nullptr != rbuffer_iter_init(&iter, rb, NULL));
    while ((ep = (T *)ds_iter_next(&iter)) != nullptr) {
      CATCH_REQUIRE(memcmp(ep, &arr[tail + count], sizeof(T)) == 0);

      CATCH_REQUIRE(memcmp(ep, &arr[tail + count], sizeof(T)) == 0);
      count++;
    } /* while() */
  } /* for() */

  rbuffer_destroy(rb);
}

template <typename T>
static void map_test(int len, struct rbuffer_config *config) {
  struct rbuffer *rb;
  struct rbuffer  myrb;
  int             arr[TH_NUM_ITEMS];

  rb = rbuffer_init(&myrb, config);
  CATCH_REQUIRE(rb != nullptr);

  th::element_generator<T> g(gen_elt_type::ekINC_VALS, config->max_elts);

  for (int i = 0; i < len; i++) {
    T e    = g.next();
    arr[i] = i;
    CATCH_REQUIRE(rbuffer_add(rb, &e) == OK);
  } /* for() */

  CATCH_REQUIRE(rbuffer_map(rb, th::map_func<T>) == OK);

  for (int i = 0; i < len; ++i) {
    T e;
    CATCH_REQUIRE(rbuffer_remove(rb, &e) == OK);
    CATCH_REQUIRE(arr[i] == e.value1 + 1);
  } /* for() */

  rbuffer_destroy(rb);
} /* map_test() */

template <typename T>
static void fifo_test(int len, struct rbuffer_config *config) {
  struct rbuffer *rb;
  struct rbuffer  myrb;
  T               arr[TH_NUM_ITEMS];

  if (!(config->flags &= RCSW_DS_RBUFFER_AS_FIFO)) {
    return;
  }

  rb = rbuffer_init(&myrb, config);
  CATCH_REQUIRE(nullptr != rb);
  th::element_generator<T> g(gen_elt_type::ekINC_VALS, config->max_elts);

  /* fill the FIFO, verifying it does not allow addition of items once full */
  unsigned curr_old, start_old;
  for (int i = 0; i < len * 2; ++i) {
    T e = g.next();
    if (i < len) {
      arr[i] = e;
      CATCH_REQUIRE(rbuffer_add(rb, &e) == OK);
    } else {
      curr_old  = rb->current;
      start_old = rb->start;
      CATCH_REQUIRE(rbuffer_add(rb, &e) == ERROR);
      CATCH_REQUIRE(errno == ENOSPC);
      CATCH_REQUIRE((curr_old == rb->current && start_old == rb->start));
      T *e2 = (T *)rbuffer_front(rb);
      CATCH_REQUIRE(e2->value1 == arr[0].value1);
    }
  } /* for() */

  /* perform a rolling test */
  g.reset();
  for (int i = 0; i < len; ++i) {
    if (rb->current == (size_t)len) {
      T e;
      CATCH_REQUIRE(rbuffer_remove(rb, &e) == OK);
      CATCH_REQUIRE(arr[0].value1 == e.value1);
      memmove(arr, arr + 1, sizeof(arr) - sizeof(T));
    } else {
      T e          = g.next();
      arr[len - 1] = e;
      CATCH_REQUIRE(rbuffer_add(rb, &e) == OK);
    }
  } /* for() */

  rbuffer_destroy(rb);
} /* fifo_test() */

template <typename T>
static void inject_test(int len, struct rbuffer_config *config) {
  struct rbuffer *rb;
  struct rbuffer  myrb;
  int             sum = 0;

  rb = rbuffer_init(&myrb, config);
  CATCH_REQUIRE(nullptr != rb);

  th::element_generator<T> g(gen_elt_type::ekINC_VALS, config->max_elts);
  for (int i = 0; i < len; i++) {
    T e = g.next();
    sum += i;
    CATCH_REQUIRE(rbuffer_add(rb, &e) == OK);
  } /* for() */

  int total = 0;
  CATCH_REQUIRE(rbuffer_inject(rb, th::inject_func<T>, &total) == OK);
  CATCH_REQUIRE(total == sum);

  rbuffer_destroy(rb);
} /* inject_test() */

template <typename T>
static void iter_test(int len, struct rbuffer_config *config) {
  struct rbuffer *rb;
  struct rbuffer  myrb;
  int             arr[TH_NUM_ITEMS * TH_NUM_ITEMS];

  config->flags &= ~RCSW_DS_RBUFFER_AS_FIFO;
  rb = rbuffer_init(&myrb, config);
  CATCH_REQUIRE(rb);

  th::element_generator<T> g(gen_elt_type::ekINC_VALS, config->max_elts);
  for (int i = 0; i < len; i++) {
    T e = g.next();
    CATCH_REQUIRE(rbuffer_add(rb, &e) == OK);
  } /* for() */

  /* test iteration of without overwriting */
  T                  *e;
  struct ds_iterator iter;
  CATCH_REQUIRE(nullptr != rbuffer_iter_init(&iter, rb, th::iter_func_even<T>));
  while ((e = (T *)ds_iter_next(&iter)) != nullptr) {
    CATCH_REQUIRE(e->value1 % 2 == 0);
  } /* while() */

  /*
   * Forward iteration
   */
  CATCH_REQUIRE(nullptr != rbuffer_iter_init(&iter, rb, th::iter_func_all<T>));
  size_t count = 0;
  while ((e = (T *)ds_iter_next(&iter)) != nullptr) {
    CATCH_REQUIRE((size_t)e->value1 == count);
    count++;
  } /* while() */
  CATCH_REQUIRE(count == rbuffer_size(rb));

  /*
   * Iteration with overwriting
   */
  CATCH_REQUIRE(rbuffer_clear(rb) == OK);
  size_t tail = 0;

  g.reset();
  for (int i = 0; i < len * len; i++) {
    T e2 = g.next();
    CATCH_REQUIRE(rbuffer_add(rb, &e2) == OK);
    arr[i] = i;
    if (i >= len) {
      tail++;
    }

    /* verify iteration */
    count = 0;
    T *ep = nullptr;
    CATCH_REQUIRE(nullptr != rbuffer_iter_init(&iter, rb, th::iter_func_all<T>));
    while ((ep = (T *)ds_iter_next(&iter)) != nullptr) {
      CATCH_REQUIRE(ep->value1 == arr[tail + count]);
      count++;
    } /* while() */
  } /* for() */

  rbuffer_destroy(rb);
} /* iter_test() */

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("RDWR Test", "[ds][rbuffer]") {
  run_test<element1>(rdwr_test<element1>);
  run_test<element2>(rdwr_test<element2>);
  run_test<element4>(rdwr_test<element4>);
  run_test<element8>(rdwr_test<element8>);
}
CATCH_TEST_CASE("Overwrite Test", "[ds][rbuffer]") {
  run_test<element1>(overwrite_test<element1>);
  run_test<element2>(overwrite_test<element2>);
  run_test<element4>(overwrite_test<element4>);
  run_test<element8>(overwrite_test<element8>);
}
CATCH_TEST_CASE("Map Test", "[ds][rbuffer]") {
  run_test<element1>(map_test<element1>);
  run_test<element2>(map_test<element2>);
  run_test<element4>(map_test<element4>);
  run_test<element8>(map_test<element8>);
}
CATCH_TEST_CASE("FIFO Test", "[ds][rbuffer]") {
  run_test<element1>(fifo_test<element1>);
  run_test<element2>(fifo_test<element2>);
  run_test<element4>(fifo_test<element4>);
  run_test<element8>(fifo_test<element8>);
}
CATCH_TEST_CASE("Inject Test", "[ds][rbuffer]") {
  run_test<element1>(inject_test<element1>);
  run_test<element2>(inject_test<element2>);
  run_test<element4>(inject_test<element4>);
  run_test<element8>(inject_test<element8>);
}
CATCH_TEST_CASE("Iterator Test", "[ds][rbuffer]") {
  /* don't run with element[1,2]--problems with max representable integer */
  run_test<element4>(iter_test<element4>);
  run_test<element8>(iter_test<element8>);
}
