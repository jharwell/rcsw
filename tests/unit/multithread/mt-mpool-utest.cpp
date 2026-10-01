/**
 * \file
 *
 * \copyright 2023 John Harwell
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <mutex>
#include <random>
#include <thread>

#include <signal.h>
#include <unistd.h>

#define CATCH_CONFIG_PREFIX_ALL
#include <catch2/catch_test_macros.hpp>

#include "rcsw/er/client.h"
#include "rcsw/multithread/mpool.h"
#include "tests/unit/element.hpp"
#include "tests/unit/test.h"

/*******************************************************************************
 * Namespaces/Decls
 ******************************************************************************/
using mpool_test = void (*)(const struct mpool_config* const config,
                            size_t                           n_threads);
#define TH_NUM_MT_ITEMS 1000

/*******************************************************************************
 * Test Harness
 ******************************************************************************/
template <typename T>
static void run_test(mpool_test test, size_t n_threads = 1) {
  RCSW_ER_INIT(TH_ZLOG_CONF);
  RCSW_ER_INSMOD(LOG4CL_MT_MPOOL, "rcsw.mt.mpool");
  RCSW_ER_INSMOD(LOG4CL_DS_LLIST, "rcsw.ds.llist");
  /* log4cl_mod_lvl_set(LOG4CL_MT_MPOOL, RCSW_ERL_ALL); */
  /* log4cl_mod_lvl_set(LOG4CL_DS_LLIST, RCSW_ERL_ALL); */

  struct mpool_config config;
  config.flags    = 0;
  config.elt_size = sizeof(T);
  config.max_elts = TH_NUM_MT_ITEMS;
  config.meta     = (dptr_t*)malloc(mpool_meta_space(config.max_elts));
  config.elements =
    (dptr_t*)malloc(mpool_element_space(config.max_elts, config.elt_size));

  uint32_t flags[] = {
    RCSW_NONE,
    RCSW_NOALLOC_HANDLE,
    RCSW_NOALLOC_DATA,
    RCSW_NOALLOC_META,
  };

  for (size_t i = 0; i < RCSW_ARRAY_ELTS(flags); ++i) {
    config.flags = flags[i];
    test(&config, n_threads);
  }

  free(config.meta);
  free(config.elements);
  RCSW_ER_DEINIT();
} /* test_runner() */

static struct mpool* make_pool(size_t max_elts) {
  struct mpool_config c;
  memset(&c, 0, sizeof(c));
  c.elt_size = 16;
  c.max_elts = max_elts;
  return mpool_init(nullptr, &c);
}

template <typename T>
static void simple_test(const struct mpool_config* const config, size_t) {
  struct mpool  pool_in;
  struct mpool* pool;

  if (config->flags & RCSW_NOALLOC_HANDLE) {
    pool = mpool_init(NULL, config);
    CATCH_REQUIRE(nullptr == pool);
    pool = mpool_init(&pool_in, config);
  } else {
    pool = mpool_init(&pool_in, config);
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
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
static void concurrency_test(const struct mpool_config* const config,
                             size_t                           n_threads) {
  struct mpool  pool_in;
  struct mpool* pool;

  if (config->flags & RCSW_NOALLOC_HANDLE) {
    pool = mpool_init(NULL, config);
    CATCH_REQUIRE(nullptr == pool);
    pool = mpool_init(&pool_in, config);
  } else {
    pool = mpool_init(&pool_in, config);
  }
  CATCH_REQUIRE(nullptr != pool);

  CATCH_REQUIRE(mpool_isempty(pool));

  std::vector<bool> checks;
  std::mutex        mtx;
  auto              cb = [&](auto* const p, size_t id) {
    std::vector<T*>          vals;
    th::element_generator<T> g(th::gen_elt_type::INC_VALS, config->max_elts);
    struct timespec          to = {.tv_sec = 0, .tv_nsec = 1000};

    while (vals.size() < TH_NUM_MT_ITEMS) {
      T* e = nullptr;
      if (OK != mpool_timedreq(p, &to, (void**)&e)) {
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

      for (size_t i = 0; i < vals.size(); ++i) {
        if (nullptr != vals[i]) {
          /* refcount: 1 -> 3 -> 2 -> 4 -> 3 -> 1 -> released at 0 */
          bool ok = true;
          ok &= (OK == mpool_ref_add(pool, (uint8_t*)vals[i]));
          ok &= (OK == mpool_ref_add(pool, (uint8_t*)vals[i]));
          ok &= (OK == mpool_release(p, (uint8_t*)vals[i]));
          ok &= (OK == mpool_ref_add(pool, (uint8_t*)vals[i]));
          usleep(1);

          ok &= (OK == mpool_ref_add(pool, (uint8_t*)vals[i]));
          ok &= (OK == mpool_ref_remove(pool, (uint8_t*)vals[i]));
          usleep(1);

          ok &= (OK == mpool_ref_remove(pool, (uint8_t*)vals[i]));
          ok &= (OK == mpool_ref_remove(pool, (uint8_t*)vals[i]));
          usleep(1);

          ok &= (OK == mpool_release(p, (uint8_t*)vals[i]));
          mtx.lock();
          checks.push_back(ok);
          mtx.unlock();
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
  threads.reserve(n_threads);
  for (size_t i = 0; i < n_threads; ++i) {
    threads.emplace_back(std::thread(cb, pool, i * 10));
  } /* for(i..) */

  for (size_t i = 0; i < n_threads; ++i) {
    threads[i].join();
  } /* for(i..) */

  CATCH_REQUIRE(mpool_isempty(pool));
  CATCH_REQUIRE(
    std::all_of(checks.begin(), checks.end(), [](bool b) { return b; }));
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
CATCH_TEST_CASE(
  "mpool_release of an already-free chunk is rejected and does "
  "not corrupt the pool",
  "[multithread][mpool]") {
  struct mpool* p  = make_pool(2);
  void*         c0 = mpool_req(p);
  CATCH_REQUIRE(OK == mpool_release(p, c0));
  CATCH_REQUIRE(ERROR == mpool_release(p, c0));

  /* Exactly two chunks are obtainable, then the pool is empty */
  struct timespec to = {0, 20 * 1000 * 1000};
  void*           a  = nullptr;
  void*           b  = nullptr;
  void*           c  = nullptr;
  CATCH_REQUIRE(OK == mpool_timedreq(p, &to, &a));
  CATCH_REQUIRE(OK == mpool_timedreq(p, &to, &b));
  CATCH_REQUIRE(a != b);
  CATCH_REQUIRE(ERROR == mpool_timedreq(p, &to, &c));
  mpool_destroy(p);
}

CATCH_TEST_CASE("mpool_timedreq rejects a NULL chunk pointer without "
                "taking a chunk",
                "[multithread][mpool]") {
  struct mpool*   p  = make_pool(1);
  struct timespec to = {0, 20 * 1000 * 1000};
  CATCH_REQUIRE(ERROR == mpool_timedreq(p, &to, nullptr));

  /* The pool's only chunk is still free */
  CATCH_REQUIRE(mpool_isempty(p));
  void* a = nullptr;
  CATCH_REQUIRE(OK == mpool_timedreq(p, &to, &a));
  CATCH_REQUIRE(nullptr != a);
  CATCH_REQUIRE(OK == mpool_release(p, a));
  mpool_destroy(p);
}

CATCH_TEST_CASE("mpool_ref_query rejects pointers that are not chunk starts",
                "[multithread][mpool]") {
  struct mpool* p    = make_pool(4);
  auto*         base = reinterpret_cast<uint8_t*>(p->elements);
  CATCH_REQUIRE(0 == mpool_ref_query(p, base));
  CATCH_REQUIRE(-1 == mpool_ref_query(p, base + 1));      /* mid-chunk */
  CATCH_REQUIRE(-1 == mpool_ref_query(p, base + 16 * 4)); /* one past end */
  mpool_destroy(p);
}

CATCH_TEST_CASE("mpool_ref_count reports errors as SIZE_MAX",
                "[multithread][mpool]") {
  struct mpool* p = make_pool(4);
  int           not_from_pool;
  CATCH_REQUIRE(SIZE_MAX == mpool_ref_count(p, &not_from_pool));
  mpool_destroy(p);
}

namespace {
struct mpool* g_eintr_pool;
void          on_usr1(int) {}
void*         blocked_req(void*) {
  void* chunk = mpool_req(g_eintr_pool);
  return chunk;
}
} /* namespace */

CATCH_TEST_CASE("mpool_req survives EINTR while waiting for a chunk",
                "[multithread][mpool]") {
  struct sigaction sa;
  struct sigaction old_sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = on_usr1; /* no SA_RESTART: sem_wait returns EINTR */
  sigaction(SIGUSR1, &sa, &old_sa);

  g_eintr_pool = make_pool(1);
  void* held   = mpool_req(g_eintr_pool); /* pool now empty */

  pthread_t t;
  pthread_create(&t, nullptr, blocked_req, nullptr);
  usleep(100 * 1000);
  pthread_kill(t, SIGUSR1); /* interrupt the blocked wait */
  usleep(100 * 1000);
  CATCH_REQUIRE(OK == mpool_release(g_eintr_pool, held));

  void* got = nullptr;
  pthread_join(t, &got);
  CATCH_REQUIRE(got == held);

  sigaction(SIGUSR1, &old_sa, nullptr);
  CATCH_REQUIRE(OK == mpool_release(g_eintr_pool, got));
  mpool_destroy(g_eintr_pool);
}
