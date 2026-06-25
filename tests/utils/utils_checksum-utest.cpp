/**
 * \file utils_checksum-utest.cpp
 *
 * Unit tests for rcsw/utils/checksum.h and rcsw/utils/hash.h.
 *
 * Checksum coverage:
 *   - utils_xchks8/16/32  (XOR-rotate)
 *   - utils_achks8/16/32  (add-ignore-carry)
 *   - utils_crc32_eth     (IEEE 802.3 direct)
 *   - utils_crc32_ethl    (IEEE 802.3 lookup-table)
 *   - utils_crc32_brown   (Gary S. Brown CRC32)
 *
 * Hash coverage:
 *   - utils_hash_default  (Bob Jenkins)
 *   - utils_hash_fnv1a    (FNV-1a)
 *   - utils_hash_djb      (DJB2)
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
#include <vector>

#include "rcsw/utils/checksum.h"
#include "rcsw/utils/hash.h"
#include "rcsw/utils/byteops.h"

/*******************************************************************************
 * Namespaces/Decls
 ******************************************************************************/

/* Canonical 8-byte frame used in the original CRC tests */
static const uint8_t kCrcFrame[] = {0x4d, 0x54, 0x30, 0x30,
                                     0x01, 0x02, 0x03, 0x04};

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

  CATCH_REQUIRE(utils_xchks8(buf8,   100,    0) != 0u);
  CATCH_REQUIRE(utils_xchks16(buf16, 100, 0u) != 0u);
  CATCH_REQUIRE(utils_xchks32(buf32, 100, 0u) != 0u);

  /* A buffer of all-zero bytes with seed 0 must produce 0 */
  uint8_t zeros8[16]   = {};
  uint16_t zeros16[16] = {};
  uint32_t zeros32[16] = {};
  CATCH_REQUIRE(utils_xchks8(zeros8,   16,    0) == 0u);
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

  CATCH_REQUIRE(utils_achks8(buf8,   100,    0) != 0u);
  CATCH_REQUIRE(utils_achks16(buf16, 100, 0u) != 0u);
  CATCH_REQUIRE(utils_achks32(buf32, 100, 0u) != 0u);


  /* Seed propagation: different seeds must produce different sums */
  CATCH_REQUIRE(utils_achks8(buf8, 100, 0u) != utils_achks8(buf8, 100, 0xFFu));

  /* Deterministic: same inputs must produce same outputs */
  CATCH_REQUIRE(utils_achks8(buf8, 100, 0u) == utils_achks8(buf8, 100, 0u));
}

static void crc32_eth_test() {
  /* Known CRC32 value for the canonical 8-byte Ethernet frame */
  uint32_t crc = utils_crc32_eth(kCrcFrame, sizeof(kCrcFrame));
  CATCH_REQUIRE(RCSW_BSWAP32(crc) == 0x0b5e0332u);

  /* Deterministic: computing twice gives the same result */
  CATCH_REQUIRE(utils_crc32_eth(kCrcFrame, sizeof(kCrcFrame)) == crc);

  /* Different data must produce a different CRC */
  uint8_t alt[] = {0x4d, 0x54, 0x30, 0x30, 0x01, 0x02, 0x03, 0x05};
  CATCH_REQUIRE(utils_crc32_eth(alt, sizeof(alt)) != crc);
}

static void crc32_ethl_test() {
  /* Table variant must agree with the direct variant on the same frame */
  utils_crc32_ethl_init();
  uint32_t crc_direct = utils_crc32_eth (kCrcFrame, sizeof(kCrcFrame));
  uint32_t crc_table  = utils_crc32_ethl(kCrcFrame, sizeof(kCrcFrame));
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

static void hash_djb_test() {
  uint8_t data[100];
  for (int i = 0; i < 100; ++i) {
    data[i] = (uint8_t)(i + 1);
  }

  uint32_t hash = 0;
  CATCH_REQUIRE(utils_hash_djb(data, sizeof(data), &hash) == OK);
  CATCH_REQUIRE(hash != 0u);

  /* Deterministic */
  uint32_t hash2 = 0;
  CATCH_REQUIRE(utils_hash_djb(data, sizeof(data), &hash2) == OK);
  CATCH_REQUIRE(hash == hash2);

  /* Different data must produce a different hash (with high probability) */
  data[0] ^= 0xFFu;
  uint32_t hash3 = 0;
  CATCH_REQUIRE(utils_hash_djb(data, sizeof(data), &hash3) == OK);
  CATCH_REQUIRE(hash != hash3);

  /* NULL pointer must return ERROR */
  CATCH_REQUIRE(utils_hash_djb(nullptr, 10, &hash) == ERROR);
  CATCH_REQUIRE(utils_hash_djb(data, 0, &hash)    == ERROR);
}

static void hash_fnv1a_test() {
  uint8_t data[100];
  for (int i = 0; i < 100; ++i) {
    data[i] = (uint8_t)(i + 1);
  }

  uint32_t hash = 0;
  CATCH_REQUIRE(utils_hash_fnv1a(data, sizeof(data), &hash) == OK);
  CATCH_REQUIRE(hash != 0u);

  /* FNV-1a must be deterministic */
  uint32_t hash2 = 0;
  CATCH_REQUIRE(utils_hash_fnv1a(data, sizeof(data), &hash2) == OK);
  CATCH_REQUIRE(hash == hash2);

  /* NULL pointer must return ERROR */
  CATCH_REQUIRE(utils_hash_fnv1a(nullptr, 10, &hash) == ERROR);
}

static void hash_default_test() {
  uint8_t data[100];
  for (int i = 0; i < 100; ++i) {
    data[i] = (uint8_t)(i + 1);
  }

  uint32_t hash = 0;
  CATCH_REQUIRE(utils_hash_default(data, sizeof(data), &hash) == OK);
  CATCH_REQUIRE(hash != 0u);

  /* Deterministic */
  uint32_t hash2 = 0;
  CATCH_REQUIRE(utils_hash_default(data, sizeof(data), &hash2) == OK);
  CATCH_REQUIRE(hash == hash2);

  /* NULL pointer must return ERROR */
  CATCH_REQUIRE(utils_hash_default(nullptr, 10, &hash) == ERROR);
}

static void hash_distinct_test() {
  /*
   * All three algorithms must produce distinct results for the same input.
   * (The probability of accidental collision on a 100-byte non-trivial
   * input is negligible.)
   */
  uint8_t data[100];
  for (int i = 0; i < 100; ++i) {
    data[i] = (uint8_t)(i + 1);
  }

  uint32_t djb = 0, fnv = 0, def = 0;
  CATCH_REQUIRE(utils_hash_djb    (data, sizeof(data), &djb) == OK);
  CATCH_REQUIRE(utils_hash_fnv1a  (data, sizeof(data), &fnv) == OK);
  CATCH_REQUIRE(utils_hash_default(data, sizeof(data), &def) == OK);

  CATCH_REQUIRE(djb != fnv);
  CATCH_REQUIRE(djb != def);
  CATCH_REQUIRE(fnv != def);
}

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("Checksum XOR-Rotate Test", "[utils][checksum]") {
  xchks_test();
}
CATCH_TEST_CASE("Checksum Add-Ignore-Carry Test", "[utils][checksum]") {
  achks_test();
}
CATCH_TEST_CASE("Checksum CRC32 Ethernet Test", "[utils][checksum]") {
  crc32_eth_test();
}
CATCH_TEST_CASE("Checksum CRC32 Ethernet Lookup-Table Test", "[utils][checksum]") {
  crc32_ethl_test();
}
CATCH_TEST_CASE("Checksum CRC32 Brown Test", "[utils][checksum]") {
  crc32_brown_test();
}
CATCH_TEST_CASE("Hash DJB2 Test", "[utils][hash]") {
  hash_djb_test();
}
CATCH_TEST_CASE("Hash FNV-1a Test", "[utils][hash]") {
  hash_fnv1a_test();
}
CATCH_TEST_CASE("Hash Default Test", "[utils][hash]") {
  hash_default_test();
}
CATCH_TEST_CASE("Hash Distinct Algorithms Test", "[utils][hash]") {
  hash_distinct_test();
}
