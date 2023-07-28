/**
 * \file mt-mpool-utest.cpp
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

#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_PREFIX_ALL
#include <catch.hpp>

#include "rcsw/multithread/mpool.h"

#include "tests/ds_test.hpp"

/*******************************************************************************
 * Namespaces/Decls
 ******************************************************************************/
using mpool_test = void(*)(const struct mpool_params* const params,
                           size_t n_threads);
#define TH_NUM_MT_ITEMS 1000

/*******************************************************************************
 * Test Helper Functions
 ******************************************************************************/
template<typename T>
static void run_test(mpool_test test, size_t n_threads = 1) {
  struct mpool_params params;
  params.flags = 0;
  params.elt_size = sizeof(T);
  params.max_elts = TH_NUM_MT_ITEMS;
  params.meta = (uint8_t*)malloc(mpool_meta_space(params.max_elts));
  params.elements = (uint8_t*)malloc(mpool_element_space(params.max_elts,
                                                         params.elt_size));

  uint32_t flags[] = {
    RCSW_NONE,
    RCSW_NOALLOC_HANDLE,
    RCSW_NOALLOC_DATA,
    RCSW_NOALLOC_META,
  };

  for (size_t i = 0; i < RCSW_ARRAY_ELTS(flags); ++i) {
    params.flags = flags[i];
    test(&params, n_threads);
  }

  free(params.meta);
  free(params.elements);
} /* test_runner() */

/*******************************************************************************
 * Test Functions
 ******************************************************************************/
template <typename T>
static void simple_test(const struct mpool_params* const params,
                 size_t) {
  struct mpool pool_in;
  struct mpool *pool;

  if (params->flags & RCSW_NOALLOC_HANDLE) {
    pool = mpool_init(NULL, params);
    CATCH_REQUIRE(nullptr == pool);
    pool = mpool_init(&pool_in, params);
  } else {
    pool = mpool_init(&pool_in, params);
  }
  CATCH_REQUIRE(nullptr != pool);
  CATCH_REQUIRE(mpool_isempty(pool));

  std::vector<T*> vals;
  for (size_t i = 0; i < mpool_capacity(pool); ++i) {
    T* e = (T*)mpool_req(pool);
    CATCH_REQUIRE(nullptr != e);
    vals.push_back(e);
    CATCH_REQUIRE(mpool_ref_count(pool, (uint8_t*)e) == 1);
  } /* for(i..) */
  CATCH_REQUIRE(mpool_isfull(pool));

  for (size_t i = 0; i < mpool_capacity(pool); ++i) {
    CATCH_REQUIRE(OK == mpool_release(pool, (uint8_t*)vals[i]));
    CATCH_REQUIRE(mpool_ref_count(pool, (uint8_t*)vals[i]) == 0);
  } /* for(i..) */

  mpool_destroy(pool);
}
template <typename T>
static void concurrency_test(const struct mpool_params* const params,
                 size_t n_threads) {
  struct mpool pool_in;
  struct mpool *pool;

  if (params->flags & RCSW_NOALLOC_HANDLE) {
    pool = mpool_init(NULL, params);
    CATCH_REQUIRE(nullptr == pool);
    pool = mpool_init(&pool_in, params);
  } else {
    pool = mpool_init(&pool_in, params);
  }
  CATCH_REQUIRE(nullptr != pool);


  CATCH_REQUIRE(mpool_isempty(pool));

  std::vector<bool> checks;
  std::mutex mtx;
  auto cb = [&](auto* const p, size_t id) {
              std::vector<T*> vals;
              th::element_generator<T> g(gen_elt_type::ekINC_VALS,
                                         params->max_elts);
              struct timespec to = {.tv_sec = 0, .tv_nsec = 1000};

              while (vals.size() < TH_NUM_MT_ITEMS) {
                T* e = nullptr;
                if (OK != mpool_timedreq(p, &to, (uint8_t**)&e)) {
                  continue;
                }
                mtx.lock();
                checks.push_back(nullptr != e);
                mtx.unlock();

                vals.push_back(e);
                *e = g.next();
                e->value1 *= id;

                for (size_t j = 0; j < vals.size(); ++j) {
                  if (nullptr != vals[j]) {
                    mtx.lock();
                    checks.push_back(vals[j]->value1 == (decltype(T::value1))(j * id));
                    mtx.unlock();
                  }
                } /* for(j..) */

                /*
                 * Release a random # of chunks and verify that everything else
                 * is OK.
                 */
                size_t idx = vals.size() % (rand() + 1);
                for (size_t i = 0; i < idx; ++i) {
                  if (nullptr != vals[i]) {
                    mpool_ref_add(pool, (uint8_t*)vals[i]);
                    mpool_ref_add(pool, (uint8_t*)vals[i]);
                    mpool_release(p, (uint8_t*)vals[i]);
                    mpool_ref_add(pool, (uint8_t*)vals[i]);
                    usleep(1);

                    mpool_ref_add(pool, (uint8_t*)vals[i]);
                    mpool_ref_remove(pool, (uint8_t*)vals[i]);
                    usleep(1);

                    mpool_ref_remove(pool, (uint8_t*)vals[i]);
                    mpool_ref_remove(pool, (uint8_t*)vals[i]);
                    usleep(1);

                    mpool_release(p, (uint8_t*)vals[i]);
                    vals[i] = nullptr;

                    for (size_t j = 0; j < vals.size(); ++j) {
                      if (nullptr != vals[j]) {
                        mtx.lock();
                        checks.push_back(vals[j]->value1 == (decltype(T::value1))(j * id));
                        mtx.unlock();
                      }
                    } /* for(j..) */
                  }
                } /* for(i..) */
              } /* while() */
            };

  std::vector<std::thread> threads;
  for (size_t i = 0; i < n_threads; ++i) {
    threads.push_back(std::thread(cb, pool, i * 10));
  } /* for(i..) */

  for (size_t i = 0; i < n_threads; ++i) {
    threads[i].join();
  } /* for(i..) */

  CATCH_REQUIRE(mpool_isempty(pool));
  CATCH_REQUIRE(std::all_of(checks.begin(),
                            checks.end(),
                            [](bool b){ return b; }
                            ));
  mpool_destroy(pool);
}


/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("Simple Test", "[mt][mpool]") {
  run_test<element8>(simple_test<element8>);
  run_test<element4>(simple_test<element4>);
  /*
   * Don't test with element2 or element1 because the integers used are not
   * large enough to hold the full range of values put into the pool.
   */
}
CATCH_TEST_CASE("Concurrency Test", "[mt][mpool]") {
  for (size_t i = 1; i <= 10; ++i) {
    run_test<element8>(concurrency_test<element8>, i);
    run_test<element4>(concurrency_test<element4>, i);  
  } /* for(i..) */

  /*
   * Don't test with element2 or element1 because the integers used are not
   * large enough to hold the full range of values put into the pool.
   */
}
