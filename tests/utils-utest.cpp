/**
 * @file utils-test.cpp
 *
 * Test of various things in the utils library.
 *
 * @copyright 2017 John Harwell, All rights reserved.
 *
 * This file is part of RCSW.
 *
 * RCSW is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * RCSW is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * RCSW.  If not, see <http://www.gnu.org/licenses/
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

/******************************************************************************
 * Constant definitions
 *****************************************************************************/
#define NUM_TESTS        5

/*******************************************************************************
 * Test Function Forward Declarations
 ******************************************************************************/
static void bit_rev_test(void);
static void crc_test(void);
static void bswap_test(void);
static void mem_dump_test(void);
static void copy_test(void);

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("Bit Reverse Test", "[utils]") { bit_rev_test(); };
CATCH_TEST_CASE("CRC Test", "[utils]") { crc_test(); };
CATCH_TEST_CASE("bswap Test", "[utils]") { bswap_test(); };
CATCH_TEST_CASE("mem_dump Test", "[utils]") { mem_dump_test(); };
CATCH_TEST_CASE("copy Test", "[utils]") { copy_test(); };

/*******************************************************************************
 * Test Functions
 ******************************************************************************/
static void bit_rev_test(void) {
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
} /* bit_rev_test() */

static void crc_test(void) {
    uint8_t data[] = { 0x4d,  0x54,  0x30, 0x30, 0x01, 0x02, 0x03, 0x04 };
    uint32_t crc = crc32_eth(data, 8);
    CATCH_REQUIRE(RCSW_BSWAP32(crc) == 0xb5e0332);

    crc = crc32_ethl(data, 8);
    CATCH_REQUIRE(RCSW_BSWAP32(crc) == 0x0b5e0332);
} /* crc_test() */

static void bswap_test(void) {
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
} /* bswap_test() */

static void mem_dump_test(void) {
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
} /* mem_dump_test() */

static void copy_test(void) {
    uint8_t buf1[1000];
    uint8_t buf2[1000];

    unsigned i;
    for (i = 4; i < sizeof(buf1)/sizeof(uint32_t); i+=4) {
        memset(buf1, 0, sizeof(buf1));
        memset(buf2, 0, sizeof(buf1));
        string_gen((char*)buf1, i);
        mem_cpy32(buf2, buf1, i);
        CATCH_REQUIRE(memcmp(buf1, buf2, sizeof(buf2)) == 0);
    } /* for() */
} /* copy_test() */
