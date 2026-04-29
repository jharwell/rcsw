/**
 * \file utils_mem-utest.cpp
 *
 * Unit tests for rcsw/utils/mem.h: utils_mem_cpy32(), utils_mem_bswap16/32(),
 * alignment precondition enforcement, and coverage of the dump functions.
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

#include <cstring>

#include "rcsw/utils/mem.h"
#include "rcsw/utils/byteops.h"
#include "rcsw/utils/align.h"

#include "tests/test_utils.hpp"

/*******************************************************************************
 * Test Helper Functions
 ******************************************************************************/

static void mem_cpy32_test() {
  /* Copy a variety of aligned sizes and verify byte-for-byte equality. */
  uint8_t src[1000];
  uint8_t dst[1000];

  for (size_t n = 4; n <= sizeof(src); n += 4) {
    std::memset(src, 0, sizeof(src));
    std::memset(dst, 0, sizeof(dst));
    utils_string_gen((char*)src, n);
    utils_mem_cpy32(dst, src, n);
    CATCH_REQUIRE(std::memcmp(src, dst, n) == 0);
  }
}

static void mem_cpy32_alignment_test() {
  /* Unaligned dest, src, or size must return dest without copying. */
  uint32_t src[4] = {1, 2, 3, 4};
  uint32_t dst[4] = {0, 0, 0, 0};

  /* Unaligned byte count – must not copy */
  void* ret = utils_mem_cpy32(dst, src, 7); /* 7 is not 4-byte aligned */
  CATCH_REQUIRE(ret == (void*)dst);
  CATCH_REQUIRE(dst[0] == 0u); /* unchanged */
}

static void mem_bswap16_test() {
  uint16_t in16[4]  = {0x1234, 0x2345, 0x3456, 0x4567};
  uint16_t out16[4] = {0x3412, 0x4523, 0x5634, 0x6745};

  CATCH_REQUIRE(utils_mem_bswap16(in16, sizeof(in16)) == OK);
  CATCH_REQUIRE(std::memcmp(in16, out16, sizeof(in16)) == 0);

  /* Applying bswap16 twice restores the original values */
  CATCH_REQUIRE(utils_mem_bswap16(in16, sizeof(in16)) == OK);
  CATCH_REQUIRE(in16[0] == 0x1234u);
  CATCH_REQUIRE(in16[1] == 0x2345u);
}

static void mem_bswap32_test() {
  uint32_t in32[4]  = {0x00001234, 0x00002345, 0x00003456, 0x00004567};
  uint32_t out32[4] = {0x34120000, 0x45230000, 0x56340000, 0x67450000};

  CATCH_REQUIRE(utils_mem_bswap32(in32, sizeof(in32)) == OK);
  CATCH_REQUIRE(std::memcmp(in32, out32, sizeof(in32)) == 0);

  /* Applying bswap32 twice restores the original values */
  CATCH_REQUIRE(utils_mem_bswap32(in32, sizeof(in32)) == OK);
  CATCH_REQUIRE(in32[0] == 0x00001234u);
  CATCH_REQUIRE(in32[1] == 0x00002345u);
}

static void mem_dump_coverage_test() {
  /*
   * The dump functions write to stdout via DPRINTF.  We capture output to
   * verify each variant produces non-empty output for a non-trivial buffer,
   * and that the verbose (offset-annotated) variants include "Offset:" in
   * their output.  Exact byte-for-byte format is not checked here.
   */
  alignas(4) uint8_t buf[128];
  for (size_t i = 0; i < sizeof(buf); ++i) {
    buf[i] = (uint8_t)(i & 0xFF);
  }

  /* Non-verbose variants: must produce output */
  CATCH_REQUIRE(!capture_stdout([&] { utils_mem_dump8(buf, sizeof(buf)); }).empty());
  CATCH_REQUIRE(!capture_stdout([&] { utils_mem_dump16(buf, sizeof(buf) / 2); }).empty());
  CATCH_REQUIRE(!capture_stdout([&] { utils_mem_dump32(buf, sizeof(buf) / 4); }).empty());

  /* Verbose variants: must contain "Offset:" header */
  std::string out8v  = capture_stdout([&] { utils_mem_dump8v(buf, sizeof(buf)); });
  std::string out16v = capture_stdout([&] { utils_mem_dump16v(buf, sizeof(buf) / 2); });
  std::string out32v = capture_stdout([&] { utils_mem_dump32v(buf, sizeof(buf) / 4); });

  CATCH_REQUIRE(out8v.find("Offset:")  != std::string::npos);
  CATCH_REQUIRE(out16v.find("Offset:") != std::string::npos);
  CATCH_REQUIRE(out32v.find("Offset:") != std::string::npos);
}

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("mem_cpy32 Test", "[utils][mem]") {
  mem_cpy32_test();
}
CATCH_TEST_CASE("mem_cpy32 Alignment Test", "[utils][mem]") {
  mem_cpy32_alignment_test();
}
CATCH_TEST_CASE("mem_bswap16 Test", "[utils][mem]") {
  mem_bswap16_test();
}
CATCH_TEST_CASE("mem_bswap32 Test", "[utils][mem]") {
  mem_bswap32_test();
}
CATCH_TEST_CASE("mem_dump Coverage Test", "[utils][mem]") {
  mem_dump_coverage_test();
}
