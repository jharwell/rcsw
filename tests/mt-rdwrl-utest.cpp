/**
 * \file mt-rdwrl-utest.cpp
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
#include <atomic>

#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_PREFIX_ALL
#include <catch.hpp>

#include "rcsw/multithread/rdwrlock.h"

#include "tests/ds_test.hpp"

/*******************************************************************************
 * Namespaces/Decls
 ******************************************************************************/
using rdwrl_test = void(*)(uint32_t flags,
                           size_t n_threads);

/*******************************************************************************
 * Test Helper Functions
 ******************************************************************************/
static void run_test(rdwrl_test test, size_t n_threads = 1) {
  uint32_t flags[] = {
    RCSW_NONE,
    RCSW_NOALLOC_HANDLE,
  };

  for (size_t i = 0; i < RCSW_ARRAY_ELTS(flags); ++i) {
    test(flags[i], n_threads);
  }
} /* test_runner() */

/*******************************************************************************
 * Test Functions
 ******************************************************************************/
static void simple_test(uint32_t flags, size_t) {
  struct rdwrlock lock_in;
  struct rdwrlock *lock;

  if (flags & RCSW_NOALLOC_HANDLE) {
    lock = rdwrl_init(NULL, flags);
    CATCH_REQUIRE(nullptr == lock);
    lock = rdwrl_init(&lock_in, flags);
  } else {
    lock = rdwrl_init(&lock_in, flags);
  }
  CATCH_REQUIRE(nullptr != lock);

  rdwrl_req(lock, ekSCOPE_RD);
  CATCH_REQUIRE(lock->n_readers == 1);
  rdwrl_exit(lock, ekSCOPE_RD);

  rdwrl_req(lock, ekSCOPE_WR);
  rdwrl_exit(lock, ekSCOPE_WR);

  rdwrl_req(lock, ekSCOPE_RD);
  rdwrl_req(lock, ekSCOPE_RD);
  rdwrl_req(lock, ekSCOPE_RD);
  CATCH_REQUIRE(lock->n_readers == 3);

  rdwrl_exit(lock, ekSCOPE_RD);
  rdwrl_exit(lock, ekSCOPE_RD);
  CATCH_REQUIRE(lock->n_readers == 1);
  rdwrl_exit(lock, ekSCOPE_RD);

  rdwrl_req(lock, ekSCOPE_WR);
  rdwrl_exit(lock, ekSCOPE_WR);

  rdwrl_destroy(lock);
}

static void concurrency_test(uint32_t flags, size_t n_threads) {
  struct rdwrlock lock_in;
  struct rdwrlock *lock;

  lock = rdwrl_init(&lock_in, flags);
  CATCH_REQUIRE(nullptr != lock);

  std::vector<bool> checks;
  std::mutex mtx;
  std::vector<size_t> vals(100, -1);

  std::atomic_size_t n_readers = 0;
  std::atomic_size_t n_writers = 0;

  auto cb = [&](size_t id) {
              struct timespec to = {.tv_sec = 0, .tv_nsec = 1000000};
              while (n_readers < 1000 || n_writers < 1000) {
                /* randomly choose to be a reader/writer */
                bool is_writer = rand() % 2;

                /* randomly choose to use a timeout or not */
                bool use_to = rand() % 2;

                if (is_writer) {
                  if (use_to) {
                    if (OK != rdwrl_timedreq(lock, ekSCOPE_WR, &to)) {
                      continue;
                    }
                  } else {
                    rdwrl_req(lock, ekSCOPE_WR);
                  }

                  /*
                   * Write all values with this thread's unique ID, verifying as
                   * we go that nothing else has come in and overwritten what
                   * we've done.
                   */
                  for (size_t i = 0; i < vals.size(); ++i) {
                    vals[i] = id;
                    for (size_t j = 0; j < i; ++j) {
                      mtx.lock();
                      checks.push_back(vals[j] == id);
                      mtx.unlock();
                    } /* for(j..) */
                  } /* for(i..) */

                  rdwrl_exit(lock, ekSCOPE_WR);
                  ++n_writers;
                } else {
                  if (use_to) {
                    if (OK != rdwrl_timedreq(lock, ekSCOPE_RD, &to)) {
                      continue;
                    }
                  } else {
                    rdwrl_req(lock, ekSCOPE_RD);
                  }

                  mtx.lock();
                  /* if a reader, check that all values are the same */
                  checks.push_back(std::all_of(std::begin(vals),
                                               std::end(vals),
                                               [&](auto& var) {
                                                 return var == vals[0];
                                               }));
                  mtx.unlock();
                  rdwrl_exit(lock, ekSCOPE_RD);
                  ++n_readers;
                }
              } /* while() */
            };

  std::vector<std::thread> threads;
  for (size_t i = 0; i < n_threads; ++i) {
    threads.push_back(std::thread(cb, i));
  } /* for(i..) */

  for (size_t i = 0; i < n_threads; ++i) {
    threads[i].join();
  } /* for(i..) */
  printf("Total writers: %zu total readers: %zu\n",
         n_writers.load(),
         n_readers.load());

  CATCH_REQUIRE(std::all_of(checks.begin(),
                            checks.end(),
                            [](bool b){ return b; }
                            ));
  rdwrl_destroy(lock);
}


/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("Simple Test", "[mt][rdwrlock]") {
  run_test(simple_test);
}

CATCH_TEST_CASE("Concurrency Test", "[mt][rdwrlock]") {
  for (size_t i = 1; i <= 10; ++i) {
    run_test(concurrency_test, i);
  } /* for(i..) */
}
