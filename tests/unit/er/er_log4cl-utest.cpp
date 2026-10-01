/**
 * \file
 *
 * \copyright 2026 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * Tests for the LOG4CL plugin.
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#define CATCH_CONFIG_PREFIX_ALL
#include <cerrno>
#include <thread>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "rcsw/er/er.h"
#include "rcsw/er/plugin/log4cl.h"

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("log4cl_insmod rejects a NULL module name", "[er][log4cl]") {
  CATCH_REQUIRE(OK == log4cl_init());
  CATCH_REQUIRE(ERROR == log4cl_insmod(0x1234, nullptr));
  log4cl_deinit();
}

CATCH_TEST_CASE("log4cl_insmod is idempotent", "[er][log4cl]") {
  CATCH_REQUIRE(OK == log4cl_init());
  CATCH_REQUIRE(OK == log4cl_insmod(0x5678, "a"));
  CATCH_REQUIRE(OK == log4cl_insmod(0x5678, "a"));
  CATCH_REQUIRE(nullptr != log4cl_mod_query(0x5678));
  CATCH_REQUIRE(OK == log4cl_rmmod(0x5678));
  CATCH_REQUIRE(nullptr == log4cl_mod_query(0x5678));
  log4cl_deinit();
}

CATCH_TEST_CASE("log4cl operations on a missing module are ENOENT",
                "[er][log4cl]") {
  CATCH_REQUIRE(OK == log4cl_init());
  errno = 0;
  CATCH_REQUIRE(ERROR == log4cl_rmmod(0x9999));
  CATCH_REQUIRE(ENOENT == errno);
  errno = 0;
  CATCH_REQUIRE(ERROR == log4cl_rmmod2("nope"));
  CATCH_REQUIRE(ENOENT == errno);
  errno = 0;
  CATCH_REQUIRE(ERROR == log4cl_mod_lvl_set(0x9999, RCSW_ERL_DEBUG));
  CATCH_REQUIRE(ENOENT == errno);
  CATCH_REQUIRE(-1 == log4cl_mod_id_get("nope"));
  log4cl_deinit();
}

CATCH_TEST_CASE("log4cl_rmmod2 removes by name", "[er][log4cl]") {
  CATCH_REQUIRE(OK == log4cl_init());
  CATCH_REQUIRE(OK == log4cl_insmod(0x77, "by.name"));
  CATCH_REQUIRE(0x77 == log4cl_mod_id_get("by.name"));
  CATCH_REQUIRE(OK == log4cl_rmmod2("by.name"));
  CATCH_REQUIRE(nullptr == log4cl_mod_query(0x77));
  log4cl_deinit();
}

CATCH_TEST_CASE("log4cl tolerates concurrent insmod and lookups",
                "[er][log4cl]") {
  CATCH_REQUIRE(OK == log4cl_init());
  constexpr int            kThreads = 8;
  constexpr int            kMods    = 200;
  std::vector<std::thread> threads;
  for (int t = 0; t < kThreads; ++t) {
    threads.emplace_back([t] {
      for (int i = 0; i < kMods; ++i) {
        int64_t id = (int64_t)t * kMods + i + 1000;
        (void)log4cl_insmod(id, "mt");
        (void)log4cl_mod_query(id);
        (void)log4cl_mod_query(1000); /* shared lookups */
      }
    });
  }
  for (auto& th : threads) {
    th.join();
  }
  for (int64_t id = 1000; id < 1000 + kThreads * kMods; ++id) {
    CATCH_REQUIRE(nullptr != log4cl_mod_query(id));
  }
  log4cl_deinit();
}
