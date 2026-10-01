/**
 * \file
 *
 * \copyright 2017 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * Test of simple stdio library.
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#define CATCH_CONFIG_PREFIX_ALL
#include <cerrno>
#include <cstring>

#include <catch2/catch_test_macros.hpp>

#include "rcsw/core/core.h"
#include "rcsw/stdio/stdio.h"
#include "rcsw/stdio/string.h"

/*******************************************************************************
 * Helper Functions
 ******************************************************************************/
/*
 * Needed to get things to link with RCSW hidden visibility when built as an
 * .so; having this in the C test harness didn't work.
 */
// NOLINTNEXTLINE(misc-use-internal-linkage)
int th_putchar(int c) {
  (void)putchar(c);
  return c;
}
/*******************************************************************************
 * Test Cases
 ******************************************************************************/
// NOLINTNEXTLINE(readability-function-size)o
CATCH_TEST_CASE("Char Test", "[stdio][noalloc]") {
  CATCH_REQUIRE(RCSW_STDIO_ISUPPER('A') == 1);
  CATCH_REQUIRE(RCSW_STDIO_ISUPPER('Z') == 1);
  CATCH_REQUIRE(RCSW_STDIO_ISUPPER('a') == 0);
  CATCH_REQUIRE(RCSW_STDIO_ISUPPER('x') == 0);
  CATCH_REQUIRE(RCSW_STDIO_ISUPPER('Z') == 1);
  CATCH_REQUIRE(RCSW_STDIO_ISUPPER('1') == 0);
  CATCH_REQUIRE(RCSW_STDIO_ISUPPER('%') == 0);
  CATCH_REQUIRE(RCSW_STDIO_ISUPPER('E') == 1);

  CATCH_REQUIRE(RCSW_STDIO_ISLOWER('A') == 0);
  CATCH_REQUIRE(RCSW_STDIO_ISLOWER('Z') == 0);
  CATCH_REQUIRE(RCSW_STDIO_ISLOWER('a') == 1);
  CATCH_REQUIRE(RCSW_STDIO_ISLOWER('x') == 1);
  CATCH_REQUIRE(RCSW_STDIO_ISLOWER('Z') == 0);
  CATCH_REQUIRE(RCSW_STDIO_ISLOWER('1') == 0);
  CATCH_REQUIRE(RCSW_STDIO_ISLOWER('%') == 0);
  CATCH_REQUIRE(RCSW_STDIO_ISLOWER('e') == 1);

  CATCH_REQUIRE(RCSW_STDIO_ISSPACE(' ') == 1);
  CATCH_REQUIRE(RCSW_STDIO_ISSPACE('Z') == 0);

  CATCH_REQUIRE(RCSW_STDIO_ISALPHA('A') == 1);
  CATCH_REQUIRE(RCSW_STDIO_ISALPHA('X') == 1);
  CATCH_REQUIRE(RCSW_STDIO_ISALPHA('Z') == 1);
  CATCH_REQUIRE(RCSW_STDIO_ISALPHA('1') == 0);
  CATCH_REQUIRE(RCSW_STDIO_ISALPHA('%') == 0);
  CATCH_REQUIRE(RCSW_STDIO_ISALPHA('E') == 1);

  CATCH_REQUIRE(RCSW_STDIO_ISPRINTABLE('A') == 1);
  CATCH_REQUIRE(RCSW_STDIO_ISPRINTABLE('#') == 1);
  CATCH_REQUIRE(RCSW_STDIO_ISPRINTABLE('~') == 1);
  CATCH_REQUIRE(RCSW_STDIO_ISPRINTABLE(0) == 0);
  CATCH_REQUIRE(RCSW_STDIO_ISPRINTABLE('%') == 1);
  CATCH_REQUIRE(RCSW_STDIO_ISPRINTABLE(240) == 0);

  CATCH_REQUIRE(RCSW_STDIO_ISDIGIT('A') == 0);
  CATCH_REQUIRE(RCSW_STDIO_ISDIGIT('X') == 0);
  CATCH_REQUIRE(RCSW_STDIO_ISDIGIT('Z') == 0);
  CATCH_REQUIRE(RCSW_STDIO_ISDIGIT('1') == 1);
  CATCH_REQUIRE(RCSW_STDIO_ISDIGIT('%') == 0);
  CATCH_REQUIRE(RCSW_STDIO_ISDIGIT('e') == 0);

  CATCH_REQUIRE(stdio_puts("this is a test") == 14);

  for (int i = 0; i < 26; ++i) {
    CATCH_REQUIRE(stdio_toupper('a' + i) == 'A' + i);
    CATCH_REQUIRE(stdio_tolower('A' + i) == 'a' + i);
  } /* for(i..) */
} /* char_test() */

CATCH_TEST_CASE("Memory Test", "[stdio][noalloc]") {
  int data[10] = {0, 1, 2, 4, 10000, -12345, 17, 0x56789, RCSW_E9, -23};
  int dest[10];

  memset(dest, 0, sizeof(dest));
  CATCH_REQUIRE(stdio_memcpy(dest, data, sizeof(data)) == dest);
  CATCH_REQUIRE(memcmp(data, dest, sizeof(data)) == 0);
}

// NOLINTNEXTLINE(readability-function-size)
CATCH_TEST_CASE("Convert Test", "[stdio][noalloc]") {
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
  CATCH_REQUIRE(stdio_strcmp(stdio_itoax(0, buf, sizeof(buf), false), "0") == 0);
  CATCH_REQUIRE(stdio_strcmp(stdio_itoax(1, buf, sizeof(buf), false), "1") == 0);
  CATCH_REQUIRE(stdio_strcmp(stdio_itoax(100, buf, sizeof(buf), false), "64") ==
                0);
  CATCH_REQUIRE(
    stdio_strcmp(stdio_itoax(0xfe87, buf, sizeof(buf), false), "fe87") == 0);
  CATCH_REQUIRE(
    stdio_strcmp(stdio_itoax(0x234, buf, sizeof(buf), false), "234") == 0);
  CATCH_REQUIRE(stdio_strcmp(stdio_itoax(0x10000000, buf, sizeof(buf), false),
                             "10000000") == 0);
  CATCH_REQUIRE(stdio_strcmp(stdio_itoax(0xFFFFFFF, buf, sizeof(buf), false),
                             "fffffff") == 0);
  CATCH_REQUIRE(
    stdio_strcmp(stdio_itoax(0xFFFFFF, buf, sizeof(buf), false), "ffffff") == 0);
  CATCH_REQUIRE(
    stdio_strcmp(stdio_itoax(0xFFFFF, buf, sizeof(buf), false), "fffff") == 0);
  CATCH_REQUIRE(
    stdio_strcmp(stdio_itoax(0xFFFF, buf, sizeof(buf), false), "ffff") == 0);

  CATCH_REQUIRE(stdio_strcmp(stdio_itoax(0, buf, sizeof(buf), true), "0x0") == 0);
  CATCH_REQUIRE(stdio_strcmp(stdio_itoax(1, buf, sizeof(buf), true), "0x1") == 0);
  CATCH_REQUIRE(stdio_strcmp(stdio_itoax(100, buf, sizeof(buf), true), "0x64") ==
                0);
  CATCH_REQUIRE(
    stdio_strcmp(stdio_itoax(0xfe87, buf, sizeof(buf), true), "0xfe87") == 0);
  CATCH_REQUIRE(
    stdio_strcmp(stdio_itoax(0x234, buf, sizeof(buf), true), "0x234") == 0);

  CATCH_REQUIRE(stdio_strcmp(stdio_itoad(0, buf, sizeof(buf)), "0") == 0);
  CATCH_REQUIRE(stdio_strcmp(stdio_itoad(1, buf, sizeof(buf)), "+1") == 0);
  CATCH_REQUIRE(stdio_strcmp(stdio_itoad(-1, buf, sizeof(buf)), "-1") == 0);
  CATCH_REQUIRE(stdio_strcmp(stdio_itoad(10, buf, sizeof(buf)), "+10") == 0);
  CATCH_REQUIRE(stdio_strcmp(stdio_itoad(-123, buf, sizeof(buf)), "-123") == 0);
  CATCH_REQUIRE(stdio_strcmp(stdio_itoad(102393, buf, sizeof(buf)), "+102393") ==
                0);
}

CATCH_TEST_CASE("stdio_itoad formats INT32_MIN without overflow",
                "[stdio][noalloc]") {
  char buf[RCSW_STDIO_ITOAD_BUFSIZE];
  CATCH_REQUIRE(buf == stdio_itoad(INT32_MIN, buf, sizeof(buf)));
  CATCH_REQUIRE(0 == strcmp(buf, "-2147483648"));
}

CATCH_TEST_CASE("stdio_itoad/itoax respect the buffer size", "[stdio][noalloc]") {
  char buf[RCSW_STDIO_ITOAX_BUFSIZE];

  /* Exactly enough room */
  CATCH_REQUIRE(buf == stdio_itoad(-123, buf, 5));
  CATCH_REQUIRE(0 == strcmp(buf, "-123"));
  CATCH_REQUIRE(buf == stdio_itoad(0, buf, 2));
  CATCH_REQUIRE(0 == strcmp(buf, "0"));
  CATCH_REQUIRE(buf == stdio_itoax(0xFFFFFFFF, buf, sizeof(buf), true));
  CATCH_REQUIRE(0 == strcmp(buf, "0xffffffff"));
  CATCH_REQUIRE(buf == stdio_itoax(0xabc, buf, 4, false));
  CATCH_REQUIRE(0 == strcmp(buf, "abc"));

  /* One byte short: refused, buffer untouched */
  memset(buf, 'z', sizeof(buf));
  errno = 0;
  CATCH_REQUIRE(nullptr == stdio_itoad(-123, buf, 4));
  CATCH_REQUIRE(ENOSPC == errno);
  errno = 0;
  CATCH_REQUIRE(nullptr == stdio_itoad(0, buf, 1));
  CATCH_REQUIRE(ENOSPC == errno);
  errno = 0;
  CATCH_REQUIRE(nullptr == stdio_itoax(0xabc, buf, 5, true));
  CATCH_REQUIRE(ENOSPC == errno);
  for (char c : buf) {
    CATCH_REQUIRE('z' == c);
  }
}

CATCH_TEST_CASE("stdio_atoi base 16 accepts input without a 0x prefix",
                "[stdio][noalloc]") {
  CATCH_REQUIRE(255 == stdio_atoi("ff", 16));
  CATCH_REQUIRE(255 == stdio_atoi("0xff", 16));
}

CATCH_TEST_CASE("stdio_atoi stops at digits that are invalid for the base",
                "[stdio][noalloc]") {
  CATCH_REQUIRE(1 == stdio_atoi("19", 8));
  CATCH_REQUIRE(5 == stdio_atoi("5a", 10));
}
