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
#include <cstring>
#include <thread>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "rcsw/ds/darray.h"
#include "rcsw/er/er.h"
#include "rcsw/er/plugin/log4cl.h"
#include "rcsw/multithread/rdwrlock.h"

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
  /* no longer installed: reports use the default module */
  CATCH_REQUIRE(-1 == log4cl_mod_query(0x5678)->id);
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
  /* no longer installed: reports use the default module */
  CATCH_REQUIRE(-1 == log4cl_mod_query(0x77)->id);
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

CATCH_TEST_CASE("log4cl_insmod before log4cl_init is EAGAIN", "[er][log4cl]") {
  /* library modules may be initialized before the plugin is */
  errno = 0;
  CATCH_REQUIRE(ERROR == log4cl_insmod(0x4242, "early"));
  CATCH_REQUIRE(EAGAIN == errno);
  CATCH_REQUIRE(nullptr == log4cl_mod_query(0x4242));

  CATCH_REQUIRE(OK == log4cl_init());
  CATCH_REQUIRE(OK == log4cl_insmod(0x4242, "early"));
  CATCH_REQUIRE(0x4242 == log4cl_mod_query(0x4242)->id);
  log4cl_deinit();
}

CATCH_TEST_CASE("log4cl filters modules that are not installed by the default "
                "level",
                "[er][log4cl]") {
  CATCH_REQUIRE(OK == log4cl_init());
  struct log4cl_module* mod = log4cl_mod_query(0x31337);
  CATCH_REQUIRE(nullptr != mod);
  CATCH_REQUIRE(-1 == mod->id);

  log4cl_default_lvl_set(RCSW_ERL_ERROR);
  CATCH_REQUIRE(log4cl_mod_emit(mod, RCSW_ERL_ERROR));
  CATCH_REQUIRE_FALSE(log4cl_mod_emit(mod, RCSW_ERL_WARN));

  log4cl_default_lvl_set(RCSW_ERL_FATAL);
  CATCH_REQUIRE_FALSE(log4cl_mod_emit(log4cl_mod_query(0x31337),
                                      RCSW_ERL_ERROR));
  CATCH_REQUIRE(log4cl_mod_emit(log4cl_mod_query(0x31337), RCSW_ERL_FATAL));
  log4cl_deinit();
}

CATCH_TEST_CASE("log4cl can be shut down and initialized again",
                "[er][log4cl]") {
  CATCH_REQUIRE(OK == log4cl_init());
  CATCH_REQUIRE(OK == log4cl_insmod(0x99, "x"));
  log4cl_deinit();
  CATCH_REQUIRE(nullptr == log4cl_mod_query(0x99)); /* not initialized */

  CATCH_REQUIRE(OK == log4cl_init());
  CATCH_REQUIRE(-1 == log4cl_mod_query(0x99)->id); /* modules were dropped */
  CATCH_REQUIRE(OK == log4cl_insmod(0x99, "x"));
  CATCH_REQUIRE(0x99 == log4cl_mod_query(0x99)->id);
  log4cl_deinit();
}

#if RCSW_CONFIG_ER_PLUGIN == RCSW_ER_PLUGIN_LOG4CL && \
    RCSW_ERL >= RCSW_ERL_ERROR
CATCH_TEST_CASE("library modules register with log4cl when initialized",
                "[er][log4cl]") {
  /* a data structure created before the plugin is up still works */
  struct darray_config dc;
  memset(&dc, 0, sizeof(dc));
  dc.elt_size        = sizeof(int);
  dc.max_elts        = 1;
  dc.init_size       = 1;
  struct darray* pre = darray_init(nullptr, &dc);
  CATCH_REQUIRE(nullptr != pre);

  CATCH_REQUIRE(OK == log4cl_init());
  struct darray*   arr = darray_init(nullptr, &dc);
  struct rdwrlock* rw  = rdwrl_init(nullptr, RCSW_NONE);
  CATCH_REQUIRE(nullptr != arr);
  CATCH_REQUIRE(nullptr != rw);

  /* both modules are now installed, so their levels can be changed */
  CATCH_REQUIRE(OK == log4cl_mod_lvl_set(LOG4CL_DS_DARRAY, RCSW_ERL_NONE));
  CATCH_REQUIRE(OK == log4cl_mod_lvl_set(LOG4CL_MT_RDWRLOCK, RCSW_ERL_INFO));
  CATCH_REQUIRE_FALSE(
    log4cl_mod_emit(log4cl_mod_query(LOG4CL_DS_DARRAY), RCSW_ERL_ERROR));

  rdwrl_destroy(rw);
  darray_destroy(arr);
  darray_destroy(pre);
  log4cl_deinit();
}
#endif
