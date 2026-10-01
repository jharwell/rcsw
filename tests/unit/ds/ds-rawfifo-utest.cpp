/**
 * \file
 *
 * \copyright 2026 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * Unit tests for rawfifo: round trips, element packing, and argument
 * validation.
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#define CATCH_CONFIG_PREFIX_ALL
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <thread>

#include <catch2/catch_test_macros.hpp>

#include "rcsw/core/core.h"
#include "rcsw/ds/rawfifo.h"

/*******************************************************************************
 * Helpers
 ******************************************************************************/
namespace {
/*
 * Enqueue/dequeue n elements of elt_size bytes through a buffer that is
 * EXACTLY max_elts * elt_size bytes (heap, so ASan sees any overrun), and
 * check both the round trip and the in-buffer layout.
 */
void roundtrip(size_t elt_size, size_t max_elts, size_t n) {
  size_t  bytes = max_elts * elt_size;
  auto*   buf   = static_cast<uint8_t*>(malloc(bytes));
  rawfifo f;
  CATCH_REQUIRE(OK == rawfifo_init(&f, buf, max_elts, elt_size));

  uint8_t in[64];
  uint8_t out[64];
  for (size_t i = 0; i < n * elt_size; ++i) {
    in[i] = static_cast<uint8_t>(i + 1);
  }
  CATCH_REQUIRE(n == rawfifo_enq(&f, in, n));
  /* elements are packed: element i occupies bytes [i*elt_size, ...) */
  CATCH_REQUIRE(0 == memcmp(buf, in, n * elt_size));
  CATCH_REQUIRE(n == rawfifo_deq(&f, out, n));
  CATCH_REQUIRE(0 == memcmp(in, out, n * elt_size));
  free(buf);
}
} /* namespace */

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("rawfifo: 4-byte elements round-trip", "[ds][rawfifo][noalloc]") {
  roundtrip(4, 8, 7);
}

CATCH_TEST_CASE("rawfifo: 1-byte elements are packed at elt_size stride",
                "[ds][rawfifo][noalloc]") {
  roundtrip(1, 8, 7);
}

CATCH_TEST_CASE("rawfifo: 2-byte elements are packed at elt_size stride",
                "[ds][rawfifo][noalloc]") {
  roundtrip(2, 8, 7);
}

CATCH_TEST_CASE("rawfifo holds max_elts - 1 elements", "[ds][rawfifo][noalloc]") {
  uint32_t buf[8];
  rawfifo  f;
  CATCH_REQUIRE(OK == rawfifo_init(&f, buf, 8, sizeof(uint32_t)));
  uint32_t in[8] = {1, 2, 3, 4, 5, 6, 7, 8};
  CATCH_REQUIRE(7 == rawfifo_enq(&f, in, 8));
  CATCH_REQUIRE(0 == rawfifo_n_free(&f));
}

CATCH_TEST_CASE("rawfifo_init rejects element sizes other than 1, 2, 4",
                "[ds][rawfifo][noalloc]") {
  uint32_t buf[8];
  rawfifo  f;
  CATCH_REQUIRE(ERROR == rawfifo_init(&f, buf, 8, 0));
  CATCH_REQUIRE(ERROR == rawfifo_init(&f, buf, 8, 3));
}

CATCH_TEST_CASE("rawfifo_init rejects max_elts < 2", "[ds][rawfifo][noalloc]") {
  uint32_t buf[8];
  rawfifo  f;
  /* one slot is always kept empty, so fewer than 2 slots holds nothing */
  CATCH_REQUIRE(ERROR == rawfifo_init(&f, buf, 0, sizeof(uint32_t)));
  CATCH_REQUIRE(ERROR == rawfifo_init(&f, buf, 1, sizeof(uint32_t)));
}

CATCH_TEST_CASE(
  "rawfifo: one producer and one consumer thread see every "
  "element exactly once, in order",
  "[ds][rawfifo][noalloc]") {
  constexpr uint32_t kCount = 50000;
  static uint32_t    buf[8]; /* small, so both sides wrap constantly */
  static rawfifo     f;
  CATCH_REQUIRE(OK ==
                rawfifo_init(&f, buf, RCSW_ARRAY_ELTS(buf), sizeof(uint32_t)));

  std::thread producer([] {
    uint32_t next = 0;
    while (next < kCount) {
      uint32_t chunk[3] = {next, next + 1, next + 2};
      size_t   n        = RCSW_MIN(static_cast<size_t>(kCount - next), size_t{3});
      size_t   added    = rawfifo_enq(&f, chunk, n);
      if (0 == added) {
        /* full: let the consumer run (1-CPU hosts) */
        std::this_thread::yield();
      }
      next += static_cast<uint32_t>(added);
    }
  });

  uint32_t expected = 0;
  bool     in_order = true;
  while (expected < kCount) {
    uint32_t out[4];
    size_t   n = rawfifo_deq(&f, out, RCSW_ARRAY_ELTS(out));
    if (0 == n) {
      std::this_thread::yield(); /* empty: left producer run */
    }
    for (size_t i = 0; i < n; ++i) {
      in_order = in_order && (out[i] == expected);
      ++expected;
    }
  }
  producer.join();
  CATCH_REQUIRE(in_order);
  CATCH_REQUIRE(0 == rawfifo_size(&f));
}

CATCH_TEST_CASE("rawfifo_clear discards the queued elements",
                "[ds][rawfifo][noalloc]") {
  uint8_t buf[4];
  rawfifo f;
  CATCH_REQUIRE(OK == rawfifo_init(&f, buf, sizeof(buf), 1));
  uint8_t in[] = {1, 2, 3};
  CATCH_REQUIRE(3 == rawfifo_enq(&f, in, 3));
  CATCH_REQUIRE(OK == rawfifo_clear(&f));
  CATCH_REQUIRE(0 == rawfifo_size(&f));
  CATCH_REQUIRE(3 == rawfifo_n_free(&f));
  uint8_t out;
  CATCH_REQUIRE(0 == rawfifo_deq(&f, &out, 1));
}
