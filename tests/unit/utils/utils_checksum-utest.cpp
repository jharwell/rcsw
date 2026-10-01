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
 *   - utils_crc32_eth     (IEEE 802.3 direct)
 *   - utils_crc32_ethl    (IEEE 802.3 lookup-table)
 *   - utils_crc32_brown   (Gary S. Brown CRC32)
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

// NOLINTNEXTLINE(readability-function-size)
static void crc32_eth_test() {
  /* Known CRC32 value for the canonical 8-byte Ethernet frame */
  uint32_t crc = utils_crc32_eth(crcframe, sizeof(crcframe));
  CATCH_REQUIRE(RCSW_BSWAP32(crc) == 0x0b5e0332u);

  /* Deterministic: computing twice gives the same result */
  CATCH_REQUIRE(utils_crc32_eth(crcframe, sizeof(crcframe)) == crc);

  /* Different data must produce a different CRC */
  uint8_t alt[] = {0x4d, 0x54, 0x30, 0x30, 0x01, 0x02, 0x03, 0x05};
  CATCH_REQUIRE(utils_crc32_eth(alt, sizeof(alt)) != crc);
}

static void crc32_ethl_test() {
  /* Table variant must agree with the direct variant on the same frame */
  uint32_t crc_direct = utils_crc32_eth(crcframe, sizeof(crcframe));
  uint32_t crc_table  = utils_crc32_ethl(crcframe, sizeof(crcframe));
  CATCH_REQUIRE(crc_direct == crc_table);

  /* Known value check (same expected result as the direct version) */
  CATCH_REQUIRE(RCSW_BSWAP32(crc_table) == 0x0b5e0332u);
}

static void crc32_brown_test() {
  /* Non-trivial data must produce a non-zero CRC */
  uint8_t buf[100];
  for (int i = 0; i < 100; ++i) {
    buf[i] = (uint8_t)(i + 1);
  }
  CATCH_REQUIRE(utils_crc32_brown(buf, sizeof(buf), 0) != 0u);

  /* Known vector: the original test checks > 0 after a data_gen; here we
     verify determinism directly. */
  uint32_t a = utils_crc32_brown(buf, sizeof(buf), 0);
  uint32_t b = utils_crc32_brown(buf, sizeof(buf), 0);
  CATCH_REQUIRE(a == b);

  /* Changing the buffer must change the CRC */
  buf[0] ^= 0xFFu;
  CATCH_REQUIRE(utils_crc32_brown(buf, sizeof(buf), 0) != a);
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
CATCH_TEST_CASE("Checksum CRC32 Ethernet Test", "[utils][checksum][noalloc]") {
  crc32_eth_test();
}
CATCH_TEST_CASE("Checksum CRC32 Ethernet Lookup-Table Test",
                "[utils][checksum][noalloc]") {
  crc32_ethl_test();
}
CATCH_TEST_CASE("Checksum CRC32 Brown Test", "[utils][checksum][noalloc]") {
  crc32_brown_test();
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

  /* CRC-32 of the empty message is 0 */
  CATCH_REQUIRE(0 == utils_crc32_eth(b8, 0));
  CATCH_REQUIRE(0 == utils_crc32_ethl(b8, 0));
  /* Brown's CRC continues from the passed CRC; empty input leaves it alone */
  CATCH_REQUIRE(0x12345678 == utils_crc32_brown(b8, 0, 0x12345678));
}

CATCH_TEST_CASE(
  "utils_crc32_ethl needs no initialization and matches "
  "utils_crc32_eth",
  "[utils][checksum][noalloc]") {
  const uint8_t msg[] = "123456789";
  /* standard CRC-32 check value */
  CATCH_REQUIRE(0xCBF43926U == utils_crc32_ethl(msg, 9));
  CATCH_REQUIRE(utils_crc32_eth(msg, 9) == utils_crc32_ethl(msg, 9));
  CATCH_REQUIRE(utils_crc32_brown(msg, 9, 0) == utils_crc32_ethl(msg, 9));
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
