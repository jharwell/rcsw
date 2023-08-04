/**
 * \file mt-pcqueue-utest.cpp
 *
 * \copyright 2023 John Harwell, All rights reserved.
 *
 * SPDX-License Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <thread>
#include <mutex>
#include <numeric>

#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_PREFIX_ALL
#include <catch.hpp>

#include "rcsw/multithread/pcqueue.h"

#include "tests/ds_test.hpp"

#define TH_NUM_MT_ITEMS 1000

/*******************************************************************************
 * Namespaces/Decls
 ******************************************************************************/
using pcqueue_test = void(*)(const struct pcqueue_params* const params,
                             size_t n_prod, size_t n_cons);

/*******************************************************************************
 * Test Helper Functions
 ******************************************************************************/
template<typename T>
static void run_test(pcqueue_test test, size_t n_prod = 1, size_t n_cons = 1) {
  struct pcqueue_params params;
  params.flags = 0;
  params.printe = NULL;
  params.elt_size = sizeof(T);
  params.max_elts = TH_NUM_MT_ITEMS * 10;
  uint32_t flags[] = {
    RCSW_NONE,
    RCSW_NOALLOC_HANDLE,
  };

  for (size_t i = 0; i < RCSW_ARRAY_ELTS(flags); ++i) {
    params.flags = flags[i];
    test(&params, n_prod, n_cons);
  }
} /* test_runner() */

/*******************************************************************************
 * Test Functions
 ******************************************************************************/
template <typename T>
static void serial_test(const struct pcqueue_params* const params,
                 size_t,
                 size_t) {
  struct pcqueue queue_in;
  struct pcqueue* queue;

  if (params->flags & RCSW_NOALLOC_HANDLE) {
    queue = pcqueue_init(NULL, params);
    CATCH_REQUIRE(nullptr == queue);
    queue = pcqueue_init(&queue_in, params);
  } else {
    queue = pcqueue_init(&queue_in, params);
  }
  CATCH_REQUIRE(nullptr != queue);

  std::vector<T> vals;
  auto prod_cb = [&](auto* const q) {
                     size_t count = 0;
                     th::element_generator<T> g(gen_elt_type::ekINC_VALS,
                                                params->max_elts);
                     while (true) {
                       T e = g.next();

                       if (OK == pcqueue_push(q, &e)) {
                         ++count;
                       }
                       if (count >= TH_NUM_MT_ITEMS) {
                         break;
                       }
                     } /* for(i..) */
                 };
  T e;
  auto cons_cb = [&](auto* const q) {
                   for (size_t i = 0; i < TH_NUM_MT_ITEMS; ++i) {
                     CATCH_REQUIRE(OK == pcqueue_pop(q, &e));
                     vals.push_back(e);
                   } /* for(i..) */
                 };

  std::thread prod(prod_cb, queue);
  std::thread cons(cons_cb, queue);

  prod.join();
  cons.join();

  CATCH_REQUIRE(vals.size() == TH_NUM_MT_ITEMS);
  std::sort(vals.begin(), vals.end());
  CATCH_REQUIRE(pcqueue_isempty(queue));

  for (size_t i = 0; i < TH_NUM_MT_ITEMS; ++i) {
    CATCH_REQUIRE(vals[i].value1 == (decltype(T::value1))i);
  } /* for(i..) */

  pcqueue_destroy(queue);
}

template <typename T>
static void concurrent_test(const struct pcqueue_params* const params,
                     size_t n_prod,
                     size_t n_cons) {
  struct pcqueue queue_in;
  struct pcqueue* queue = pcqueue_init(&queue_in, params);
  CATCH_REQUIRE(nullptr != queue);

  auto prod_cb = [&](auto* const q) {
                   th::element_generator<T> g(gen_elt_type::ekINC_VALS,
                                              params->max_elts);
                   /*
                    * Advance so the first element put in the queue is 1 not
                    * 0.
                    */
                   g.next();

                   size_t count = 0;

                     while (true) {
                       T e = g.next();
                       if (OK == pcqueue_push(q, &e)) {
                         ++count;
                       }
                       if (count >= TH_NUM_MT_ITEMS) {
                         break;
                       }
                     } /* for(i..) */
                 };


  std::vector<status_t> cons_res;
  std::vector<T> pops;

  /* mutex can't be thread-local */
  std::mutex mtx;

  auto cons_cb = [&](auto* const q) {
                   T e;
                   size_t count = 0;
                   while (count < 10) {
                     struct timespec to = {.tv_sec = 0, .tv_nsec = 100000000};
                     status_t rval = pcqueue_timedpop(q, &to, &e);
                     if (OK == rval) {
                       std::scoped_lock lock(mtx);
                       pops.push_back(e);
                       cons_res.push_back(rval);
                     } else {
                       ++count;
                     }
                   } /* for(i..) */
                 };

  std::vector<std::thread> producers;
  std::vector<std::thread> consumers;

  for (size_t i = 0; i < n_prod; ++i) {
    producers.push_back(std::thread(prod_cb, queue));
  } /* for(i..) */

  for (size_t i = 0; i < n_cons; ++i) {
    consumers.push_back(std::thread(cons_cb, queue));
  } /* for(i..) */

  for (size_t i = 0; i < producers.size(); ++i) {
    producers[i].join();
  } /* for(i..) */

  for (size_t i = 0; i < consumers.size(); ++i) {
    consumers[i].join();
  } /* for(i..) */

  /* all items should have been retrieved */
  CATCH_REQUIRE(pcqueue_isempty(queue));
  CATCH_REQUIRE(std::all_of(cons_res.begin(),
                            cons_res.end(),
                            [](status_t res) { return res == OK; }));
  CATCH_REQUIRE(std::all_of(pops.begin(),
                            pops.end(),
                            [](const T& val) { return val.value1 != -1; }));

  /*
   * All consumers should have received a set of TH_NUM_MT_ITEMS elements which
   * are [0, TH_NUM_MT_ITEMS], so summing them using the standard n(n+1)/2 and
   * multiplying by the # producers checks this.
   */
  auto sum = std::accumulate(std::begin(pops),
                             std::end(pops),
                             0U,
                             [](auto& accum, const auto& val){
                               accum += val.value1;
                               return accum;
                             });
  CATCH_REQUIRE(sum == n_prod * (TH_NUM_MT_ITEMS * (TH_NUM_MT_ITEMS + 1))/ 2);
  pcqueue_destroy(queue);
}

template <typename T>
void timeout_test(const struct pcqueue_params* const params,
                 size_t n_prod,
                 size_t n_cons) {
  struct pcqueue queue_in;
  struct pcqueue* queue = pcqueue_init(&queue_in, params);
  CATCH_REQUIRE(nullptr != queue);

  th::element_generator<T> g(gen_elt_type::ekINC_VALS, params->max_elts);
  auto prod_cb = [&](auto* const q) {
                     size_t count = 0;

                     while (true) {
                       T e = g.next();
                       if (OK == pcqueue_push(q, &e)) {
                         ++count;
                       }
                       /*
                        * Purposely don't push enough items so that all
                        * consumers get the # they are trying to get.
                        */
                       if (count >= TH_NUM_MT_ITEMS) {
                         break;
                       }
                     } /* for(i..) */
                 };


  std::vector<status_t> cons_res(TH_NUM_MT_ITEMS * n_cons, ERROR);
  std::vector<T> pops(TH_NUM_MT_ITEMS * n_cons, g.sentinel());

  /* mutex can't be thread-local */
  std::mutex mtx;

  auto cons_cb = [&](auto* const q, size_t id) {
                   T e;
                   struct timespec to = {.tv_sec = 0, .tv_nsec = 1};
                   for (size_t i = 0; i < TH_NUM_MT_ITEMS; ++i) {
                     status_t rval = pcqueue_timedpop(q, &to, &e);
                     if (OK == rval) {
                       pops[id * TH_NUM_MT_ITEMS + i] = e;
                       cons_res[id * TH_NUM_MT_ITEMS + i] = rval;
                     }
                   } /* for(i..) */
                 };

  std::vector<std::thread> producers;
  std::vector<std::thread> consumers;

  for (size_t i = 0; i < n_prod; ++i) {
    producers.push_back(std::thread(prod_cb, queue));
  } /* for(i..) */

  for (size_t i = 0; i < n_cons; ++i) {
    consumers.push_back(std::thread(cons_cb, queue, i));
  } /* for(i..) */

  for (size_t i = 0; i < producers.size(); ++i) {
    producers[i].join();
  } /* for(i..) */

  for (size_t i = 0; i < consumers.size(); ++i) {
    consumers[i].join();
  } /* for(i..) */

  /* NOT all consumers should have retrieved all items */
  CATCH_REQUIRE(!std::all_of(cons_res.begin(),
                            cons_res.end(),
                            [](status_t res) { return res == OK; }));
  CATCH_REQUIRE(!std::all_of(pops.begin(),
                            pops.end(),
                            [](const T& val) { return val.value1 != -1; }));

  pcqueue_destroy(queue);
}

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("Serial Test", "[mt][pcqueue]") {
  run_test<element8>(serial_test<element8>);
  run_test<element4>(serial_test<element4>);
  /*
   * Don't test with element2 or element1 because the integers used are not
   * large enough to hold the full range of values put into the queue.
   */
}

CATCH_TEST_CASE("Concurrency Test", "[mt][pcqueue]") {
  /* 1 producer, multiple consumers */
  for (size_t i = 1; i <= 10; ++i) {
    run_test<element8>(concurrent_test<element8>, 1, i);
    run_test<element4>(concurrent_test<element4>, 1, i);
  } /* for(i..) */

  /* Multiple producers, 1 consumer */
  for (size_t i = 1; i <= 10; ++i) {
    run_test<element8>(concurrent_test<element8>, i, 1);
    run_test<element4>(concurrent_test<element4>, i, 1);
  } /* for(i..) */

  /* Multiple producers, multiple consumers */
  for (size_t i = 1; i <= 10; ++i) {
    for (size_t j = 1; j <= 10; ++j) {
      run_test<element8>(concurrent_test<element8>, i, j);
      run_test<element4>(concurrent_test<element4>, i, j);
    } /* for(j..) */
  } /* for(i..) */

  /*
   * Don't test with element2 or element1 because the integers used are not
   * large enough to hold the full range of values put into the queue.
   */
}

CATCH_TEST_CASE("Timeout Test", "[mt][pcqueue]") {
  run_test<element8>(timeout_test<element8>, 1, 10);
  run_test<element4>(timeout_test<element4>, 1, 10);
  /*
   * Don't test with element2 or element1 because the integers used are not
   * large enough to hold the full range of values put into the queue.
   */
}
