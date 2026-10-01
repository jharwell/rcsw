/**
 * \file
 *
 * \copyright 2023 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * Unit tests for rcsw/utils/hash.h.
 *
 * Hash coverage:
 *   - utils_hash_default  (Bob Jenkins)
 *   - utils_hash_fnv1a    (FNV-1a)
 *   - utils_hash_djb      (DJB2)
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#define CATCH_CONFIG_PREFIX_ALL
#include <algorithm>
#include <cstring>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "rcsw/utils/byteops.h"
#include "rcsw/utils/checksum.h"
#include "rcsw/utils/hash.h"

/*******************************************************************************
 * Test Harness
 ******************************************************************************/
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
  CATCH_REQUIRE(utils_hash_djb(data, 0, &hash) == ERROR);
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

  uint32_t djb = 0;
  uint32_t fnv = 0;
  uint32_t def = 0;
  CATCH_REQUIRE(utils_hash_djb(data, sizeof(data), &djb) == OK);
  CATCH_REQUIRE(utils_hash_fnv1a(data, sizeof(data), &fnv) == OK);
  CATCH_REQUIRE(utils_hash_default(data, sizeof(data), &def) == OK);

  CATCH_REQUIRE(djb != fnv);
  CATCH_REQUIRE(djb != def);
  CATCH_REQUIRE(fnv != def);
}

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("Hash DJB2 Test", "[utils][hash][noalloc]") { hash_djb_test(); }
CATCH_TEST_CASE("Hash FNV-1a Test", "[utils][hash][noalloc]") {
  hash_fnv1a_test();
}
CATCH_TEST_CASE("Hash Default Test", "[utils][hash][noalloc]") {
  hash_default_test();
}
CATCH_TEST_CASE("Hash Distinct Algorithms Test", "[utils][hash][noalloc]") {
  hash_distinct_test();
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

CATCH_TEST_CASE("utils_hash_djb rejects a NULL output pointer",
                "[utils][hash][noalloc]") {
  const char data[] = "abc";
  CATCH_REQUIRE(ERROR == utils_hash_djb(data, 3, nullptr));
}
