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
  CATCH_REQUIRE(stdio_isupper('A') == 1);
  CATCH_REQUIRE(stdio_isupper('Z') == 1);
  CATCH_REQUIRE(stdio_isupper('a') == 0);
  CATCH_REQUIRE(stdio_isupper('x') == 0);
  CATCH_REQUIRE(stdio_isupper('Z') == 1);
  CATCH_REQUIRE(stdio_isupper('1') == 0);
  CATCH_REQUIRE(stdio_isupper('%') == 0);
  CATCH_REQUIRE(stdio_isupper('E') == 1);

  CATCH_REQUIRE(stdio_islower('A') == 0);
  CATCH_REQUIRE(stdio_islower('Z') == 0);
  CATCH_REQUIRE(stdio_islower('a') == 1);
  CATCH_REQUIRE(stdio_islower('x') == 1);
  CATCH_REQUIRE(stdio_islower('Z') == 0);
  CATCH_REQUIRE(stdio_islower('1') == 0);
  CATCH_REQUIRE(stdio_islower('%') == 0);
  CATCH_REQUIRE(stdio_islower('e') == 1);

  CATCH_REQUIRE(stdio_isspace(' ') == 1);
  CATCH_REQUIRE(stdio_isspace('Z') == 0);

  CATCH_REQUIRE(stdio_isalpha('A') == 1);
  CATCH_REQUIRE(stdio_isalpha('X') == 1);
  CATCH_REQUIRE(stdio_isalpha('Z') == 1);
  CATCH_REQUIRE(stdio_isalpha('1') == 0);
  CATCH_REQUIRE(stdio_isalpha('%') == 0);
  CATCH_REQUIRE(stdio_isalpha('E') == 1);

  CATCH_REQUIRE(stdio_isprintable('A') == 1);
  CATCH_REQUIRE(stdio_isprintable('#') == 1);
  CATCH_REQUIRE(stdio_isprintable('~') == 1);
  CATCH_REQUIRE(stdio_isprintable(0) == 0);
  CATCH_REQUIRE(stdio_isprintable('%') == 1);
  CATCH_REQUIRE(stdio_isprintable(240) == 0);

  CATCH_REQUIRE(stdio_isdigit('A') == 0);
  CATCH_REQUIRE(stdio_isdigit('X') == 0);
  CATCH_REQUIRE(stdio_isdigit('Z') == 0);
  CATCH_REQUIRE(stdio_isdigit('1') == 1);
  CATCH_REQUIRE(stdio_isdigit('%') == 0);
  CATCH_REQUIRE(stdio_isdigit('e') == 0);

  CATCH_REQUIRE(stdio_ishex('0') == 1);
  CATCH_REQUIRE(stdio_ishex('9') == 1);
  CATCH_REQUIRE(stdio_ishex('a') == 1);
  CATCH_REQUIRE(stdio_ishex('F') == 1);
  CATCH_REQUIRE(stdio_ishex('g') == 0);
  CATCH_REQUIRE(stdio_ishex('G') == 0);
  CATCH_REQUIRE(stdio_ishex('%') == 0);

  /* Negative (signed char) values are in no class */
  CATCH_REQUIRE(stdio_isprintable(-16) == 0);
  CATCH_REQUIRE(stdio_isalpha(-16) == 0);

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
