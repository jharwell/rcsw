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
#include <limits.h>
#include <pthread.h>
#include <semaphore.h>

#include <catch2/catch_test_macros.hpp>

#include "rcsw/multithread/csem.h"

/*******************************************************************************
 * Test Harness
 ******************************************************************************/
CATCH_TEST_CASE("csem_init failure does not leak the handle",
                "[multithread][csem]") {
  /* Detected by LeakSanitizer at process exit (ASAN_OPTIONS=detect_leaks=1) */
  CATCH_REQUIRE(nullptr ==
                csem_init(nullptr, static_cast<size_t>(SEM_VALUE_MAX) + 1, 0));
}
