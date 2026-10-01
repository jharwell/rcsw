/**
 * \file
 *
 * \copyright 2026 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * Tests for grind. A fake
 * clock makes the timing tests deterministic.
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#define CATCH_CONFIG_PREFIX_ALL
#include <cerrno>
#include <cmath>
#include <cstring>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "rcsw/tool/grind.h"

/*******************************************************************************
 * Helpers
 ******************************************************************************/
namespace {
struct timespec g_now = {1000, 0};

struct timespec fake_clock(void) { return g_now; }

void advance_ns(long ns) {
  g_now.tv_nsec += ns;
  g_now.tv_sec += g_now.tv_nsec / 1000000000L;
  g_now.tv_nsec %= 1000000000L;
}

char  g_name[]  = "a";
char* g_names[] = {g_name};

struct grind_config base_config(enum grind_mode mode, uint32_t flags) {
  struct grind_config c;
  memset(&c, 0, sizeof(c));
  c.names   = g_names;
  c.n_inst  = 1;
  c.mode    = mode;
  c.res     = 1;
  c.tsize   = 4;
  c.flags   = flags;
  c.gettime = fake_clock;
  return c;
}
} /* namespace */

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("grind_init initializes all interval state", "[tool][grind]") {
  /* A caller-provided handle full of garbage must behave like a fresh one */
  struct grinder handle;
  memset(&handle, 0xFF, sizeof(handle));
  struct grind_config c = base_config(
    RCSW_GRIND_MODE_COUNT,
    RCSW_GRIND_INTERVAL | RCSW_GRIND_RESET_AUTO | RCSW_NOALLOC_HANDLE);
  c.interval = {10, 0};

  struct grinder* g = grind_init(&handle, &c);
  CATCH_REQUIRE(nullptr != g);
  for (int i = 0; i < 4; ++i) {
    CATCH_REQUIRE(OK == grind_capture_count(g, "a"));
  }
  CATCH_REQUIRE(g->grindees[0].full);
  grind_destroy(g);
}

CATCH_TEST_CASE(
  "grind_get_utilization uses the full interval, including "
  "nanoseconds",
  "[tool][grind]") {
  /* zeroed handle: isolates this from the uninitialized-state defect */
  struct grinder handle;
  memset(&handle, 0, sizeof(handle));
  struct grind_config c =
    base_config(RCSW_GRIND_MODE_DURATION,
                RCSW_GRIND_INTERVAL | RCSW_NOALLOC_HANDLE);
  c.interval        = {0, 500000000}; /* 0.5 s */
  struct grinder* g = grind_init(&handle, &c);
  CATCH_REQUIRE(nullptr != g);

  CATCH_REQUIRE(OK == grind_capture_start(g, "a"));
  advance_ns(100000000); /* 100 ms */
  CATCH_REQUIRE(OK == grind_capture_end(g, "a"));

  /* 100 ms of a 500 ms interval */
  CATCH_REQUIRE(std::fabs(grind_get_utilization(g, "a") - 20.0) < 1e-6);
  grind_destroy(g);
}

CATCH_TEST_CASE(
  "RCSW_GRIND_RESET_AUTO without RCSW_GRIND_INTERVAL resets a "
  "full grindee on the next capture",
  "[tool][grind]") {
  struct grind_config c =
    base_config(RCSW_GRIND_MODE_COUNT, RCSW_GRIND_RESET_AUTO);
  c.tsize               = 2;
  struct grinder* g     = grind_init(nullptr, &c);
  CATCH_REQUIRE(nullptr != g);
  CATCH_REQUIRE(OK == grind_capture_count(g, "a"));
  CATCH_REQUIRE(OK == grind_capture_count(g, "a"));
  CATCH_REQUIRE(g->grindees[0].full);
  CATCH_REQUIRE(OK == grind_capture_count(g, "a")); /* resets, then counts */
  grind_destroy(g);
}

CATCH_TEST_CASE("grind_report_utilization_buf has snprintf semantics",
                "[tool][grind]") {
  struct grind_config c = base_config(RCSW_GRIND_MODE_DURATION, RCSW_NONE);
  struct grinder*     g = grind_init(nullptr, &c);
  CATCH_REQUIRE(nullptr != g);
  CATCH_REQUIRE(OK == grind_capture_start(g, "a"));
  advance_ns(1000);
  CATCH_REQUIRE(OK == grind_capture_end(g, "a"));

  /* Size query */
  int needed = grind_report_utilization_buf(g, nullptr, 0);
  CATCH_REQUIRE(needed > 0);

  /* Exactly enough room: full report, NUL-terminated */
  std::vector<char> full(static_cast<size_t>(needed) + 1, 'z');
  CATCH_REQUIRE(needed ==
                grind_report_utilization_buf(g, full.data(), full.size()));
  CATCH_REQUIRE(static_cast<size_t>(needed) == strlen(full.data()));
  CATCH_REQUIRE(nullptr != strstr(full.data(), "100.00%"));
  CATCH_REQUIRE(nullptr != strchr(full.data(), '\n'));

  /* Too small: truncated but terminated, never overrun (heap: ASan checks) */
  std::vector<char> small(10, 'z');
  CATCH_REQUIRE(needed ==
                grind_report_utilization_buf(g, small.data(), small.size()));
  CATCH_REQUIRE(9 == strlen(small.data()));
  CATCH_REQUIRE(0 == strncmp(small.data(), full.data(), 9));
  grind_destroy(g);
}

CATCH_TEST_CASE("grind keeps nanosecond totals beyond 32 bits", "[tool][grind]") {
  struct grind_config c = base_config(RCSW_GRIND_MODE_DURATION, RCSW_NONE);
  struct grinder*     g = grind_init(nullptr, &c);
  CATCH_REQUIRE(nullptr != g);
  /* 3 x 5 s = 15e9 ns > UINT32_MAX */
  for (int i = 0; i < 3; ++i) {
    CATCH_REQUIRE(OK == grind_capture_start(g, "a"));
    g_now.tv_sec += 5;
    CATCH_REQUIRE(OK == grind_capture_end(g, "a"));
  }
  CATCH_REQUIRE(UINT64_C(15000000000) == grind_sum_all(g));
  grind_destroy(g);
}

CATCH_TEST_CASE("grind captures on an unknown grindee are ENOENT",
                "[tool][grind]") {
  struct grind_config c = base_config(RCSW_GRIND_MODE_COUNT, RCSW_NONE);
  struct grinder*     g = grind_init(nullptr, &c);
  CATCH_REQUIRE(nullptr != g);
  errno = 0;
  CATCH_REQUIRE(ERROR == grind_capture_count(g, "nope"));
  CATCH_REQUIRE(ENOENT == errno);
  grind_destroy(g);
}
