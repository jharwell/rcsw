/**
 * \file boot-utest.cpp
 *
 * Test of bootloader/bootstrap library.
 *
 * \copyright 2023 John Harwell, All rights reserved.
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
#include "rcsw/boot/minimon.h"

/*******************************************************************************
 * Helper Functions
 ******************************************************************************/
/*
 * Needed to get things to link with RCSW hidden visibility when built as an
 * .so; having this in the C test harness didn't work.
 */
int th_putchar(int c) {
  putchar(c);
  return c;
}

int th_getchar(void) {
  return getchar();
}

CATCH_TEST_CASE("minimon Test", "[boot]") {
  minimon_init(NULL);
  uint32_t * test = (uint32_t*)malloc(4);
  printf("test: %p\n", test);
  minimon_start();
}
