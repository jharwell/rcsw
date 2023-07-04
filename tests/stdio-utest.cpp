/**
 * \file stdio-test.cpp
 *
 * Test of simple stdio library.
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

#include "rcsw/stdio/stdio.h"
#include "rcsw/stdio/string.h"

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("Char Test", "[stdio]") {
  CATCH_REQUIRE(STDIO_ISUPPER('A') == 1);
  CATCH_REQUIRE(STDIO_ISUPPER('Z') == 1);
  CATCH_REQUIRE(STDIO_ISUPPER('a') == 0);
  CATCH_REQUIRE(STDIO_ISUPPER('x') == 0);
  CATCH_REQUIRE(STDIO_ISUPPER('Z') == 1);
  CATCH_REQUIRE(STDIO_ISUPPER('1') == 0);
  CATCH_REQUIRE(STDIO_ISUPPER('%') == 0);
  CATCH_REQUIRE(STDIO_ISUPPER('E') == 1);

  CATCH_REQUIRE(STDIO_ISLOWER('A') == 0);
  CATCH_REQUIRE(STDIO_ISLOWER('Z') == 0);
  CATCH_REQUIRE(STDIO_ISLOWER('a') == 1);
  CATCH_REQUIRE(STDIO_ISLOWER('x') == 1);
  CATCH_REQUIRE(STDIO_ISLOWER('Z') == 0);
  CATCH_REQUIRE(STDIO_ISLOWER('1') == 0);
  CATCH_REQUIRE(STDIO_ISLOWER('%') == 0);
  CATCH_REQUIRE(STDIO_ISLOWER('e') == 1);

  CATCH_REQUIRE(STDIO_ISSPACE(' ') == 1);
  CATCH_REQUIRE(STDIO_ISSPACE('Z') == 0);

  CATCH_REQUIRE(STDIO_ISALPHA('A') == 1);
  CATCH_REQUIRE(STDIO_ISALPHA('X') == 1);
  CATCH_REQUIRE(STDIO_ISALPHA('Z') == 1);
  CATCH_REQUIRE(STDIO_ISALPHA('1') == 0);
  CATCH_REQUIRE(STDIO_ISALPHA('%') == 0);
  CATCH_REQUIRE(STDIO_ISALPHA('E') == 1);

  CATCH_REQUIRE(STDIO_ISPRINTABLE('A') == 1);
  CATCH_REQUIRE(STDIO_ISPRINTABLE('#') == 1);
  CATCH_REQUIRE(STDIO_ISPRINTABLE('~') == 1);
  CATCH_REQUIRE(STDIO_ISPRINTABLE(0) == 0);
  CATCH_REQUIRE(STDIO_ISPRINTABLE('%') == 1);
  CATCH_REQUIRE(STDIO_ISPRINTABLE(240) == 0);

  CATCH_REQUIRE(STDIO_ISDIGIT('A') == 0);
  CATCH_REQUIRE(STDIO_ISDIGIT('X') == 0);
  CATCH_REQUIRE(STDIO_ISDIGIT('Z') == 0);
  CATCH_REQUIRE(STDIO_ISDIGIT('1') == 1);
  CATCH_REQUIRE(STDIO_ISDIGIT('%') == 0);
  CATCH_REQUIRE(STDIO_ISDIGIT('e') == 0);

  CATCH_REQUIRE(stdio_puts("this is a test") == 14);

  for (int i = 0; i < 26; ++i) {
    CATCH_REQUIRE(stdio_toupper('a' + i) == 'A' + i);
    CATCH_REQUIRE(stdio_tolower('A' + i) == 'a' + i);
  } /* for(i..) */
} /* char_test() */

CATCH_TEST_CASE("Memory Test", "[stdio]") {
  int data[10] = {0, 1, 2, 4, 10000, -12345, 17, 0x56789, ONEE9, -23};
  int dest[10];

  memset(dest, 0, sizeof(dest));
  CATCH_REQUIRE(stdio_memcpy(dest, data, sizeof(data)) == dest);
  CATCH_REQUIRE(memcmp(data, dest, sizeof(data)) == 0);
}

CATCH_TEST_CASE("Convert Test", "[stdio]") {
  CATCH_REQUIRE(stdio_atoi("0", 10) == 0);
  CATCH_REQUIRE(stdio_atoi("0x0", 16) == 0);
  CATCH_REQUIRE(stdio_atoi("0x100", 16) == 256);
  CATCH_REQUIRE(stdio_atoi("0xABCDEF", 16) == 0xABCDEF);
  CATCH_REQUIRE(stdio_atoi("1", 10) == 1);
  CATCH_REQUIRE(stdio_atoi("0x1", 16) == 1);
  CATCH_REQUIRE(stdio_atoi("1234", 10) == 1234);
  CATCH_REQUIRE(stdio_atoi("1234", 10) == 1234);
  CATCH_REQUIRE(stdio_atoi("-1234", 10) == -1234);
  CATCH_REQUIRE(stdio_atoi("+1234", 10) == 1234);
  CATCH_REQUIRE(stdio_atoi("0x1234", 16) == 0x1234);

  CATCH_REQUIRE(stdio_atoi("    1234", 10) == 1234);
  CATCH_REQUIRE(stdio_atoi("    0x1234", 16) == 0x1234);
  CATCH_REQUIRE(stdio_atoi("    -1234", 10) == -1234);
  CATCH_REQUIRE(stdio_atoi("    +1234", 10) == 1234);

  char buf[20];
  CATCH_REQUIRE(stdio_strcmp(stdio_itoax(0, buf, false),"0") == 0);
  CATCH_REQUIRE(stdio_strcmp(stdio_itoax(1, buf, false), "1") == 0);
  CATCH_REQUIRE(stdio_strcmp(stdio_itoax(100, buf, false), "64") == 0);
  CATCH_REQUIRE(stdio_strcmp(stdio_itoax(0xfe87, buf, false), "fe87") == 0);
  CATCH_REQUIRE(stdio_strcmp(stdio_itoax(0x234, buf, false), "234") == 0);
  CATCH_REQUIRE(stdio_strcmp(stdio_itoax(0x10000000, buf, false), "10000000") == 0);
  CATCH_REQUIRE(stdio_strcmp(stdio_itoax(0xFFFFFFF, buf, false), "fffffff") == 0);
  CATCH_REQUIRE(stdio_strcmp(stdio_itoax(0xFFFFFF, buf, false), "ffffff") == 0);
  CATCH_REQUIRE(stdio_strcmp(stdio_itoax(0xFFFFF, buf, false), "fffff") == 0);
  CATCH_REQUIRE(stdio_strcmp(stdio_itoax(0xFFFF, buf, false), "ffff") == 0);

  CATCH_REQUIRE(stdio_strcmp(stdio_itoax(0, buf, true),"0x0") == 0);
  CATCH_REQUIRE(stdio_strcmp(stdio_itoax(1, buf, true), "0x1") == 0);
  CATCH_REQUIRE(stdio_strcmp(stdio_itoax(100, buf, true), "0x64") == 0);
  CATCH_REQUIRE(stdio_strcmp(stdio_itoax(0xfe87, buf, true), "0xfe87") == 0);
  CATCH_REQUIRE(stdio_strcmp(stdio_itoax(0x234, buf, true), "0x234") == 0);

  CATCH_REQUIRE(stdio_strcmp(stdio_itoad(0, buf), "0") == 0);
  CATCH_REQUIRE(stdio_strcmp(stdio_itoad(1, buf), "+1") == 0);
  CATCH_REQUIRE(stdio_strcmp(stdio_itoad(-1, buf), "-1") == 0);
  CATCH_REQUIRE(stdio_strcmp(stdio_itoad(10, buf), "+10") == 0);
  CATCH_REQUIRE(stdio_strcmp(stdio_itoad(-123, buf), "-123") == 0);
  CATCH_REQUIRE(stdio_strcmp(stdio_itoad(102393, buf), "+102393") == 0);
}
