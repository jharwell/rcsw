/**
 * \file utils-test.cpp
 *
 * Test of various things in the utils library.
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_PREFIX_ALL
#include <catch.hpp>

#include "rcsw/utils/utils.h"
#include "rcsw/utils/checksum.h"
#include "rcsw/utils/hash.h"
#include "rcsw/utils/mem.h"
#include "rcsw/utils/time.h"
#include "rcsw/al/clock.h"

#include "tests/ds_test.h"
#include "tests/ds_test.hpp"

/*******************************************************************************
 * Callbacks
 ******************************************************************************/
static size_t arr32_permute_count = 0;

static void arr32_permute_cb(uint32_t*) {
  ++arr32_permute_count;
} /* arr32_permute_cb() */

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("Bit Reverse Test", "[utils]") {
  uint8_t foo8 = 0x1;
  uint16_t foo16 = 0x1;
  uint32_t foo32 = 0x1;

  CATCH_REQUIRE(RCSW_REV8(foo8) == 0x80);
  CATCH_REQUIRE(RCSW_REV16(foo16) == 0x8000);
  CATCH_REQUIRE(RCSW_REV32(foo32) == 0x80000000);

  CATCH_REQUIRE(RCSW_REVL8(foo8) == 0x80);
  CATCH_REQUIRE(RCSW_REVL16(foo16) == 0x8000);
  CATCH_REQUIRE(RCSW_REVL32(foo32) == 0x80000000);

  foo8 = 0x80;
  foo16 = 0x8000;
  foo32 = 0x80000000;

  CATCH_REQUIRE(RCSW_REV8(foo8) == 0x1);
  CATCH_REQUIRE(RCSW_REV16(foo16) == 0x1);
  CATCH_REQUIRE(RCSW_REV32(foo32) == 0x1);

  CATCH_REQUIRE(RCSW_REVL8(foo8) == 0x1);
  CATCH_REQUIRE(RCSW_REVL16(foo16) == 0x1);
  CATCH_REQUIRE(RCSW_REVL32(foo32) == 0x1);
};

CATCH_TEST_CASE("CRC Test", "[utils]") {
  uint8_t data[] = { 0x4d,  0x54,  0x30, 0x30, 0x01, 0x02, 0x03, 0x04 };
  uint32_t crc = crc32_eth(data, 8);
  CATCH_REQUIRE(RCSW_BSWAP32(crc) == 0xb5e0332);

  crc = crc32_ethl(data, 8);
  CATCH_REQUIRE(RCSW_BSWAP32(crc) == 0x0b5e0332);
  };

CATCH_TEST_CASE("Bitswap Test", "[utils]") {
  uint16_t foo16 = 0x1;
  uint32_t foo32 = 0x1;
  uint64_t foo64 = 0x1;

  CATCH_REQUIRE(RCSW_BSWAP16(foo16) == 0x0100);
  CATCH_REQUIRE(RCSW_BSWAP32(foo32) == 0x01000000);
  CATCH_REQUIRE(RCSW_BSWAP64(foo64) == 0x0100000000000000);

  foo16 = 0x8000;
  foo32 = 0x80000000;
  foo64 = 0x8000000000000000;

  CATCH_REQUIRE(RCSW_BSWAP16(foo16) == 0x80);
  CATCH_REQUIRE(RCSW_BSWAP32(foo32) == 0x80);
  CATCH_REQUIRE(RCSW_BSWAP64(foo64) == 0x80);

  foo16 = 0x0123;
  foo32 = 0x01234567;
  foo64 = 0x0123456789ABCDEF;

  CATCH_REQUIRE(RCSW_BSWAP16(foo16) == 0x2301);
  CATCH_REQUIRE(RCSW_BSWAP32(foo32) == 0x67452301);
  CATCH_REQUIRE(RCSW_BSWAP64(foo64) == 0xEFCDAB8967452301);
};

CATCH_TEST_CASE("Bitmask Test", "[utils]") {
  uint16_t foo16 = 0x1234;
  uint32_t foo32 = 0x34567890;
  uint64_t foo64 = 0xFFAABBEE00112233;

  CATCH_REQUIRE(RCSW_M32U16(foo32) == 0x34560000);
  CATCH_REQUIRE(RCSW_M32L16(foo32) == 0x7890);

  CATCH_REQUIRE(RCSW_M64U32(foo64) == 0xFFAABBEE00000000);
  CATCH_REQUIRE(RCSW_M64L32(foo64) == 0x00112233);
};

CATCH_TEST_CASE("Comparison Test", "[utils]") {
  CATCH_REQUIRE(RCSW_IS_BETWEENC(0, 0, 1));
  CATCH_REQUIRE(RCSW_IS_BETWEENC(1, 0, 1));

  CATCH_REQUIRE(!RCSW_IS_BETWEENO(0, 0, 1));
  CATCH_REQUIRE(!RCSW_IS_BETWEENO(1, 0, 1));

  CATCH_REQUIRE(RCSW_IS_BETWEENHO(0, 0, 1));
  CATCH_REQUIRE(!RCSW_IS_BETWEENHO(1, 0, 1));
};

CATCH_TEST_CASE("mem_dump Test", "[utils]") {
    uint8_t buf [] = {
        0xF, 0xE, 0x17, 0x14, 0x3, 0x78, 0xFF, 0x46,
        0xF, 0xE, 0x17, 0x14, 0x3, 0x78, 0xFF, 0x46,
        0xF, 0xE, 0x17, 0x14, 0x3, 0x78, 0xFF, 0x46,
        0xF, 0xE, 0x17, 0x14, 0x3, 0x78, 0xFF, 0x46,
        0xF, 0xE, 0x17, 0x14, 0x3, 0x78, 0xFF, 0x46,
        0xF, 0xE, 0x17, 0x14, 0x3, 0x78, 0xFF, 0x46,
        0xF, 0xE, 0x17, 0x14, 0x3, 0x78, 0xFF, 0x46,
        0xF, 0xE, 0x17, 0x14, 0x3, 0x78, 0xFF, 0x46,
        0xF, 0xE, 0x17, 0x14, 0x3, 0x78, 0xFF, 0x46,
        0xF, 0xE, 0x17, 0x14, 0x3, 0x78, 0xFF, 0x46,
        0xF, 0xE, 0x17, 0x14, 0x3, 0x78, 0xFF, 0x46,
        0xF, 0xE, 0x17, 0x14, 0x3, 0x78, 0xFF, 0x46,
        0xF, 0xE, 0x17, 0x14, 0x3, 0x78, 0xFF, 0x46,
        0xF, 0xE, 0x17, 0x14, 0x3, 0x78, 0xFF, 0x46,
        0xF, 0xE, 0x17, 0x14, 0x3, 0x78, 0xFF, 0x46,
    };
    mem_dump8((uint8_t*)buf, sizeof(buf));
    printf("\n");
    mem_dump16((uint16_t*)buf, sizeof(buf)/2);
    printf("\n");
    mem_dump32((uint32_t*)buf, sizeof(buf)/4);
    printf("\n");
    mem_dump8v((uint8_t*)buf, sizeof(buf));
    printf("\n");
    mem_dump16v((uint16_t*)buf, sizeof(buf)/2);
    printf("\n");
    mem_dump32v((uint32_t*)buf, sizeof(buf)/4);
};

CATCH_TEST_CASE("mem_copy Test", "[utils]") {
  uint8_t buf1[1000];
  uint8_t buf2[1000];

  unsigned i;
  for (i = 4; i < sizeof(buf1)/sizeof(uint32_t); i+=4) {
    memset(buf1, 0, sizeof(buf1));
    memset(buf2, 0, sizeof(buf1));
    util_string_gen((char*)buf1, i);
    mem_cpy32(buf2, buf1, i);
    CATCH_REQUIRE(memcmp(buf1, buf2, sizeof(buf2)) == 0);
  } /* for() */
};

CATCH_TEST_CASE("mem_bswap Test", "[utils]") {
  uint16_t in16[4] = {
    0x1234,
    0x2345,
    0x3456,
    0x4567
  };

  uint16_t out16[4] = {
    0x3412,
    0x4523,
    0x5634,
    0x6745
  };

  uint32_t in32[4] = {
    0x00001234,
    0x00002345,
    0x00003456,
    0x00004567
  };

  uint32_t out32[4] = {
    0x34120000,
    0x45230000,
    0x56340000,
    0x67450000
  };

  CATCH_REQUIRE(OK == mem_bswap16(in16, RCSW_ARRAY_ELTS(in16)));
  CATCH_REQUIRE(0 == memcmp(in16, out16, RCSW_ARRAY_ELTS(in16)));

  CATCH_REQUIRE(OK == mem_bswap32(in32, RCSW_ARRAY_ELTS(in32)));
  CATCH_REQUIRE(0 == memcmp(in32, out32, RCSW_ARRAY_ELTS(in32)));
}

CATCH_TEST_CASE("time Test", "[utils]") {
  struct timespec t1 = {
    .tv_sec = 1,
    .tv_nsec = 100
  };

  struct timespec t2 = {
    .tv_sec = 5,
    .tv_nsec = 800
  };

  CATCH_REQUIRE(time_ts_cmp(&t1, &t2) < 0);
  CATCH_REQUIRE(time_ts_cmp(&t2, &t1) > 0);
  CATCH_REQUIRE(time_ts_cmp(&t2, &t2) == 0);

  struct timespec t3 = t1;
  time_ts_add(&t3, &t1);
  CATCH_REQUIRE(t3.tv_sec == 2);
  CATCH_REQUIRE(t3.tv_nsec == 200);

  t3.tv_sec = 0;
  t3.tv_nsec = 0;

  time_ts_diff(&t1, &t2, &t3);

  CATCH_REQUIRE(t3.tv_sec == 4);
  CATCH_REQUIRE(t3.tv_nsec == 700);

  CATCH_REQUIRE(time_ts_make_abs(&t1, &t2) == OK);

  struct timespec ts1 = clock_monotime();
  sleep(1);
  struct timespec ts2 = clock_monotime();
  CATCH_REQUIRE(time_ts2mono(&ts2) > time_ts2mono(&ts1));
}

CATCH_TEST_CASE("checksum Test", "[utils]") {
  th::element_set<struct element1> data(100);
  data.data_gen();

  /*
   * Not really tests, more just to get code coverage and uncover memory
   * alignment issues on embedded platforms.
   */
  CATCH_REQUIRE(xchks8((uint8_t*)data.elts.data(), data.elts.size(), 0) > 0);
  CATCH_REQUIRE(xchks16((uint16_t*)data.elts.data(), data.elts.size(), 0) > 0);
  CATCH_REQUIRE(xchks32((uint32_t*)data.elts.data(), data.elts.size(), 0) > 0);
  CATCH_REQUIRE(achks8((uint8_t*)data.elts.data(), data.elts.size(), 0) > 0);
  CATCH_REQUIRE(achks16((uint16_t*)data.elts.data(), data.elts.size(), 0) > 0);
  CATCH_REQUIRE(achks32((uint32_t*)data.elts.data(), data.elts.size(), 0) > 0);
  CATCH_REQUIRE(crc32_brown((uint8_t*)data.elts.data(), 0, data.elts.size()) > 0);
  CATCH_REQUIRE(hash_djb(data.elts.data(), data.elts.size()) > 0);
}

CATCH_TEST_CASE("misc Test", "[utils]") {
  CATCH_REQUIRE(util_reflect32(0xF0021001, 32) == 0x8008400F);
  CATCH_REQUIRE(util_reflect32(0xF0000001, 16) == 0x8000);

  uint8_t arr[5] = {0x5, 0x1, 0x2, 0x3, 0x4};

  arr8_reverse(arr, RCSW_ARRAY_ELTS(arr));

  uint8_t ret[5] = {0x4, 0x3, 0x2, 0x1, 0x5};
  CATCH_REQUIRE(memcmp(arr, ret, RCSW_ARRAY_ELTS(arr)) == 0);

  uint32_t arr2[4] = {0x4, 0x1, 0x2, 0x3};
  arr32_elt_swap(arr2, 0, 1);
  arr32_elt_swap(arr2, 2, 3);

  uint32_t ret2[4] = {0x1, 0x4, 0x3, 0x2};
  CATCH_REQUIRE(memcmp(arr2, ret2, RCSW_ARRAY_ELTS(arr)) == 0);

  uint32_t arr3[4] = {0x4, 0x1, 0x2, 0x3};
  arr32_permute(arr3, RCSW_ARRAY_ELTS(arr3), 0, arr32_permute_cb);

  CATCH_REQUIRE(arr32_permute_count == 24);
}
