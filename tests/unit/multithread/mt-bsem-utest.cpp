/**
 * \file
 *
 * \copyright 2026 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * Tests for multithread.
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#define CATCH_CONFIG_PREFIX_ALL
#include <ctime>

#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#include <catch2/catch_test_macros.hpp>

#include "rcsw/core/flags.h"
#include "rcsw/multithread/bsem.h"

/*******************************************************************************
 * bsem
 ******************************************************************************/
CATCH_TEST_CASE("bsem_post on an available semaphore is a no-op",
                "[multithread][bsem][noalloc]") {
  struct bsem s;
  CATCH_REQUIRE(nullptr != bsem_init(&s, RCSW_NOALLOC_HANDLE));
  CATCH_REQUIRE(OK == bsem_post(&s));
  CATCH_REQUIRE(OK == bsem_wait(&s)); /* still exactly one unit available */
  struct timespec to = {0, 10 * 1000 * 1000};
  CATCH_REQUIRE(ERROR == bsem_timedwait(&s, &to));
  bsem_destroy(&s);
}

namespace {
struct bsem g_flush_sem;
void*       flush_waiter(void*) {
  bsem_wait(&g_flush_sem);
  return nullptr;
}
} /* namespace */

CATCH_TEST_CASE("bsem_flush wakes every waiter", "[multithread][bsem][noalloc]") {
  CATCH_REQUIRE(nullptr != bsem_init(&g_flush_sem, RCSW_NOALLOC_HANDLE));
  CATCH_REQUIRE(OK == bsem_wait(&g_flush_sem)); /* take the initial unit */
  pthread_t t[3];
  for (auto& th : t) {
    pthread_create(&th, nullptr, flush_waiter, nullptr);
  }
  usleep(100 * 1000);
  CATCH_REQUIRE(OK == bsem_flush(&g_flush_sem));
  for (auto& th : t) {
    pthread_join(th, nullptr); /* every waiter must have been released */
  }
  bsem_destroy(&g_flush_sem);
}
