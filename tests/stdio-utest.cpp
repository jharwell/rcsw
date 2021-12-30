/**
 * @file stdio-test.cpp
 *
 * Test of simple stdio library.
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
#if !defined(__nos__)
#include <stdio.h>
#endif
#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_PREFIX_ALL
#include <catch.hpp>

#include "rcsw/stdio/sstdio.h"
#include "rcsw/stdio/sstring.h"
#include "rcsw/common/dbg.h"

/******************************************************************************
 * Constant definitions
 *****************************************************************************/
#define NUM_TESTS        5

/*******************************************************************************
 * Test Function Forward Declarations
 ******************************************************************************/
static void printf_basic_test(void);
static void printf_tricky_test(void);
static void printf_float_test(void);
static void char_test(void);
static void puts_test(void);
static void convert_test(void);

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("printf Basic Test", "[stdio]") { printf_basic_test(); };
CATCH_TEST_CASE("printf Tricky Test", "[stdio]") { printf_tricky_test(); };
CATCH_TEST_CASE("printf float Test", "[stdio]") { printf_float_test(); };
CATCH_TEST_CASE("Char Test", "[stdio]") { char_test(); };
CATCH_TEST_CASE("puts() Test", "[stdio]") {puts_test(); };
CATCH_TEST_CASE("Convert Test", "[stdio]") { convert_test(); };

/*******************************************************************************
 * Test Functions
 ******************************************************************************/
static void printf_basic_test(void) {
  sstdio_printf("This is a string. If this prints, then basic printing is ok.\n");
  sstdio_printf("\n***** Testing printing pos/neg decimal numbers *****\n");
  sstdio_printf("decimal number 13        : %d\n",13);
  sstdio_printf("decimal number 4004      : %d\n",4004);
  sstdio_printf("decimal number 12345678  : %d\n",12345678);
  sstdio_printf("decimal number 10000     : %d\n",10000);
  sstdio_printf("decimal number 0         : %d\n",0);
  sstdio_printf("decimal number 1         : %d\n",1);
  sstdio_printf("decimal number -7        : %d\n",-7);
  sstdio_printf("decimal number -13       : %d\n",-13);
  sstdio_printf("decimal number -4004     : %d\n",-4004);
  sstdio_printf("decimal number -12345678 : %d\n",-12345678);
  sstdio_printf("decimal number -10000    : %d\n",-10000);
  sstdio_printf("decimal number 0         : %d\n",0);
  sstdio_printf("decimal number -1        : %d\n",-1);

  sstdio_printf("\n***** Testing printing with char substitution *****\n");
  sstdio_printf("char A: %c\n",'A');
  sstdio_printf("char a: %c\n",'a');
  sstdio_printf("char Z: %c\n",'Z');
  sstdio_printf("char z: %c\n",'z');
  sstdio_printf("chars a b: %c %c\n",'a','b');
  sstdio_printf("chars A B c: %c %c %c\n",'A','B','c');
  sstdio_printf("chars Z Y X W: %c %c %c %c\n",'Z','Y','X','W');
  sstdio_printf("chars A Z Y B C: %c %c %c %c %c\n",'A','Z','Y','B','C');

  sstdio_printf("\n***** Test printing with string substitution *****\n");
  sstdio_printf("string \"A\": %s\n","A");
  sstdio_printf("string \"AB\": %s\n","AB");
  sstdio_printf("string \"ABC\": %s\n","ABC");
  sstdio_printf("strings \"A\" \"X\"in it: %s %s\n","A","X");
  sstdio_printf("strings \"AB\" \"XY\" \"QR\": %s %s %s\n","AB","XY","QR");

  sstdio_printf("\n***** Testing printing multiple decimal numbers in the same string *****\n");
  sstdio_printf("decimal numbers 123 234: %d %d\n",123,234);
  sstdio_printf("decimal numbers 123 234 345: %d %d %d\n",123,234,345);
  sstdio_printf("decimal numbers 123 234 345 456 567: %d %d %d %d %d\n",123,234,345,456,567);
  sstdio_printf("decimal numbers 123 234 345 456 567 678: %d %d %d %d %d %d\n",123,234,345,456,567,678);

  sstdio_printf("\n***** Testing printing positive hex numbers in lowercase *****\n");
  sstdio_printf("number 0       : %x\n",0);
  sstdio_printf("number 1       : %x\n",1);
  sstdio_printf("number 256     : %x\n",256);
  sstdio_printf("number 32      : %x\n",32);
  sstdio_printf("number 17      : %x\n",17);
  sstdio_printf("number 1028749 : %x\n",1028749);
  sstdio_printf("number 48750   : %x\n",48750);

  sstdio_printf("\n***** Testing printing positive hex numbers in uppercase *****\n");
  sstdio_printf("number 0       : %X\n",0);
  sstdio_printf("number 1       : %X\n",1);
  sstdio_printf("number 256     : %X\n",256);
  sstdio_printf("number 32      : %X\n",32);
  sstdio_printf("number 17      : %X\n",17);
  sstdio_printf("number 1028749 : %X\n",1028749);
  sstdio_printf("number 48750   : %X\n",48750);
} /* printf_basic_test() */

static void printf_tricky_test(void) {
  sstdio_printf("\n***** Testing printing positive hex numbers with filling *****\n");
  sstdio_printf("number 0 (zero fill to 4)        : %04x\n", 0);
  sstdio_printf("number 1 (zero fill to 2)        : %02x\n", 1);
  sstdio_printf("number 2 (zero fill to 3)        : %03X\n", 2);
  sstdio_printf("number 256 (zero fill to 8)      : %08x\n", 256);
  sstdio_printf("number 32 (space fill to 5)      : %5x\n", 32);
  sstdio_printf("number 17 (space fill to 6)      : %6X\n", 17);
  sstdio_printf("number 1028749 (space fill to 1) : %1x\n", 1028749);
  sstdio_printf("number 49750 (space fill to 3)   : %3X\n", 48750);

  sstdio_printf("\n***** Testing printing positive hex numbers for correct number of digits *****\n");
  sstdio_printf("number 0x1        : %0x\n", 0x1);
  sstdio_printf("number 0x12       : %0x\n", 0x12);
  sstdio_printf("number 0x123      : %0x\n", 0x123);
  sstdio_printf("number 0x1234     : %0x\n", 0x1234);
  sstdio_printf("number 0x12345    : %0x\n", 0x12345);
  sstdio_printf("number 0x123456   : %0x\n", 0x123456);
  sstdio_printf("number 0x1234567  : %0x\n", 0x1234567);
  sstdio_printf("number 0x12345678 : %0x\n", 0x12345678);

  sstdio_printf("\n***** Test printing with integers with space filling *****\n");
  sstdio_printf("number 1      (1 digit)  : %1d\n", 1);
  sstdio_printf("number -1     (2 digits) : %2d\n", -1);
  sstdio_printf("number 1      (3 digits) : %3d\n", 1);
  sstdio_printf("number -1     (4 digits) : %4d\n", -1);
  sstdio_printf("number 1      (5 digits) : %5d\n", 1);
  sstdio_printf("number -1     (6 digits) : %6d\n", -1);
  sstdio_printf("number 1      (7 digits) : %7d\n", 1);
  sstdio_printf("number -1     (8 digits) : %8d\n", -1);
  sstdio_printf("number 12     (2 digits) : %2d\n", 12);
  sstdio_printf("number -123   (3 digits) : %3d\n", -123);
  sstdio_printf("number 1234   (8 digits) : %8d\n", 1234);
  sstdio_printf("number -12345 (6 digits) : %6d\n", -12345);
  sstdio_printf("number 123    (6 digits) : %6d\n", 123);
  sstdio_printf("number -12    (7 digits) : %7d\n", -12);
  sstdio_printf("number 1345   (8 digits) : %8d\n", 1345);


  sstdio_printf("\n***** Test printing with integers with zero filling *****\n");
  sstdio_printf("number 1      (1 digit)  : %01d\n", 1);
  sstdio_printf("number -1     (2 digits) : %02d\n", -1);
  sstdio_printf("number 1      (3 digits) : %03d\n", 1);
  sstdio_printf("number -1     (4 digits) : %04d\n", -1);
  sstdio_printf("number 1      (5 digits) : %05d\n", 1);
  sstdio_printf("number -1     (6 digits) : %06d\n", -1);
  sstdio_printf("number 1      (7 digits) : %07d\n", 1);
  sstdio_printf("number -1     (8 digits) : %08d\n", -1);
  sstdio_printf("number 12     (2 digits) : %02d\n", 12);
  sstdio_printf("number -123   (3 digits) : %03d\n", -123);
  sstdio_printf("number 1234   (8 digits) : %08d\n", 1234);
  sstdio_printf("number -12345 (6 digits) : %06d\n", -12345);
  sstdio_printf("number 123    (6 digits) : %06d\n", 123);
  sstdio_printf("number -12    (7 digits) : %07d\n", -12);
  sstdio_printf("number 1345   (8 digits) : %08d\n", 1345);

  sstdio_printf("\n***** Test printing pointers *****\n");
  sstdio_printf("pointer 0x40000000  : %p\n", (void*)(0x40000000));
  sstdio_printf("pointer -0x40000000 : %p\n", (void*)(-0x40000000));
  sstdio_printf("pointer 0x0         : %p\n", (void*)(0x0));

  sstdio_printf("\n***** Test always printing the sign value *****\n");
  sstdio_printf("number 100                 : %+d\n", 100);
  sstdio_printf("number -100                : %+d\n", -100);
  sstdio_printf("number 100 (zero fill to 1): %+01d\n", 100);
  sstdio_printf("number 100 (zero fill to 2): %+02d\n", 100);
  sstdio_printf("number 100 (zero fill to 3): %+03d\n", 100);
  sstdio_printf("number 100 (zero fill to 4): %+04d\n", 100);
  sstdio_printf("number 100 (zero fill to 5): %+05d\n", 100);
  sstdio_printf("number 100 (zero fill to 6): %+06d\n", 100);
  sstdio_printf("number 100 (zero fill to 7): %+07d\n", 100);
  sstdio_printf("number 100 (zero fill to 8): %+08d\n", 100);

  sstdio_printf("number 100 (space fill to 1): %+1d\n", 100);
  sstdio_printf("number 100 (space fill to 2): %+2d\n", 100);
  sstdio_printf("number 100 (space fill to 3): %+3d\n", 100);
  sstdio_printf("number 100 (space fill to 4): %+4d\n", 100);
  sstdio_printf("number 100 (space fill to 5): %+5d\n", 100);
  sstdio_printf("number 100 (space fill to 6): %+6d\n", 100);
  sstdio_printf("number 100 (space fill to 7): %+7d\n", 100);
  sstdio_printf("number 100 (space fill to 8): %+8d\n", 100);

  sstdio_printf("number -100 (space fill to 1): %+1d\n", -100);
  sstdio_printf("number -100 (space fill to 2): %+2d\n", -100);
  sstdio_printf("number -100 (space fill to 3): %+3d\n", -100);
  sstdio_printf("number -100 (space fill to 4): %+4d\n", -100);
  sstdio_printf("number -100 (space fill to 5): %+5d\n", -100);
  sstdio_printf("number -100 (space fill to 6): %+6d\n", -100);
  sstdio_printf("number -100 (space fill to 7): %+7d\n", -100);
  sstdio_printf("number -100 (space fill to 8): %+8d\n", -100);

  sstdio_printf("number -100 (zero fill to 1): %+01d\n", -100);
  sstdio_printf("number -100 (zero fill to 2): %+02d\n", -100);
  sstdio_printf("number -100 (zero fill to 3): %+03d\n", -100);
  sstdio_printf("number -100 (zero fill to 4): %+04d\n", -100);
  sstdio_printf("number -100 (zero fill to 5): %+05d\n", -100);
  sstdio_printf("number -100 (zero fill to 6): %+06d\n", -100);
  sstdio_printf("number -100 (zero fill to 7): %+07d\n", -100);
  sstdio_printf("number -100 (zero fill to 8): %+08d\n", -100);

  sstdio_printf("number 10                 : %+d\n", 10);
  sstdio_printf("number -10                : %+d\n", -10);
  sstdio_printf("number 10 (zero fill to 1): %+01d\n", 10);
  sstdio_printf("number 10 (zero fill to 2): %+02d\n", 10);
  sstdio_printf("number 10 (zero fill to 3): %+03d\n", 10);
  sstdio_printf("number 10 (zero fill to 4): %+04d\n", 10);
  sstdio_printf("number 10 (zero fill to 5): %+05d\n", 10);
  sstdio_printf("number 10 (zero fill to 6): %+06d\n", 10);
  sstdio_printf("number 10 (zero fill to 7): %+07d\n", 10);
  sstdio_printf("number 10 (zero fill to 8): %+08d\n", 10);

  sstdio_printf("number 10 (space fill to 1): %+1d\n", 10);
  sstdio_printf("number 10 (space fill to 2): %+2d\n", 10);
  sstdio_printf("number 10 (space fill to 3): %+3d\n", 10);
  sstdio_printf("number 10 (space fill to 4): %+4d\n", 10);
  sstdio_printf("number 10 (space fill to 5): %+5d\n", 10);
  sstdio_printf("number 10 (space fill to 6): %+6d\n", 10);
  sstdio_printf("number 10 (space fill to 7): %+7d\n", 10);
  sstdio_printf("number 10 (space fill to 8): %+8d\n", 10);

  sstdio_printf("number -10 (space fill to 1): %+1d\n", -10);
  sstdio_printf("number -10 (space fill to 2): %+2d\n", -10);
  sstdio_printf("number -10 (space fill to 3): %+3d\n", -10);
  sstdio_printf("number -10 (space fill to 4): %+4d\n", -10);
  sstdio_printf("number -10 (space fill to 5): %+5d\n", -10);
  sstdio_printf("number -10 (space fill to 6): %+6d\n", -10);
  sstdio_printf("number -10 (space fill to 7): %+7d\n", -10);
  sstdio_printf("number -10 (space fill to 8): %+8d\n", -10);

  sstdio_printf("number -10 (zero fill to 1): %+01d\n", -10);
  sstdio_printf("number -10 (zero fill to 2): %+02d\n", -10);
  sstdio_printf("number -10 (zero fill to 3): %+03d\n", -10);
  sstdio_printf("number -10 (zero fill to 4): %+04d\n", -10);
  sstdio_printf("number -10 (zero fill to 5): %+05d\n", -10);
  sstdio_printf("number -10 (zero fill to 6): %+06d\n", -10);
  sstdio_printf("number -10 (zero fill to 7): %+07d\n", -10);
  sstdio_printf("number -10 (zero fill to 8): %+08d\n", -10);

  sstdio_printf("number 1                 : %+d\n", 1);
  sstdio_printf("number -1                : %+d\n", -1);
  sstdio_printf("number 1 (zero fill to 1): %+01d\n", 1);
  sstdio_printf("number 1 (zero fill to 2): %+02d\n", 1);
  sstdio_printf("number 1 (zero fill to 3): %+03d\n", 1);
  sstdio_printf("number 1 (zero fill to 4): %+04d\n", 1);
  sstdio_printf("number 1 (zero fill to 5): %+05d\n", 1);
  sstdio_printf("number 1 (zero fill to 6): %+06d\n", 1);
  sstdio_printf("number 1 (zero fill to 7): %+07d\n", 1);
  sstdio_printf("number 1 (zero fill to 8): %+08d\n", 1);

  sstdio_printf("number 1 (space fill to 1): %+1d\n", 1);
  sstdio_printf("number 1 (space fill to 2): %+2d\n", 1);
  sstdio_printf("number 1 (space fill to 3): %+3d\n", 1);
  sstdio_printf("number 1 (space fill to 4): %+4d\n", 1);
  sstdio_printf("number 1 (space fill to 5): %+5d\n", 1);
  sstdio_printf("number 1 (space fill to 6): %+6d\n", 1);
  sstdio_printf("number 1 (space fill to 7): %+7d\n", 1);
  sstdio_printf("number 1 (space fill to 8): %+8d\n", 1);

  sstdio_printf("number -1 (space fill to 1): %+1d\n", -1);
  sstdio_printf("number -1 (space fill to 2): %+2d\n", -1);
  sstdio_printf("number -1 (space fill to 3): %+3d\n", -1);
  sstdio_printf("number -1 (space fill to 4): %+4d\n", -1);
  sstdio_printf("number -1 (space fill to 5): %+5d\n", -1);
  sstdio_printf("number -1 (space fill to 6): %+6d\n", -1);
  sstdio_printf("number -1 (space fill to 7): %+7d\n", -1);
  sstdio_printf("number -1 (space fill to 8): %+8d\n", -1);

  sstdio_printf("number -1 (zero fill to 1): %+01d\n", -1);
  sstdio_printf("number -1 (zero fill to 2): %+02d\n", -1);
  sstdio_printf("number -1 (zero fill to 3): %+03d\n", -1);
  sstdio_printf("number -1 (zero fill to 4): %+04d\n", -1);
  sstdio_printf("number -1 (zero fill to 5): %+05d\n", -1);
  sstdio_printf("number -1 (zero fill to 6): %+06d\n", -1);
  sstdio_printf("number -1 (zero fill to 7): %+07d\n", -1);
  sstdio_printf("number -1 (zero fill to 8): %+08d\n", -1);

  printf("\n***** Test left justifying output *****\n");
  sstdio_printf("number 17 (fill to 4): **%-4d**\n", 17);
  sstdio_printf("number 0  (fill to 4): **%-4d**\n", 0);
  sstdio_printf("number 1  (fill to 4): **%-4d**\n", 1);
  sstdio_printf("number 17 (fill to 1): **%-1d**\n", 17);
  sstdio_printf("number 17 (fill to 2): **%-2d**\n", 17);
  sstdio_printf("number 17 (fill to 8): **%-8d**\n", 17);

  printf("\n***** Test left justifying output while always showing the sign *****\n");
  sstdio_printf("number 17 (fill to 4): **%+-4d**\n", 17);
  sstdio_printf("number 0  (fill to 4): **%+-4d**\n", 0);
  sstdio_printf("number 1  (fill to 4): **%+-4d**\n", 1);
  sstdio_printf("number 17 (fill to 1): **%+-1d**\n", 17);
  sstdio_printf("number 17 (fill to 2): **%+-2d**\n", 17);
  sstdio_printf("number 17 (fill to 8): **%+-8d**\n", 17);

  sstdio_printf("number -17 (fill to 4): **%+-4d**\n", -17);
  sstdio_printf("number -1  (fill to 4): **%+-4d**\n", -1);
  sstdio_printf("number -17 (fill to 1): **%+-1d**\n", -17);
  sstdio_printf("number -17 (fill to 2): **%+-2d**\n", -17);
  sstdio_printf("number -17 (fill to 8): **%+-8d**\n", -17);
} /* printf_tricky_test() */

static void printf_float_test(void) {
  printf("***** Test basic floating point printing *****\n");
  sstdio_printf("float     17 : %f\n", 17.0);
  sstdio_printf("float     42 : %f\n", 42.0);
  sstdio_printf("float    1.1 : %f\n", 1.1);
  sstdio_printf("float   -1.1 : %f\n", -1.1);
  sstdio_printf("float   -0.1 : %f\n", -0.1);
  sstdio_printf("float    0.1 : %f\n", 0.1);
  sstdio_printf("float 0.1234 : %f\n", 0.1234);
  sstdio_printf("float -0.1234: %f\n", -0.1234);

  printf("\n***** Test floating point exponential notation *****\n");
  sstdio_printf("float 1000000000000000   : %f\n", 1000000000000000.0);
  sstdio_printf("float 123456789123456    : %f\n", 123456789123456.0);
  sstdio_printf("float -984840404943093   : %f\n", -984840404943093.0);
  sstdio_printf("float 0.0000000000000003 : %f\n", 0.000000000000003);

  printf("\n***** Test forcing exponential notation *****\n");
  sstdio_printf("float 1.37: %e\n", 1.37);
  sstdio_printf("float 0.37: %e\n", 0.37);
  sstdio_printf("float 0   : %e\n", 0.0);
  sstdio_printf("float -1  : %E\n", -1.0);
  sstdio_printf("float 1   : %E\n", 1.0);
  sstdio_printf("float 0.1 : %E\n", 0.1);
  sstdio_printf("float 3.0 : %E\n", 3.0);

  printf("\n***** Test printing a certain number decimal digits w/%%e *****\n");
  sstdio_printf("float 0.37 (8 decimals): %.8e\n", 0.37);
  sstdio_printf("float 0.37 (7 decimals): %.7e\n", 0.37);
  sstdio_printf("float 0.37 (6 decimals): %.6e\n", 0.37);
  sstdio_printf("float 0.37 (5 decimals): %.5e\n", 0.37);
  sstdio_printf("float 0.37 (4 decimals): %.4e\n", 0.37);
  sstdio_printf("float 0.37 (3 decimals): %.3e\n", 0.37);
  sstdio_printf("float 0.37 (2 decimals): %.2e\n", 0.37);
  sstdio_printf("float 0.37 (1 decimals): %.1e\n", 0.37);

  printf("\n***** Test correct rounding when printing a certain number of decimal digits w/%%e *****\n");
  sstdio_printf("float 0.12345678   (8 decimals): %.8e\n", 0.12345678);
  sstdio_printf("float 0.123456780  (8 decimals): %.8e\n", 0.123456780);
  sstdio_printf("float 0.1234567814 (8 decimals): %.8e\n", 0.1234567814);
  sstdio_printf("float 0.1234567815 (8 decimals): %.8e\n", 0.1234567815);

  sstdio_printf("float 0.12345678   (4 decimals): %.4e\n", 0.12345678);
  sstdio_printf("float 0.123456780  (4 decimals): %.4e\n", 0.123456780);
  sstdio_printf("float 0.1234567814 (4 decimals): %.4e\n", 0.1234567814);
  sstdio_printf("float 0.1234567815 (4 decimals): %.4e\n", 0.1234567815);

  sstdio_printf("float 0.12345678   (1 decimals): %.1e\n", 0.12345678);
  sstdio_printf("float 0.124456780  (1 decimals): %.1e\n", 0.124456780);
  sstdio_printf("float 0.1254567814 (1 decimals): %.1e\n", 0.1254567814);
  sstdio_printf("float 0.1264567815 (1 decimals): %.1e\n", 0.1264567815);

  sstdio_printf("float 123456.78    (1 decimals): %.1e\n", 123456.78);
  sstdio_printf("float 123456.78    (2 decimals): %.2e\n", 123456.78);
  sstdio_printf("float 1244567.80   (1 decimals): %.1e\n", 1244567.80);
  sstdio_printf("float 1244567.80   (2 decimals): %.2e\n", 1244567.80);
  sstdio_printf("float 1254567.814  (3 decimals): %.3e\n", 1254567.814);
  sstdio_printf("float 12645678.15  (1 decimals): %.1e\n", 12645678.15);

  printf("\n***** Test printing a certain number decimal digits w/%%f *****\n");
  sstdio_printf("float 0.37 (8 decimals): %.8f\n", 0.37);
  sstdio_printf("float 0.37 (7 decimals): %.7f\n", 0.37);
  sstdio_printf("float 0.37 (6 decimals): %.6f\n", 0.37);
  sstdio_printf("float 0.37 (5 decimals): %.5f\n", 0.37);
  sstdio_printf("float 0.37 (4 decimals): %.4f\n", 0.37);
  sstdio_printf("float 0.37 (3 decimals): %.3f\n", 0.37);
  sstdio_printf("float 0.37 (2 decimals): %.2f\n", 0.37);
  sstdio_printf("float 0.37 (1 decimals): %.1f\n", 0.37);

  printf("\n***** Test correct rounding when printing a certain number of decimal digits w/%%f *****\n");
  sstdio_printf("float 0.12345678   (8 decimals): %.8f\n", 0.12345678);
  sstdio_printf("float 0.123456780  (8 decimals): %.8f\n", 0.123456780);
  sstdio_printf("float 0.123456784  (8 decimals): %.8f\n", 0.123456784);
  sstdio_printf("float 0.123456785  (8 decimals): %.8f\n", 0.123456785);

  sstdio_printf("float 0.12345678   (4 decimals): %.4f\n", 0.12345678);
  sstdio_printf("float 0.123456780  (4 decimals): %.4f\n", 0.123456780);
  sstdio_printf("float 0.123456784  (4 decimals): %.4f\n", 0.123456784);
  sstdio_printf("float 0.123456785  (4 decimals): %.4f\n", 0.123456785);

  sstdio_printf("float 0.12345678   (1 decimals): %.1f\n", 0.12345678);
  sstdio_printf("float 0.124456780  (1 decimals): %.1f\n", 0.124456780);
  sstdio_printf("float 0.15456784   (1 decimals): %.1f\n", 0.15456784);
  sstdio_printf("float 0.126456785  (1 decimals): %.1f\n", 0.126456785);
} /* printf_float_test() */

static void char_test(void) {

  CATCH_CHECK(sstring_isupper('A') == 1);
  CATCH_CHECK(sstring_isupper('Z') == 1);
  CATCH_CHECK(sstring_isupper('a') == 0);
  CATCH_CHECK(sstring_isupper('x') == 0);
  CATCH_CHECK(sstring_isupper('Z') == 1);
  CATCH_CHECK(sstring_isupper('1') == 0);
  CATCH_CHECK(sstring_isupper('%') == 0);
  CATCH_CHECK(sstring_isupper('e') == 0);

  CATCH_CHECK(sstring_islower('A') == 0);
  CATCH_CHECK(sstring_islower('Z') == 0);
  CATCH_CHECK(sstring_islower('a') == 1);
  CATCH_CHECK(sstring_islower('x') == 1);
  CATCH_CHECK(sstring_islower('Z') == 0);
  CATCH_CHECK(sstring_islower('1') == 0);
  CATCH_CHECK(sstring_islower('%') == 0);
  CATCH_CHECK(sstring_islower('e') == 1);

  CATCH_CHECK(sstring_isspace(' ') == 1);
  CATCH_CHECK(sstring_isspace('Z') == 0);

  CATCH_CHECK(sstring_isalpha('a') == 1);
  CATCH_CHECK(sstring_isalpha('x') == 1);
  CATCH_CHECK(sstring_isalpha('Z') == 1);
  CATCH_CHECK(sstring_isalpha('1') == 0);
  CATCH_CHECK(sstring_isalpha('%') == 0);
  CATCH_CHECK(sstring_isalpha('e') == 1);

  CATCH_CHECK(sstring_isprint('a') == 1);
  CATCH_CHECK(sstring_isprint('#') == 1);
  CATCH_CHECK(sstring_isprint('~') == 1);
  CATCH_CHECK(sstring_isprint(0) == 0);
  CATCH_CHECK(sstring_isprint('%') == 1);
  CATCH_CHECK(sstring_isprint(240) == 0);

  CATCH_CHECK(sstring_isdigit('a') == 0);
  CATCH_CHECK(sstring_isdigit('x') == 0);
  CATCH_CHECK(sstring_isdigit('Z') == 0);
  CATCH_CHECK(sstring_isdigit('1') == 1);
  CATCH_CHECK(sstring_isdigit('%') == 0);
  CATCH_CHECK(sstring_isdigit('e') == 0);
} /* char_test() */

static void convert_test(void) {
  printf("\n***** Test atoi() *****\n");
  printf("Decimal string 1234    -> decimal number : %d\n", sstdio_atoi("1234", 10));
  printf("Decimal string 0x1234  -> decimal number : %#X\n", sstdio_atoi("0x1234", 16));
  printf("Decimal string -1234   -> decimal number : %d\n", sstdio_atoi("-1234", 10));
  printf("Decimal string 0       -> decimal number : %d\n", sstdio_atoi("0", 10));
  printf("Decimal string 1       -> decimal number : %d\n", sstdio_atoi("1", 10));
  printf("Decimal string 0       -> hex number     : %d\n", sstdio_atoi("0", 10));
  printf("Decimal string 1       -> hex number     : %d\n", sstdio_atoi("1", 10));
  printf("Decimal string -0x1234 -> hex number     : %#x\n", sstdio_atoi("-0x1234", 16));

  printf("Hex string 0x100  -> decimal number : %d\n", sstdio_atoi("0x100", 16));
  printf("Hex string 0x100  -> hex number     : %#X\n", sstdio_atoi("0x100", 16));
  printf("Hex string -0x100 -> a hex number   : %#x\n", sstdio_atoi("-0x100", 16));
  printf("Hex string -0x100 -> decimal number : %d\n", sstdio_atoi("-0x100", 16));

  printf("\n***** Test itoa *****\n");
  char buf[20];
  printf("Decimal 0      -> hex string: %s\n", sstdio_itoax(0, buf));
  printf("Decimal 1      -> hex string: %s\n", sstdio_itoax(1, buf));
  printf("Decimal -1     -> hex string: %s\n", sstdio_itoax(-1, buf));
  printf("Decimal -100   -> hex string: %s\n", sstdio_itoax(-100, buf));
  printf("Decimal 100    -> hex string: %s\n", sstdio_itoax(100, buf));
  printf("Decimal 0xfe87 -> hex string: %s\n", sstdio_itoax(0xfe87, buf));
  printf("Decimal 0x234  -> hex string: %s\n", sstdio_itoax(0x234, buf));

  printf("Hex 0x0    -> hex string: %s\n", sstdio_itoax(0, buf));
  printf("Hex 0x1    -> hex string: %s\n", sstdio_itoax(1, buf));
  printf("Hex 0x100  -> hex string: %s\n", sstdio_itoax(0x100, buf));
  printf("Hex 0xfe87 -> hex string: %s\n", sstdio_itoax(0xfe87, buf));
  printf("Hex 0x234  -> hex string: %s\n", sstdio_itoax(0x234, buf));

  printf("Decimal 0      -> decimal string: %s\n", sstdio_itoad(0, buf));
  printf("Decimal 1      -> decimal string: %s\n", sstdio_itoad(1, buf));
  printf("Decimal -1     -> decimal string: %s\n", sstdio_itoad(-1, buf));
  printf("Decimal 10     -> decimal string: %s\n", sstdio_itoad(10, buf));
  printf("Decimal -123   -> decimal string: %s\n", sstdio_itoad(-123, buf));
  printf("Decimal 102393 -> decimal string: %s\n", sstdio_itoad(102393, buf));
} /* convert_test() */

static void puts_test(void) {
  printf("\n***** Test puts() *****\n");
  const char* s = "I'm having a...a...you know--a headache with pictures!\r\nYou mean an idea?\r\n";
  sstdio_puts(s);
} /* string_test */
