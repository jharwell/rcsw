/**
 * \file utils_time-utest.cpp
 *
 * Unit tests for rcsw/utils/time.h: utils_ts_cmp(), utils_ts_add(),
 * utils_ts_diff(), utils_ts2mono(), utils_ts2monons(), utils_monons2ts(),
 * utils_ts_make_abs(), and utils_ts_make_rel().
 *
 * \copyright 2023 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#define CATCH_CONFIG_PREFIX_ALL
#include <catch2/catch_test_macros.hpp>

#include <unistd.h>

#include "rcsw/utils/time.h"
#include "rcsw/al/clock.h"

/*******************************************************************************
 * Test Helper Functions
 ******************************************************************************/

static void ts_cmp_test() {
  struct timespec t1 = {.tv_sec = 1, .tv_nsec = 100};
  struct timespec t2 = {.tv_sec = 5, .tv_nsec = 800};
  struct timespec t3 = {.tv_sec = 1, .tv_nsec = 200}; /* same sec, later nsec */

  CATCH_REQUIRE(utils_ts_cmp(&t1, &t2) < 0);
  CATCH_REQUIRE(utils_ts_cmp(&t2, &t1) > 0);
  CATCH_REQUIRE(utils_ts_cmp(&t2, &t2) == 0);

  /* Same seconds, different nanoseconds */
  CATCH_REQUIRE(utils_ts_cmp(&t1, &t3) < 0);
  CATCH_REQUIRE(utils_ts_cmp(&t3, &t1) > 0);
  CATCH_REQUIRE(utils_ts_cmp(&t1, &t1) == 0);
}

static void ts_add_test() {
  struct timespec sum = {.tv_sec = 1, .tv_nsec = 100};
  struct timespec val = {.tv_sec = 1, .tv_nsec = 100};

  /* Simple addition without nanosecond overflow */
  utils_ts_add(&sum, &val);
  CATCH_REQUIRE(sum.tv_sec  == 2);
  CATCH_REQUIRE(sum.tv_nsec == 200);

  /* Nanosecond overflow: tv_nsec wraps and tv_sec is incremented */
  struct timespec a = {.tv_sec = 1, .tv_nsec = 999999999L};
  struct timespec b = {.tv_sec = 0, .tv_nsec = 2};
  utils_ts_add(&a, &b);
  CATCH_REQUIRE(a.tv_sec  == 2);
  CATCH_REQUIRE(a.tv_nsec == 1);

  /* Adding zero has no effect */
  struct timespec c   = {.tv_sec = 7, .tv_nsec = 42};
  struct timespec zero = {.tv_sec = 0, .tv_nsec = 0};
  utils_ts_add(&c, &zero);
  CATCH_REQUIRE(c.tv_sec  == 7);
  CATCH_REQUIRE(c.tv_nsec == 42);
}

static void ts_diff_test() {
  struct timespec t1   = {.tv_sec = 1, .tv_nsec = 100};
  struct timespec t2   = {.tv_sec = 5, .tv_nsec = 800};
  struct timespec diff = {};

  /* Normal case: end > start, no borrow */
  utils_ts_diff(&t1, &t2, &diff);
  CATCH_REQUIRE(diff.tv_sec  == 4);
  CATCH_REQUIRE(diff.tv_nsec == 700);

  /* Borrow case: end.tv_nsec < start.tv_nsec */
  struct timespec early = {.tv_sec = 1, .tv_nsec = 900};
  struct timespec late  = {.tv_sec = 3, .tv_nsec = 100};
  utils_ts_diff(&early, &late, &diff);
  CATCH_REQUIRE(diff.tv_sec  == 1);
  CATCH_REQUIRE(diff.tv_nsec == (long)(1000000000L + 100 - 900));

  /* diff(t, t) == {0, 0} */
  utils_ts_diff(&t1, &t1, &diff);
  CATCH_REQUIRE(diff.tv_sec  == 0);
  CATCH_REQUIRE(diff.tv_nsec == 0);
}

static void ts2mono_test() {
  struct timespec ts = {.tv_sec = 2, .tv_nsec = 500000000L}; /* 2.5 s */
  double mono = utils_ts2mono(&ts);
  CATCH_REQUIRE(mono > 2.4999);
  CATCH_REQUIRE(mono < 2.5001);

  /* Later timestamp must produce a larger scalar */
  struct timespec ts1 = {.tv_sec = 1, .tv_nsec = 0};
  struct timespec ts2 = {.tv_sec = 2, .tv_nsec = 0};
  CATCH_REQUIRE(utils_ts2mono(&ts2) > utils_ts2mono(&ts1));
}

static void ts2monons_test() {
  struct timespec ts = {.tv_sec = 1, .tv_nsec = 1};
  /* Expected: 1 * 1e9 + 1 = 1000000001 */
  CATCH_REQUIRE(utils_ts2monons(&ts) == 1000000001ULL);

  struct timespec zero = {.tv_sec = 0, .tv_nsec = 0};
  CATCH_REQUIRE(utils_ts2monons(&zero) == 0ULL);
}

static void monons2ts_test() {
  /* Round-trip through monons2ts / ts2monons */
  uint64_t ns = 3000000007ULL; /* 3 s + 7 ns */
  struct timespec ts = utils_monons2ts(ns);
  CATCH_REQUIRE(ts.tv_sec  == 3);
  CATCH_REQUIRE(ts.tv_nsec == 7);

  CATCH_REQUIRE(utils_ts2monons(&ts) == ns);

  /* Zero */
  struct timespec z = utils_monons2ts(0ULL);
  CATCH_REQUIRE(z.tv_sec  == 0);
  CATCH_REQUIRE(z.tv_nsec == 0);
}

static void ts_make_abs_test() {
  /* utils_ts_make_abs() adds a relative offset to the current wall clock. */
  struct timespec rel = {.tv_sec = 10, .tv_nsec = 0};
  struct timespec abs_out = {};

  CATCH_REQUIRE(utils_ts_make_abs(&rel, &abs_out) == OK);

  /* The result must be in the future relative to now */
  struct timespec now;
  clock_gettime(CLOCK_REALTIME, &now);
  CATCH_REQUIRE(utils_ts_cmp(&abs_out, &now) > 0);
}

static void ts_make_rel_test() {
  /* A deadline in the past must return ERROR and set rel_out to {0,0}. */
  struct timespec past = {.tv_sec = 1, .tv_nsec = 0}; /* Unix epoch + 1 s */
  struct timespec rel_out = {.tv_sec = -1, .tv_nsec = -1};
  CATCH_REQUIRE(utils_ts_make_rel(&past, &rel_out) == ERROR);
  CATCH_REQUIRE(rel_out.tv_sec  == 0);
  CATCH_REQUIRE(rel_out.tv_nsec == 0);

  /* A deadline 60 s in the future must return OK and a positive remainder. */
  struct timespec future = {};
  clock_gettime(CLOCK_REALTIME, &future);
  future.tv_sec += 60;
  struct timespec rem = {};
  CATCH_REQUIRE(utils_ts_make_rel(&future, &rem) == OK);
  CATCH_REQUIRE(rem.tv_sec > 0);

  /* NULL arguments must return ERROR */
  CATCH_REQUIRE(utils_ts_make_rel(nullptr, &rem)    == ERROR);
  CATCH_REQUIRE(utils_ts_make_rel(&future, nullptr) == ERROR);
}

static void monotonic_advance_test() {
  /* A real sleep must move the monotonic clock forward. */
  struct timespec before = clock_monotime();
  sleep(1);
  struct timespec after = clock_monotime();
  CATCH_REQUIRE(utils_ts2mono(&after) > utils_ts2mono(&before));
}

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("Time ts_cmp Test", "[utils][time]") {
  ts_cmp_test();
}
CATCH_TEST_CASE("Time ts_add Test", "[utils][time]") {
  ts_add_test();
}
CATCH_TEST_CASE("Time ts_diff Test", "[utils][time]") {
  ts_diff_test();
}
CATCH_TEST_CASE("Time ts2mono Test", "[utils][time]") {
  ts2mono_test();
}
CATCH_TEST_CASE("Time ts2monons Test", "[utils][time]") {
  ts2monons_test();
}
CATCH_TEST_CASE("Time monons2ts Test", "[utils][time]") {
  monons2ts_test();
}
CATCH_TEST_CASE("Time ts_make_abs Test", "[utils][time]") {
  ts_make_abs_test();
}
CATCH_TEST_CASE("Time ts_make_rel Test", "[utils][time]") {
  ts_make_rel_test();
}
CATCH_TEST_CASE("Time Monotonic Advance Test", "[utils][time][slow]") {
  monotonic_advance_test();
}
