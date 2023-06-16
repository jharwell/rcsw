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
#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_PREFIX_ALL
#include <catch.hpp>

#include "rcsw/ds/rbuffer.h"
#include "rcsw/utils/utils.h"
#include "tests/ds_test.h"
#include "tests/ds_test.hpp"

/*******************************************************************************
 * Test Helper Functions
 ******************************************************************************/
template <typename T>
static void run_test(ds_test_t test) {
  /* dbg_init(); */
  /* dbg_insmod(M_TESTING,"Testing"); */
  /* dbg_insmod(M_DS_RBUFFER,"RBuffer"); */

  struct ds_params params;
  params.tag = ekRCSW_DS_RBUFFER;
  params.flags = 0;
  params.cmpe = th_cmpe<T>;
  params.printe = th_printe<T>;
  params.elt_size = sizeof(T);
  CATCH_REQUIRE(th_ds_init(&params) == OK);

  uint32_t flags[] = {
    RCSW_DS_NOALLOC_HANDLE,
    RCSW_DS_NOALLOC_DATA,
    RCSW_DS_RBUFFER_AS_FIFO
  };

  for (size_t j = 3; j < TH_NUM_ITEMS; ++j) {
    for (size_t i = 0; i < RCSW_ARRAY_SIZE(flags); ++i) {
      params.flags = flags[i];
      params.max_elts = j;
      test(j, &params);
    } /* for(i..) */
  } /* for(j..) */
  th_ds_shutdown(&params);
} /* test_runner() */

/*******************************************************************************
 * Test Functions
 ******************************************************************************/
template <typename T>
static void rdwr_test(int len, struct ds_params* params) {
  struct rbuffer *rb;
  struct rbuffer myrb;

  params->flags &= ~RCSW_DS_RBUFFER_AS_FIFO;
  rb = rbuffer_init(&myrb, params);
  CATCH_REQUIRE(rb != nullptr);
  rbuffer_print(rb);

  element_generator<T> g(gen_elt_type::ekINC_VALS, params->max_elts);

  for (int i = 0; i < len; i++) {
    T e = g.next();
    CATCH_REQUIRE(rbuffer_add(rb, &e) == OK);
  } /* for() */
  rbuffer_print(rb);

  /* verify rbuffer contents */
  for (int i = 0; i < len; ++i) {
    T* e = (T*)rbuffer_data_get(rb, i);
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

template<typename T>
static void overwrite_test(int len, struct ds_params *  params) {
  struct rbuffer *rb;
  struct rbuffer myrb;
  T arr[TH_NUM_ITEMS*TH_NUM_ITEMS];

  params->flags &= ~RCSW_DS_RBUFFER_AS_FIFO;
  rb = rbuffer_init(&myrb, params);

  unsigned tail = 0;
  unsigned count = 0;
  element_generator<T> g(gen_elt_type::ekINC_VALS, params->max_elts);

  /* test overwriting */
  for (int i = 0; i < len * len; i++) {
    T e = g.next();
    CATCH_REQUIRE(rbuffer_add(rb, &e) == OK);
    arr[i] = e;
    if (i >= len) {
      tail++;
    }
    count = 0;
    T * ep = nullptr;
    struct ds_iterator *iter = ds_iter_init(rb,
                                            ekRCSW_DS_RBUFFER,
                                            ekRCSW_DS_ITER_FORWARD);
    while ((ep = (T*)ds_iter_next(iter)) != nullptr) {
      CATCH_REQUIRE(memcmp(ep,
                           &arr[tail+count+rb->start],
                           sizeof(T)) == 0);
      count++;
    } /* while() */
  } /* for() */

  rbuffer_destroy(rb);
} /* overwrite_test() */

template<typename T>
static void map_test(int len, struct ds_params * params) {
  struct rbuffer *rb;
  struct rbuffer myrb;
  int arr[TH_NUM_ITEMS];

  rb = rbuffer_init(&myrb, params);
  CATCH_REQUIRE(rb != nullptr);

  element_generator<T> g(gen_elt_type::ekINC_VALS, params->max_elts);

  for (int i = 0; i < len; i++) {
    T e = g.next();
    arr[i] = i;
    CATCH_REQUIRE(rbuffer_add(rb, &e) == OK);
  } /* for() */

  CATCH_REQUIRE(rbuffer_map(rb, th_map_func<T>) == OK);

  for (int i = 0; i < len; ++i) {
    T e;
    CATCH_REQUIRE(rbuffer_remove(rb, &e) == OK);
    CATCH_REQUIRE(arr[i] == e.value1 +1);
  } /* for() */

  rbuffer_destroy(rb);
} /* map_test() */

template<typename T>
static void fifo_test(int len, struct ds_params * params) {
  struct rbuffer *rb;
  struct rbuffer myrb;
  T arr[TH_NUM_ITEMS];

  if (!(params->flags &= RCSW_DS_RBUFFER_AS_FIFO)) {
    return;
  }

  rb = rbuffer_init(&myrb, params);
  CATCH_REQUIRE(nullptr != rb);
  element_generator<T> g(gen_elt_type::ekINC_VALS, params->max_elts);

  /* fill the FIFO, verifying it does not allow addition of items once full */
  unsigned curr_old, start_old;
  for (int i = 0; i < len*2; ++i) {
    T e = g.next();
    if (i < len) {
      arr[i] = e;
      CATCH_REQUIRE(rbuffer_add(rb, &e) == OK);
    } else {
      curr_old = rb->current;
      start_old = rb->start;
      CATCH_REQUIRE(rbuffer_add(rb, &e) == ERROR);
      CATCH_REQUIRE(errno == ENOSPC);
      CATCH_REQUIRE((curr_old == rb->current && start_old == rb->start));
      T* e2 = (T*)rbuffer_front(rb);
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
      memmove(arr, arr+1, sizeof(arr) - sizeof(T));
    } else {
      T e = g.next();
      arr[len -1] = e;
      CATCH_REQUIRE(rbuffer_add(rb, &e) == OK);
    }
  } /* for() */

  rbuffer_destroy(rb);
} /* fifo_test() */

template <typename T>
static void inject_test(int len, struct ds_params * params) {
  struct rbuffer *rb;
  struct rbuffer myrb;
  int sum = 0;

  rb = rbuffer_init(&myrb, params);
  CATCH_REQUIRE(nullptr != rb);

  element_generator<T> g(gen_elt_type::ekINC_VALS, params->max_elts);
  for (int i = 0; i < len; i++) {
    T e = g.next();
    sum += i;
    CATCH_REQUIRE(rbuffer_add(rb, &e) == OK);
  } /* for() */

  int total = 0;
  CATCH_REQUIRE(rbuffer_inject(rb, th_inject_func<T>, &total) == OK);
  CATCH_REQUIRE(total == sum);

  rbuffer_destroy(rb);
} /* inject_test() */

template <typename T>
static void iter_test(int len, struct ds_params * params) {
  struct rbuffer *rb;
  struct rbuffer myrb;
  int arr[TH_NUM_ITEMS * TH_NUM_ITEMS];

  params->flags &= ~RCSW_DS_RBUFFER_AS_FIFO;
  rb = rbuffer_init(&myrb, params);
  CATCH_REQUIRE(rb);

  element_generator<T> g(gen_elt_type::ekINC_VALS, params->max_elts);
  for (int i = 0; i < len; i++) {
    T e = g.next();
    CATCH_REQUIRE(rbuffer_add(rb, &e) == OK);
  } /* for() */

  /* test iteration of without overwriting */
  T* e;
  struct ds_iterator * iter = ds_filter_init(rb,
                                             ekRCSW_DS_RBUFFER,
                                             th_iter_func<T>);
  CATCH_REQUIRE(iter != nullptr);
  while ((e = (T*)ds_iter_next(iter)) != nullptr) {
    CATCH_REQUIRE(e->value1 % 2 == 0);
  } /* while() */

  /*
   * Forward iteration
   */
  iter = ds_iter_init(rb, ekRCSW_DS_RBUFFER, ekRCSW_DS_ITER_FORWARD);
  CATCH_REQUIRE(iter);
  size_t count = 0;
  while ((e = (T*)ds_iter_next(iter)) != nullptr) {
    CATCH_REQUIRE((size_t)e->value1 == count);
    count++;
  } /* while() */
  CATCH_REQUIRE(count == rbuffer_n_elts(rb));

  /*
   * Backward iteration
   */
  iter = ds_iter_init(rb, ekRCSW_DS_RBUFFER, ekRCSW_DS_ITER_BACKWARD);
  CATCH_REQUIRE(nullptr != iter);
  count = 0;
  while ((e = (T*)ds_iter_next(iter)) != nullptr) {
    CATCH_REQUIRE((size_t)e->value1 == len - count - 1);
    count++;
  } /* while() */
  CATCH_REQUIRE(count == rbuffer_n_elts(rb));

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
    T* ep = nullptr;
    iter = ds_iter_init(rb, ekRCSW_DS_RBUFFER, ekRCSW_DS_ITER_FORWARD);
    while ((ep = (T*)ds_iter_next(iter)) != nullptr) {
      CATCH_REQUIRE(ep->value1 == arr[tail+count+rb->start]);
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
