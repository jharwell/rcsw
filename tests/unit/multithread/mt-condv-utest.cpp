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
#include <cerrno>
#include <ctime>

#include <pthread.h>
#include <semaphore.h>

#include <catch2/catch_test_macros.hpp>

#include "rcsw/core/flags.h"
#include "rcsw/multithread/condv.h"
#include "rcsw/multithread/mutex.h"

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("condv_timedwait reports a timeout as ETIMEDOUT",
                "[multithread][condv][noalloc]") {
  struct condv cv;
  struct mutex m;
  CATCH_REQUIRE(nullptr != condv_init(&cv, RCSW_NOALLOC_HANDLE));
  CATCH_REQUIRE(nullptr != mutex_init(&m, RCSW_NOALLOC_HANDLE));
  mutex_lock(&m);
  struct timespec to = {0, 1000 * 1000};
  errno              = 0;
  CATCH_REQUIRE(ERROR == condv_timedwait(&cv, &m, &to));
  CATCH_REQUIRE(ETIMEDOUT == errno);
  mutex_unlock(&m);
  condv_destroy(&cv);
  mutex_destroy(&m);
}
