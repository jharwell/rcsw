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
#include <cstring>
#include <ctime>

#include <pthread.h>
#include <semaphore.h>

#include <catch2/catch_test_macros.hpp>

#include "rcsw/core/flags.h"
#include "rcsw/multithread/cvm.h"

/*******************************************************************************
 * Test Harness
 ******************************************************************************/
static double now_sec() {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return static_cast<double>(ts.tv_sec) + static_cast<double>(ts.tv_nsec) * 1e-9;
}

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("cvm_init succeeds with a caller-provided handle",
                "[multithread][cvm][noalloc]") {
  struct cvm  c;
  struct cvm* p = cvm_init(&c, RCSW_NOALLOC_HANDLE);
  CATCH_REQUIRE(p == &c);
  cvm_destroy(p);
}

CATCH_TEST_CASE("cvm_init succeeds with a library-allocated handle",
                "[multithread][cvm]") {
  struct cvm* p = cvm_init(nullptr, RCSW_NONE);
  CATCH_REQUIRE(nullptr != p);
  cvm_destroy(p);
}

CATCH_TEST_CASE("cvm_timedwait times out after the RELATIVE timeout",
                "[multithread][cvm][noalloc]") {
  /* The timeout is relative: the wait must end ~50 ms after it starts */
  struct cvm c;
  memset(&c, 0, sizeof(c));
  c.flags = RCSW_NOALLOC_HANDLE;
  CATCH_REQUIRE(nullptr != mutex_init(&c.mtx, RCSW_NOALLOC_HANDLE));
  CATCH_REQUIRE(nullptr != condv_init(&c.cv, RCSW_NOALLOC_HANDLE));

  struct timespec to = {0, 50 * 1000 * 1000}; /* 50 ms */
  mutex_lock(&c.mtx);
  double   start = now_sec();
  status_t rc    = cvm_timedwait(&c, &to);
  double   dt    = now_sec() - start;
  mutex_unlock(&c.mtx);

  CATCH_REQUIRE(ERROR == rc);
  CATCH_REQUIRE(dt >= 0.04);
  CATCH_REQUIRE(dt < 1.0);
  condv_destroy(&c.cv);
  mutex_destroy(&c.mtx);
}
