/**
 * \file
 *
 * \copyright 2023 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * Unit tests for rcsw/utils/checksum.h
 *
 * Checksum coverage:
 *   - utils_xchks8/16/32  (XOR-rotate)
 *   - utils_achks8/16/32  (add-ignore-carry)
 *   - utils_crc32         (CRC-32, lookup table)
 *   - utils_crc32_bitwise (CRC-32, no table)
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#define CATCH_CONFIG_PREFIX_ALL
#include <algorithm>
#include <cerrno>
#include <cstring>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "rcsw/utils/byteops.h"
#include "rcsw/utils/checksum.h"

/*******************************************************************************
 * Namespaces/Decls
 ******************************************************************************/

/* Canonical 8-byte frame used in the original CRC tests */
static const uint8_t crcframe[] = {
  0x4d, 0x54, 0x30, 0x30, 0x01, 0x02, 0x03, 0x04};

/*******************************************************************************
 * Test Helper Functions
 ******************************************************************************/

static void xchks_test() {
  /* Non-zero data must produce a non-zero checksum for all widths */
  uint8_t  buf8[100];
  uint16_t buf16[100];
  uint32_t buf32[100];

  for (int i = 0; i < 100; ++i) {
    buf8[i]  = (uint8_t)(i + 1);
    buf16[i] = (uint16_t)(i + 1);
    buf32[i] = (uint32_t)(i + 1);
  }

  CATCH_REQUIRE(utils_xchks8(buf8, 100, 0) != 0u);
  CATCH_REQUIRE(utils_xchks16(buf16, 100, 0u) != 0u);
  CATCH_REQUIRE(utils_xchks32(buf32, 100, 0u) != 0u);

  /* A buffer of all-zero bytes with seed 0 must produce 0 */
  uint8_t  zeros8[16]  = {};
  uint16_t zeros16[16] = {};
  uint32_t zeros32[16] = {};
  CATCH_REQUIRE(utils_xchks8(zeros8, 16, 0) == 0u);
  CATCH_REQUIRE(utils_xchks16(zeros16, 16, 0u) == 0u);
  CATCH_REQUIRE(utils_xchks32(zeros32, 16, 0u) == 0u);

  /* Non-zero seed must influence the result */
  CATCH_REQUIRE(utils_xchks8(zeros8, 16, 0xFFu) != utils_xchks8(zeros8, 16, 0u));
}

static void achks_test() {
  /* Non-zero data must yield a non-zero add-ignore-carry checksum */
  uint8_t  buf8[100];
  uint16_t buf16[100];
  uint32_t buf32[100];

  for (int i = 0; i < 100; ++i) {
    buf8[i]  = (uint8_t)(i + 1);
    buf16[i] = (uint16_t)(i + 1);
    buf32[i] = (uint32_t)(i + 1);
  }

  CATCH_REQUIRE(utils_achks8(buf8, 100, 0) != 0u);
  CATCH_REQUIRE(utils_achks16(buf16, 100, 0u) != 0u);
  CATCH_REQUIRE(utils_achks32(buf32, 100, 0u) != 0u);

  /* Seed propagation: different seeds must produce different sums */
  CATCH_REQUIRE(utils_achks8(buf8, 100, 0u) != utils_achks8(buf8, 100, 0xFFu));

  /* Deterministic: same inputs must produce same outputs */
  CATCH_REQUIRE(utils_achks8(buf8, 100, 0u) == utils_achks8(buf8, 100, 0u));
}

static void crc32_test() {
  /* Known CRC32 value for the canonical 8-byte Ethernet frame */
  uint32_t crc = utils_crc32(crcframe, sizeof(crcframe), 0);
  CATCH_REQUIRE(RCSW_BSWAP32(crc) == 0x0b5e0332u);

  /* Different data must produce a different CRC */
  uint8_t alt[] = {0x4d, 0x54, 0x30, 0x30, 0x01, 0x02, 0x03, 0x05};
  CATCH_REQUIRE(utils_crc32(alt, sizeof(alt), 0) != crc);
}

static void crc32_bitwise_test() {
  /* The table-free version must agree with the table version */
  uint8_t buf[300];
  for (size_t i = 0; i < sizeof(buf); ++i) {
    buf[i] = static_cast<uint8_t>(i * 7 + 1);
  }
  for (size_t n = 0; n <= sizeof(buf); n += 13) {
    CATCH_REQUIRE(utils_crc32(buf, n, 0) == utils_crc32_bitwise(buf, n, 0));
  }
  CATCH_REQUIRE(RCSW_BSWAP32(utils_crc32_bitwise(crcframe, sizeof(crcframe), 0)) ==
                0x0b5e0332u);
}

static void crc32_continue_test() {
  /* Passing a previous result back in continues the CRC across buffers */
  uint8_t buf[100];
  for (int i = 0; i < 100; ++i) {
    buf[i] = static_cast<uint8_t>(i + 1);
  }
  uint32_t whole = utils_crc32(buf, sizeof(buf), 0);
  for (size_t split : {0UL, 1UL, 37UL, 99UL, 100UL}) {
    uint32_t part = utils_crc32(buf, split, 0);
    CATCH_REQUIRE(whole == utils_crc32(buf + split, sizeof(buf) - split, part));
    part = utils_crc32_bitwise(buf, split, 0);
    CATCH_REQUIRE(whole ==
                  utils_crc32_bitwise(buf + split, sizeof(buf) - split, part));
  }
}

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("Checksum XOR-Rotate Test", "[utils][checksum][noalloc]") {
  xchks_test();
}
CATCH_TEST_CASE("Checksum Add-Ignore-Carry Test", "[utils][checksum][noalloc]") {
  achks_test();
}
CATCH_TEST_CASE("Checksum CRC32 Test", "[utils][checksum][noalloc]") {
  crc32_test();
}
CATCH_TEST_CASE("Checksum CRC32 Bitwise Test", "[utils][checksum][noalloc]") {
  crc32_bitwise_test();
}
CATCH_TEST_CASE("Checksum CRC32 Continuation Test",
                "[utils][checksum][noalloc]") {
  crc32_continue_test();
}

CATCH_TEST_CASE(
  "checksums of an empty buffer return the seed / standard "
  "empty CRC",
  "[utils][checksum][noalloc]") {
  uint8_t  b8[1]  = {0};
  uint16_t b16[1] = {0};
  uint32_t b32[1] = {0};

  CATCH_REQUIRE(0x5A == utils_xchks8(b8, 0, 0x5A));
  CATCH_REQUIRE(0x5A == utils_achks8(b8, 0, 0x5A));
  CATCH_REQUIRE(0x1234 == utils_xchks16(b16, 0, 0x1234));
  CATCH_REQUIRE(0x1234 == utils_achks16(b16, 0, 0x1234));
  CATCH_REQUIRE(0xDEADBEEF == utils_xchks32(b32, 0, 0xDEADBEEF));
  CATCH_REQUIRE(0xDEADBEEF == utils_achks32(b32, 0, 0xDEADBEEF));

  /* CRC-32 of the empty message is 0; empty input continues unchanged */
  CATCH_REQUIRE(0 == utils_crc32(b8, 0, 0));
  CATCH_REQUIRE(0 == utils_crc32_bitwise(b8, 0, 0));
  CATCH_REQUIRE(0x12345678 == utils_crc32(b8, 0, 0x12345678));
  CATCH_REQUIRE(0x12345678 == utils_crc32_bitwise(b8, 0, 0x12345678));
}

CATCH_TEST_CASE(
  "utils_crc32 and utils_crc32_bitwise give the standard CRC-32 check "
  "value",
  "[utils][checksum][noalloc]") {
  const uint8_t msg[] = "123456789";
  /* standard CRC-32 check value */
  CATCH_REQUIRE(0xCBF43926U == utils_crc32(msg, 9, 0));
  CATCH_REQUIRE(0xCBF43926U == utils_crc32_bitwise(msg, 9, 0));
}

CATCH_TEST_CASE("16/32-bit checksums fail with -1 on misaligned input",
                "[utils][checksum][noalloc]") {
  alignas(uint32_t) uint8_t buf[16] = {1, 2, 3, 4, 5, 6, 7, 8};
  const auto* b16 = reinterpret_cast<const uint16_t*>(buf);
  const auto* b32 = reinterpret_cast<const uint32_t*>(buf);
  const auto* m16 = reinterpret_cast<const uint16_t*>(buf + 1);
  const auto* m32 = reinterpret_cast<const uint32_t*>(buf + 2);

  /* Misaligned buffer */
  errno = 0;
  CATCH_REQUIRE(UINT16_MAX == utils_xchks16(m16, 4, 0));
  CATCH_REQUIRE(EINVAL == errno);
  errno = 0;
  CATCH_REQUIRE(UINT16_MAX == utils_achks16(m16, 4, 0));
  CATCH_REQUIRE(EINVAL == errno);
  errno = 0;
  CATCH_REQUIRE(UINT32_MAX == utils_xchks32(m32, 4, 0));
  CATCH_REQUIRE(EINVAL == errno);
  errno = 0;
  CATCH_REQUIRE(UINT32_MAX == utils_achks32(m32, 4, 0));
  CATCH_REQUIRE(EINVAL == errno);

  /* Misaligned length */
  errno = 0;
  CATCH_REQUIRE(UINT16_MAX == utils_xchks16(b16, 3, 0));
  CATCH_REQUIRE(EINVAL == errno);
  errno = 0;
  CATCH_REQUIRE(UINT32_MAX == utils_achks32(b32, 6, 0));
  CATCH_REQUIRE(EINVAL == errno);

  /* Aligned input succeeds and leaves errno alone */
  errno = 0;
  CATCH_REQUIRE(UINT32_MAX != utils_achks32(b32, 8, 0));
  CATCH_REQUIRE(0 == errno);
}
