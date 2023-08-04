///////////////////////////////////////////////////////////////////////////////
// @author (c) Eyal Rozenberg <eyalroz1@gmx.com>
//             2021-2022, Haifa, Palestine/Israel
// \author (c) Marco Paland (info@paland.com)
//             2017-2019, PALANDesign Hannover, Germany
//
// \license The MIT License (MIT)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// \brief printf unit tests
//
///////////////////////////////////////////////////////////////////////////////
#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_PREFIX_ALL
#include <catch.hpp>

#define RCSW_CONFIG_STDIO_PUTCHAR myputchar
#include "rcsw/stdio/printf_internal.h"
#include "rcsw/stdio/printf_config.h"
#include "rcsw/stdio/printf.h"
#include "rcsw/stdio/string.h"

#include <cstring>
#include <sstream>
#include <cmath>
#include <limits>
#include <climits>
#include <iostream>
#include <iomanip>


#if defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__)) || defined(__MINGW32__)
#include <sys/types.h>
#elif defined(_WIN32)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#else
// Let's just cross our fingers and hope `ssize_t` is defined.
#endif

#define CAPTURE_AND_PRINT(printer_, ...)                  \
do {                                                      \
  CATCH_INFO( #printer_  <<                                     \
     " arguments (ignore the equations; interpret \"expr" \
     "\" := expr\" as just \"expr\"): ");                 \
  printer_(__VA_ARGS__);                                  \
} while(0)

#define CAPTURE_AND_PRINT_WITH_RETVAL(retval, printer_, ...) \
do {                                                      \
  CATCH_INFO( #printer_  <<                                     \
     " arguments (ignore the equations; interpret \"expr" \
     "\" := expr\" as just \"expr\"): ");                 \
  retval = printer_(__VA_ARGS__);                         \
} while(0)


#define PRINTING_CHECK_WITH_BUF_SIZE(expected_, dummy, printer_, buffer_, buffer_size, ...) \
do {                                                             \
  CATCH_INFO( #printer_ << " arguments, replicated ( \"arg := arg\" "  \
  "):\n----"); \
  std::memset(buffer_, 0xCC, base_buffer_size);                  \
  printer_(buffer_, buffer_size, __VA_ARGS__);                   \
  if (!strcmp(buffer_, expected_)) {                             \
    buffer_[strlen(expected_) + 1] = '\0';                       \
  }                                                              \
  CATCH_INFO( "----");                                                 \
  CATCH_INFO( "Resulting buffer contents: " << '"' << buffer_ << '"'); \
  CATCH_CHECK(!strcmp(buffer_, expected_));                            \
} while(0)

#define PRINTING_CHECK(expected_, dummy, printer_, buffer_, ...) \
do {                                                             \
  CATCH_INFO( #printer_ << " arguments, replicated ( \"arg := arg\" "  \
  "):\n----"); \
  std::memset(buffer_, 0xCC, base_buffer_size);                  \
  printer_(buffer_, __VA_ARGS__);                                \
  if (!strcmp(buffer_, expected_)) {                             \
    buffer_[strlen(expected_) + 1] = '\0';                       \
  }                                                              \
  CATCH_INFO( "----");                                                 \
  CATCH_INFO( "Resulting buffer contents: " << '"' << buffer_ << '"'); \
  CATCH_CHECK(!strcmp(buffer_, expected_));                            \
} while(0)

// Multi-compiler-compatible local warning suppression
#ifdef TEST_WITH_NON_STANDARD_FORMAT_STRINGS
DISABLE_WARNING_PUSH
DISABLE_WARNING_PRINTF_FORMAT
DISABLE_WARNING_PRINTF_FORMAT_EXTRA_ARGS
DISABLE_WARNING_PRINTF_FORMAT_INVALID_SPECIFIER
#endif

#if defined(_MSC_VER)
DISABLE_WARNING(4996) // Discouragement of use of std::sprintf()
DISABLE_WARNING(4310) // Casting to smaller type
DISABLE_WARNING(4127) // Constant conditional expression
#endif

constexpr const size_t base_buffer_size { 100 };

// This macro is idempotent here, but for other platforms may
// be defined differently
#define mkstr(_str) _str


BEGIN_C_DECLS
// dummy putchar
static char   printf_buffer[base_buffer_size];
static size_t printf_idx = 0U;
int th_putchar(int c) {
  printf_buffer[printf_idx++] = c;
  return c;
}
END_C_DECLS

void _out_usf(char character, void* arg)
{
  (void)arg;
  printf_buffer[printf_idx++] = character;
}

#ifndef STRINGIFY
#define STRINGIFY(_x) #_x
#endif
#define PRINTF_TEST_CASE(unstringified_name)  CATCH_TEST_CASE(STRINGIFY(unstringified_name), "[]")

PRINTF_TEST_CASE(printf) {
  printf_idx = 0U;
  memset(printf_buffer, 0xCC, base_buffer_size);
  CATCH_INFO("printf_ format string and arguments: ");
  /* CATCH_CAPTURE("% d", 4232); */
  CATCH_CHECK(stdio_printf("% d", 4232) == 5);
  CATCH_INFO("printf_ format string and arguments: ");
  /* CATCH_CAPTURE("% d", 4232); */
  CATCH_CHECK(printf_buffer[5] == (char)0xCC);
  printf_buffer[5] = '\0';
  CATCH_INFO("printf_ format string and arguments: ");
  /* CATCH_CAPTURE("% d", 4232); */

  CATCH_CHECK(!strcmp(printf_buffer, " 4232"));
}


PRINTF_TEST_CASE(fctprintf) {
  printf_idx = 0U;
  memset(printf_buffer, 0xCC, base_buffer_size);
  stdio_usfprintf(&_out_usf, nullptr, "This is a test of %X", 0x12EFU);
  CATCH_INFO("usfprintf format string and arguments: ");
  /* CATCH_CAPTURE("This is a test of %X", 0x12EFU); */
  CATCH_CHECK(!strncmp(printf_buffer, "This is a test of 12EF", 22U));
  CATCH_CHECK(printf_buffer[22] == (char)0xCC);
}

// output function type
typedef void (*out_usf_type_)(char character, void* arg);


static void vusfprintf_builder_1(out_usf_type_ f, char* buffer, ...)
{
  va_list args;
  va_start(args, buffer);
  CAPTURE_AND_PRINT(stdio_vusfprintf, f, nullptr, "This is a test of %X", args);
  va_end(args);
}

PRINTF_TEST_CASE(vusfprintf) {
  printf_idx = 0U;
  memset(printf_buffer, 0xCC, base_buffer_size);
  vusfprintf_builder_1(&_out_usf, nullptr, 0x12EFU);
  CATCH_CHECK(!strncmp(printf_buffer, "This is a test of 12EF", 22U));
  CATCH_CHECK(printf_buffer[22] == (char)0xCC);
}

PRINTF_TEST_CASE(snprintf_) {
  char buffer[base_buffer_size];
  PRINTING_CHECK_WITH_BUF_SIZE("-1000", ==, stdio_snprintf, buffer, base_buffer_size, "%d", -1000);
  PRINTING_CHECK_WITH_BUF_SIZE("-1",    ==, stdio_snprintf, buffer, 3U, "%d", -1000);
}

static void vprintf_builder_1(char* buffer, ...)
{
  va_list args;
  va_start(args, buffer);
  stdio_vprintf("%d", args);
  va_end(args);
}

static void vsprintf_builder_1(char* buffer, ...)
{
  va_list args;
  va_start(args, buffer);
  CAPTURE_AND_PRINT(stdio_vsprintf, buffer, "%d", args);
  va_end(args);
}

static void stdio_vsnprintfbuilder_1(char* buffer, ...)
{
  va_list args;
  va_start(args, buffer);
  CAPTURE_AND_PRINT(stdio_vsnprintf, buffer, 100U, "%d", args);
  va_end(args);
}

static void vsprintf_builder_3(char* buffer, ...)
{
  va_list args;
  va_start(args, buffer);
  CAPTURE_AND_PRINT(stdio_vsprintf, buffer, "%d %d %s", args);
  va_end(args);
}

static void stdio_vsnprintfbuilder_3(char* buffer, ...)
{
  va_list args;
  va_start(args, buffer);
  CAPTURE_AND_PRINT(stdio_vsnprintf, buffer, 100U, "%d %d %s", args);
  va_end(args);
}


PRINTF_TEST_CASE(vprintf) {
  char buffer[base_buffer_size];
  printf_idx = 0U;
  memset(printf_buffer, 0xCC, base_buffer_size);
  vprintf_builder_1(buffer, 2345);
  CATCH_CHECK(printf_buffer[4] == (char)0xCC);
  printf_buffer[4] = 0;
  CATCH_CHECK(!strcmp(printf_buffer, "2345"));
}


PRINTF_TEST_CASE(vsprintf) {
  char buffer[base_buffer_size];

  vsprintf_builder_1(buffer, -1);
  CATCH_CHECK(!strcmp(buffer, "-1"));

  vsprintf_builder_3(buffer, 3, -1000, "test");
  CATCH_CHECK(!strcmp(buffer, "3 -1000 test"));
}


PRINTF_TEST_CASE(stdio_vsnprintf) {
  char buffer[base_buffer_size];

  stdio_vsnprintfbuilder_1(buffer, -1);
  CATCH_CHECK(!strcmp(buffer, "-1"));

  stdio_vsnprintfbuilder_3(buffer, 3, -1000, "test");
  CATCH_CHECK(!strcmp(buffer, "3 -1000 test"));
}

#if RCSW_CONFIG_STDIO_PRINTF_WITH_WRITEBACK
PRINTF_TEST_CASE(writeback_specifier) {
  char buffer[base_buffer_size];

  struct {
    char char_;
    short short_;
    int int_;
    long long_;
    long long long_long_;
  } num_written;


  num_written.int_ = 1234;
  stdio_printf("%n", &num_written.int_);
  CATCH_CHECK(num_written.int_ == 0);
  num_written.int_ = 1234;
  stdio_printf("foo%nbar", &num_written.int_);
  CATCH_CHECK(num_written.int_ == 3);

  num_written.int_ = 1234;
  PRINTING_CHECK("", ==, stdio_sprintf, buffer, "%n", &num_written.int_);
  CATCH_CHECK(num_written.int_ == 0);
  num_written.int_ = 1234;
  PRINTING_CHECK("foobar", ==, stdio_sprintf, buffer, "foo%nbar", &num_written.int_);
  CATCH_CHECK(num_written.int_ == 3);
}
#endif

PRINTF_TEST_CASE(ret_value)
{
  char buffer[base_buffer_size];
  int ret;

  ret = stdio_snprintf(buffer, 6, "0%s", "1234");
  CATCH_CHECK(!strcmp(buffer, "01234"));
  CATCH_CHECK(ret == 5);

  std::memset(buffer, 0xCC, sizeof(buffer));
  ret = stdio_snprintf(buffer, 6, "0%s", "12345");
  CATCH_CHECK(!strcmp(buffer, "01234"));
  CATCH_CHECK(ret == 6);  // "5" is truncated

  std::memset(buffer, 0xCC, sizeof(buffer));
  ret = stdio_snprintf(buffer, 6, "0%s", "1234567");
  CATCH_CHECK(!strcmp(buffer, "01234"));
  CATCH_CHECK(ret == 8);  // "567" are truncated

  std::memset(buffer, 0xCC, sizeof(buffer));
  ret = stdio_snprintf(buffer, 6, "0%s", (const char *) NULL);
  CATCH_CHECK(!strcmp(buffer, "0(nul"));
  CATCH_CHECK(ret == 7);  // "l)" is truncated

  std::memset(buffer, 0xCC, sizeof(buffer));
  ret = stdio_snprintf(buffer, 10, "hello, world");
  CATCH_CHECK(ret == 12);

  std::memset(buffer, 0xCC, sizeof(buffer));
  ret = stdio_snprintf(buffer, 3, "%d", 10000);
  CATCH_CHECK(ret == 5);
  CATCH_CHECK(strlen(buffer) == 2U);
  CATCH_CHECK(buffer[0] == '1');
  CATCH_CHECK(buffer[1] == '0');
  CATCH_CHECK(buffer[2] == '\0');
}

#if RCSW_CONFIG_STDIO_PRINTF_WITH_DEC || RCSW_CONFIG_STDIO_PRINTF_WITH_EXP
PRINTF_TEST_CASE(brute_force_float)
{
  char buffer[base_buffer_size];
#if RCSW_CONFIG_STDIO_PRINTF_WITH_DEC
  // brute force float
  bool any_failed = false;
  std::stringstream sstr;
  sstr.precision(5);
  for (float i = -100000; i < 100000; i += (float) 1) {
    stdio_sprintf(buffer, "%.5f", (double) (i / 10000));
    sstr.str("");
    sstr << std::fixed << i / 10000;
    if (strcmp(buffer, sstr.str().c_str()) != 0) {
      std::cerr
      << ": sprintf_(\"%.5f\", " << std::setw(6) << i << ") = " << std::setw(10) << buffer << " , "
      << "expected " << std::setw(10) << sstr.str().c_str() << "\n";
      any_failed = true;
    }
  }
  CATCH_CHECK(not any_failed);

#if RCSW_CONFIG_STDIO_PRINTF_WITH_EXP
  // This is tested when _both_ decimal and exponential specifiers are supported.
  // brute force exp
  sstr.setf(std::ios::scientific, std::ios::floatfield);
  any_failed = false;
  long n = 0;
  for (float i = (float) -1e20; i < (float) 1e20; i += (float) 1e15, n++) {
    stdio_sprintf(buffer, "%.5f", (double) i);
    sstr.str("");
    sstr << i;
    if (strcmp(buffer, sstr.str().c_str()) != 0) {
      std::cerr
      << n << ": sprintf_(\"%.5f\", " << std::setw(18) << std::setprecision(30) << i << ") = " << std::setw(15)
      << buffer << " , "
      << "expected " << std::setw(12) << sstr.str().c_str() << "\n";
      any_failed = true;
    }
  }
  CATCH_CHECK(not any_failed);
#endif
#endif
}

#endif // PRINTF_WITH_DECIMAL_SPECIFIERS || PRINTF_WITH_EXPONENTIAL_SPECIFIERS



///////////////////////////////////////////////////////////////////////////////
// @author (c) Eyal Rozenberg <eyalroz1@gmx.com>
//             2021-2022, Haifa, Palestine/Israel
// \author (c) Marco Paland (info@paland.com)
//             2017-2019, PALANDesign Hannover, Germany
//
// \license The MIT License (MIT)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// \brief printf unit tests
//
///////////////////////////////////////////////////////////////////////////////


// This file is to be included into either test_suite.cpp or another
// variant of the test suite. It is _not_ self contained in any way


PRINTF_TEST_CASE(space_flag)
{
  char buffer[base_buffer_size];
  PRINTING_CHECK(" 42",                     ==, stdio_sprintf, buffer, "% d", 42);
  PRINTING_CHECK("-42",                     ==, stdio_sprintf, buffer, "% d", -42);
  PRINTING_CHECK("   42",                   ==, stdio_sprintf, buffer, "% 5d", 42);
  PRINTING_CHECK("  -42",                   ==, stdio_sprintf, buffer, "% 5d", -42);
  PRINTING_CHECK("             42",         ==, stdio_sprintf, buffer, "% 15d", 42);
  PRINTING_CHECK("            -42",         ==, stdio_sprintf, buffer, "% 15d", -42);
  PRINTING_CHECK("            -42",         ==, stdio_sprintf, buffer, "% 15d", -42);
#if RCSW_CONFIG_STDIO_PRINTF_WITH_DEC
  PRINTING_CHECK("        -42.987",         ==, stdio_sprintf, buffer, "% 15.3f", -42.987);
  PRINTING_CHECK("         42.987",         ==, stdio_sprintf, buffer, "% 15.3f", 42.987);
#endif
  PRINTING_CHECK(" 1024",                   ==, stdio_sprintf, buffer, "% d", 1024);
  PRINTING_CHECK("-1024",                   ==, stdio_sprintf, buffer, "% d", -1024);
  PRINTING_CHECK(" 1024",                   ==, stdio_sprintf, buffer, "% i", 1024);
  PRINTING_CHECK("-1024",                   ==, stdio_sprintf, buffer, "% i", -1024);
}

#ifdef TEST_WITH_NON_STANDARD_FORMAT_STRINGS

PRINTF_TEST_CASE(space_flag__non_standard_format)
{
  char buffer[base_buffer_size];
  PRINTING_CHECK("Hello testing",           ==, stdio_sprintf, buffer, "% s", mkstr("Hello testing"));
  PRINTING_CHECK("1024",                    ==, stdio_sprintf, buffer, "% u", 1024);
#ifdef PRINTF_WITH_MSVC_STYLE_INTEGER_SPECIFIERS
  PRINTING_CHECK("1024",                    ==, stdio_sprintf, buffer, "% I16u", (uint16_t) 1024);
  PRINTING_CHECK("1024",                    ==, stdio_sprintf, buffer, "% I32u", (uint32_t) 1024);
  PRINTING_CHECK("1024",                    ==, stdio_sprintf, buffer, "% I64u", (uint64_t) 1024);
#endif
  PRINTING_CHECK("4294966272",              ==, stdio_sprintf, buffer, "% u", 4294966272U);
#ifdef PRINTF_WITH_MSVC_STYLE_INTEGER_SPECIFIERS
  PRINTING_CHECK("4294966272",              ==, stdio_sprintf, buffer, "% I32u", (uint32_t) 4294966272U);
  PRINTING_CHECK("4294966272",              ==, stdio_sprintf, buffer, "% I64u", (uint64_t) 4294966272U);
#endif
  PRINTING_CHECK("777",                     ==, stdio_sprintf, buffer, "% o", 511);
  PRINTING_CHECK("37777777001",             ==, stdio_sprintf, buffer, "% o", 4294966785U);
  PRINTING_CHECK("1234abcd",                ==, stdio_sprintf, buffer, "% x", 305441741);
  PRINTING_CHECK("edcb5433",                ==, stdio_sprintf, buffer, "% x", 3989525555U);
  PRINTING_CHECK("1234ABCD",                ==, stdio_sprintf, buffer, "% X", 305441741);
  PRINTING_CHECK("EDCB5433",                ==, stdio_sprintf, buffer, "% X", 3989525555U);
  PRINTING_CHECK("x",                       ==, stdio_sprintf, buffer, "% c", 'x');
}

#endif


PRINTF_TEST_CASE(plus_flag)
{
  char buffer[base_buffer_size];
  PRINTING_CHECK("+42",                     ==, stdio_sprintf, buffer, "%+d", 42);
  PRINTING_CHECK("-42",                     ==, stdio_sprintf, buffer, "%+d", -42);
  PRINTING_CHECK("  +42",                   ==, stdio_sprintf, buffer, "%+5d", 42);
  PRINTING_CHECK("  -42",                   ==, stdio_sprintf, buffer, "%+5d", -42);
  PRINTING_CHECK("            +42",         ==, stdio_sprintf, buffer, "%+15d", 42);
  PRINTING_CHECK("            -42",         ==, stdio_sprintf, buffer, "%+15d", -42);
  PRINTING_CHECK("+1024",                   ==, stdio_sprintf, buffer, "%+d", 1024);
  PRINTING_CHECK("-1024",                   ==, stdio_sprintf, buffer, "%+d", -1024);
  PRINTING_CHECK("+1024",                   ==, stdio_sprintf, buffer, "%+i", 1024);
  PRINTING_CHECK("-1024",                   ==, stdio_sprintf, buffer, "%+i", -1024);
#ifdef PRINTF_WITH_MSVC_STYLE_INTEGER_SPECIFIERS
  PRINTING_CHECK("+1024",                   ==, stdio_sprintf, buffer, "%+I16d", (int16_t) 1024);
  PRINTING_CHECK("-1024",                   ==, stdio_sprintf, buffer, "%+I16d", (int16_t) -1024);
  PRINTING_CHECK("+1024",                   ==, stdio_sprintf, buffer, "%+I32d", (int32_t) 1024);
  PRINTING_CHECK("-1024",                   ==, stdio_sprintf, buffer, "%+I32d", (int32_t) -1024);
  PRINTING_CHECK("+1024",                   ==, stdio_sprintf, buffer, "%+I64d", (int64_t) 1024);
  PRINTING_CHECK("-1024",                   ==, stdio_sprintf, buffer, "%+I64d", (int64_t) -1024);
#endif
  PRINTING_CHECK("+",                       ==, stdio_sprintf, buffer, "%+.0d", 0);
}

#ifdef TEST_WITH_NON_STANDARD_FORMAT_STRINGS

PRINTF_TEST_CASE(plus_flag__non_standard_format)
{
  char buffer[base_buffer_size];
  PRINTING_CHECK("Hello testing",           ==, stdio_sprintf, buffer, "%+s", mkstr("Hello testing"));
  PRINTING_CHECK("1024",                    ==, stdio_sprintf, buffer, "%+u", 1024);
#ifdef PRINTF_WITH_MSVC_STYLE_INTEGER_SPECIFIERS
  PRINTING_CHECK("1024",                    ==, stdio_sprintf, buffer, "%+I32u", (uint32_t) 1024);
#endif
  PRINTING_CHECK("4294966272",              ==, stdio_sprintf, buffer, "%+u", 4294966272U);
#ifdef PRINTF_WITH_MSVC_STYLE_INTEGER_SPECIFIERS
  PRINTING_CHECK("4294966272",              ==, stdio_sprintf, buffer, "%+I32u", (uint32_t) 4294966272U);
#endif
  PRINTING_CHECK("777",                     ==, stdio_sprintf, buffer, "%+o", 511);
  PRINTING_CHECK("37777777001",             ==, stdio_sprintf, buffer, "%+o", 4294966785U);
  PRINTING_CHECK("1234abcd",                ==, stdio_sprintf, buffer, "%+x", 305441741);
  PRINTING_CHECK("edcb5433",                ==, stdio_sprintf, buffer, "%+x", 3989525555U);
  PRINTING_CHECK("1234ABCD",                ==, stdio_sprintf, buffer, "%+X", 305441741);
  PRINTING_CHECK("EDCB5433",                ==, stdio_sprintf, buffer, "%+X", 3989525555U);
  PRINTING_CHECK("x",                       ==, stdio_sprintf, buffer, "%+c", 'x');
}

#endif


PRINTF_TEST_CASE(zero_flag)
{
  char buffer[base_buffer_size];
  PRINTING_CHECK("42",                      ==, stdio_sprintf, buffer, "%0d", 42);
  PRINTING_CHECK("42",                      ==, stdio_sprintf, buffer, "%0ld", 42L);
  PRINTING_CHECK("-42",                     ==, stdio_sprintf, buffer, "%0d", -42);
  PRINTING_CHECK("00042",                   ==, stdio_sprintf, buffer, "%05d", 42);
  PRINTING_CHECK("-0042",                   ==, stdio_sprintf, buffer, "%05d", -42);
  PRINTING_CHECK("000000000000042",         ==, stdio_sprintf, buffer, "%015d", 42);
  PRINTING_CHECK("-00000000000042",         ==, stdio_sprintf, buffer, "%015d", -42);
#if RCSW_CONFIG_STDIO_PRINTF_WITH_DEC
  PRINTING_CHECK("000000000042.12",         ==, stdio_sprintf, buffer, "%015.2f", 42.1234);
  PRINTING_CHECK("00000000042.988",         ==, stdio_sprintf, buffer, "%015.3f", 42.9876);
  PRINTING_CHECK("-00000042.98760",         ==, stdio_sprintf, buffer, "%015.5f", -42.9876);
#endif
}


PRINTF_TEST_CASE(minus_flag)
{
  char buffer[base_buffer_size];
  PRINTING_CHECK("42",                      ==, stdio_sprintf, buffer, "%-d", 42);
  PRINTING_CHECK("-42",                     ==, stdio_sprintf, buffer, "%-d", -42);
  PRINTING_CHECK("42   ",                   ==, stdio_sprintf, buffer, "%-5d", 42);
  PRINTING_CHECK("-42  ",                   ==, stdio_sprintf, buffer, "%-5d", -42);
  PRINTING_CHECK("42             ",         ==, stdio_sprintf, buffer, "%-15d", 42);
  PRINTING_CHECK("-42            ",         ==, stdio_sprintf, buffer, "%-15d", -42);
}

#ifdef TEST_WITH_NON_STANDARD_FORMAT_STRINGS

PRINTF_TEST_CASE(minus_flag_and_non_standard_zero_modifier_for_integers)
{
  char buffer[base_buffer_size];
  PRINTING_CHECK("42",                      ==, stdio_sprintf, buffer, "%-0d", 42);
  PRINTING_CHECK("-42",                     ==, stdio_sprintf, buffer, "%-0d", -42);
  PRINTING_CHECK("42   ",                   ==, stdio_sprintf, buffer, "%-05d", 42);
  PRINTING_CHECK("-42  ",                   ==, stdio_sprintf, buffer, "%-05d", -42);
  PRINTING_CHECK("42             ",         ==, stdio_sprintf, buffer, "%-015d", 42);
  PRINTING_CHECK("-42            ",         ==, stdio_sprintf, buffer, "%-015d", -42);
  PRINTING_CHECK("42",                      ==, stdio_sprintf, buffer, "%0-d", 42);
  PRINTING_CHECK("-42",                     ==, stdio_sprintf, buffer, "%0-d", -42);
  PRINTING_CHECK("42   ",                   ==, stdio_sprintf, buffer, "%0-5d", 42);
  PRINTING_CHECK("-42  ",                   ==, stdio_sprintf, buffer, "%0-5d", -42);
  PRINTING_CHECK("42             ",         ==, stdio_sprintf, buffer, "%0-15d", 42);
  PRINTING_CHECK("-42            ",         ==, stdio_sprintf, buffer, "%0-15d", -42);

#if PRINTF_WITH_EXPONENTIAL_SPECIFIERS
  PRINTING_CHECK("-4.200e+01     ",         ==, stdio_sprintf, buffer, "%0-15.3e", -42.);
#else
  PRINTING_CHECK("e",                       ==, stdio_sprintf, buffer, "%0-15.3e", -42.);
#endif

#if PRINTF_WITH_EXPONENTIAL_SPECIFIERS
  PRINTING_CHECK("-42            ",         ==, stdio_sprintf, buffer, "%0-15.3g", -42.);
#else
  PRINTING_CHECK("g",                       ==, stdio_sprintf, buffer,  "%0-15.3g", -42.);
#endif
}

#endif


PRINTF_TEST_CASE(sharp_flag)
{
  char buffer[base_buffer_size];
  PRINTING_CHECK("0",                       ==, stdio_sprintf, buffer, "%#o",   0);
  PRINTING_CHECK("0",                       ==, stdio_sprintf, buffer, "%#0o",  0);
  PRINTING_CHECK("0",                       ==, stdio_sprintf, buffer, "%#.0o", 0);
  PRINTING_CHECK("0",                       ==, stdio_sprintf, buffer, "%#.1o", 0);
  PRINTING_CHECK("   0",                    ==, stdio_sprintf, buffer, "%#4o",  0);
  PRINTING_CHECK("0000",                    ==, stdio_sprintf, buffer, "%#.4o", 0);
  PRINTING_CHECK("01",                      ==, stdio_sprintf, buffer, "%#o",   1);
  PRINTING_CHECK("01",                      ==, stdio_sprintf, buffer, "%#0o",  1);
  PRINTING_CHECK("01",                      ==, stdio_sprintf, buffer, "%#.0o", 1);
  PRINTING_CHECK("01",                      ==, stdio_sprintf, buffer, "%#.1o", 1);
  PRINTING_CHECK("  01",                    ==, stdio_sprintf, buffer, "%#4o",  1);
  PRINTING_CHECK("0001",                    ==, stdio_sprintf, buffer, "%#.4o", 1);
  PRINTING_CHECK("0x1001",                  ==, stdio_sprintf, buffer, "%#04x", 0x1001);
  PRINTING_CHECK("01001",                   ==, stdio_sprintf, buffer, "%#04o", 01001);
  PRINTING_CHECK("",                        ==, stdio_sprintf, buffer, "%#.0x", 0);
  PRINTING_CHECK("0x0000614e",              ==, stdio_sprintf, buffer, "%#.8x", 0x614e);
}

#ifdef TEST_WITH_NON_STANDARD_FORMAT_STRINGS

PRINTF_TEST_CASE(sharp_flag__non_standard_format)
{
  char buffer[base_buffer_size];
  PRINTING_CHECK("0b110",                   ==, stdio_sprintf, buffer, "%#b",    6);
  PRINTING_CHECK("0b11111111",              ==, stdio_sprintf, buffer, "%#010b", 0xff);
  PRINTING_CHECK("0b011111111",             ==, stdio_sprintf, buffer, "%#011b", 0xff);
  PRINTING_CHECK("077",                     ==, stdio_sprintf, buffer, "%#03o",  077);
  PRINTING_CHECK("0077",                    ==, stdio_sprintf, buffer, "%#04o",  077);
}

#endif

#if RCSW_CONFIG_STDIO_PRINTF_WITH_LL

PRINTF_TEST_CASE(sharp_flag_with_long_long)
{
  char buffer[base_buffer_size];
  PRINTING_CHECK("0",                       ==, stdio_sprintf, buffer, "%#llo",   (long long) 0);
  PRINTING_CHECK("0",                       ==, stdio_sprintf, buffer, "%#0llo",  (long long) 0);
  PRINTING_CHECK("0",                       ==, stdio_sprintf, buffer, "%#.0llo", (long long) 0);
  PRINTING_CHECK("0",                       ==, stdio_sprintf, buffer, "%#.1llo", (long long) 0);
  PRINTING_CHECK("   0",                    ==, stdio_sprintf, buffer, "%#4llo",  (long long) 0);
  PRINTING_CHECK("0000",                    ==, stdio_sprintf, buffer, "%#.4llo", (long long) 0);
  PRINTING_CHECK("01",                      ==, stdio_sprintf, buffer, "%#llo",   (long long) 1);
  PRINTING_CHECK("01",                      ==, stdio_sprintf, buffer, "%#0llo",  (long long) 1);
  PRINTING_CHECK("01",                      ==, stdio_sprintf, buffer, "%#.0llo", (long long) 1);
  PRINTING_CHECK("01",                      ==, stdio_sprintf, buffer, "%#.1llo", (long long) 1);
  PRINTING_CHECK("  01",                    ==, stdio_sprintf, buffer, "%#4llo",  (long long) 1);
  PRINTING_CHECK("0001",                    ==, stdio_sprintf, buffer, "%#.4llo", (long long) 1);
  PRINTING_CHECK("0x1001",                  ==, stdio_sprintf, buffer, "%#04llx", (long long) 0x1001);
  PRINTING_CHECK("01001",                   ==, stdio_sprintf, buffer, "%#04llo", (long long) 01001);
  PRINTING_CHECK("",                        ==, stdio_sprintf, buffer, "%#.0llx", (long long) 0);
  PRINTING_CHECK("0x0000614e",              ==, stdio_sprintf, buffer, "%#.8llx", (long long) 0x614e);
}


#ifdef TEST_WITH_NON_STANDARD_FORMAT_STRINGS

PRINTF_TEST_CASE(sharp_flag_with_long_long__non_standard_format)
{
  char buffer[base_buffer_size];
  PRINTING_CHECK("0b110",                   ==, stdio_sprintf, buffer, "%#llb", (long long) 6);
}

#endif
#endif // PRINTF_WITH_LL

PRINTF_TEST_CASE(specifier)
{
  char buffer[base_buffer_size];
  PRINTING_CHECK("Hello testing",           ==, stdio_sprintf, buffer, "Hello testing");
  PRINTING_CHECK("Hello testing",           ==, stdio_sprintf, buffer, "%s", mkstr("Hello testing"));

  PRINTING_CHECK("(null)",                  ==, stdio_sprintf, buffer, "%s", (const char *) nullptr);
  PRINTING_CHECK("1024",                    ==, stdio_sprintf, buffer, "%d", 1024);
#if INT_MAX >= 2147483647LL
  PRINTING_CHECK("2147483647",              ==, stdio_sprintf, buffer, "%d", 2147483647);
  PRINTING_CHECK("4294966272",              ==, stdio_sprintf, buffer, "%u", 4294966272U);
  PRINTING_CHECK("37777777001",             ==, stdio_sprintf, buffer, "%o", 4294966785U);
  PRINTING_CHECK("1234abcd",                ==, stdio_sprintf, buffer, "%x", 305441741);
  PRINTING_CHECK("edcb5433",                ==, stdio_sprintf, buffer, "%x", 3989525555U);
  PRINTING_CHECK("1234ABCD",                ==, stdio_sprintf, buffer, "%X", 305441741);
  PRINTING_CHECK("EDCB5433",                ==, stdio_sprintf, buffer, "%X", 3989525555U);
#endif
  PRINTING_CHECK("-1024",                   ==, stdio_sprintf, buffer, "%d", -1024);
  PRINTING_CHECK("1024",                    ==, stdio_sprintf, buffer, "%i", 1024);
  PRINTING_CHECK("-1024",                   ==, stdio_sprintf, buffer, "%i", -1024);
  PRINTING_CHECK("1024",                    ==, stdio_sprintf, buffer, "%u", 1024);
  PRINTING_CHECK("777",                     ==, stdio_sprintf, buffer, "%o", 511);
  PRINTING_CHECK("%",                       ==, stdio_sprintf, buffer, "%%");

#ifdef PRINTF_WITH_MSVC_STYLE_INTEGER_SPECIFIERS
  PRINTING_CHECK("127",                     ==, stdio_sprintf, buffer, "%I8d", (int8_t) 127LL);
#if (SHRT_MAX >= 32767)
  PRINTING_CHECK("32767",                   ==, stdio_sprintf, buffer, "%I16d", (int16_t) 32767LL);
#endif
#if (LLONG_MAX >= 2147483647)
  PRINTING_CHECK("2147483647",              ==, stdio_sprintf, buffer, "%I32d", (int32_t) 2147483647LL);
#if (LLONG_MAX >= 9223372036854775807LL)
  PRINTING_CHECK("9223372036854775807",     ==, stdio_sprintf, buffer, "%I64d", (int64_t) 9223372036854775807LL);
#endif
#endif
#endif // PRINTF_WITH_MSVC_STYLE_INTEGER_SPECIFIERS
}


PRINTF_TEST_CASE(width)
{
  char buffer[base_buffer_size];
  PRINTING_CHECK("Hello testing",           ==, stdio_sprintf, buffer, "%1s", mkstr("Hello testing"));
  PRINTING_CHECK("1024",                    ==, stdio_sprintf, buffer, "%1d", 1024);
  PRINTING_CHECK("-1024",                   ==, stdio_sprintf, buffer, "%1d", -1024);
  PRINTING_CHECK("1024",                    ==, stdio_sprintf, buffer, "%1i", 1024);
  PRINTING_CHECK("-1024",                   ==, stdio_sprintf, buffer, "%1i", -1024);
  PRINTING_CHECK("1024",                    ==, stdio_sprintf, buffer, "%1u", 1024);
#ifdef PRINTF_WITH_MSVC_STYLE_INTEGER_SPECIFIERS
  PRINTING_CHECK("1024",                    ==, stdio_sprintf, buffer, "%1I16u", (uint16_t) 1024);
  PRINTING_CHECK("1024",                    ==, stdio_sprintf, buffer, "%1I32u", (uint32_t) 1024);
  PRINTING_CHECK("1024",                    ==, stdio_sprintf, buffer, "%1I64u", (uint64_t) 1024);
#endif
  PRINTING_CHECK("4294966272",              ==, stdio_sprintf, buffer, "%1u", 4294966272U);
#ifdef PRINTF_WITH_MSVC_STYLE_INTEGER_SPECIFIERS
  PRINTING_CHECK("4294966272",              ==, stdio_sprintf, buffer, "%1I32u", (uint32_t) 4294966272U);
  PRINTING_CHECK("4294966272",              ==, stdio_sprintf, buffer, "%1I64u", (uint64_t) 4294966272U);
#endif
  PRINTING_CHECK("777",                     ==, stdio_sprintf, buffer, "%1o", 511);
  PRINTING_CHECK("37777777001",             ==, stdio_sprintf, buffer, "%1o", 4294966785U);
  PRINTING_CHECK("1234abcd",                ==, stdio_sprintf, buffer, "%1x", 305441741);
  PRINTING_CHECK("edcb5433",                ==, stdio_sprintf, buffer, "%1x", 3989525555U);
  PRINTING_CHECK("1234ABCD",                ==, stdio_sprintf, buffer, "%1X", 305441741);
  PRINTING_CHECK("EDCB5433",                ==, stdio_sprintf, buffer, "%1X", 3989525555U);
  PRINTING_CHECK("x",                       ==, stdio_sprintf, buffer, "%1c", 'x');
}


PRINTF_TEST_CASE(width_20)
{
  char buffer[base_buffer_size];
  PRINTING_CHECK("               Hello",    ==, stdio_sprintf, buffer, "%20s", mkstr("Hello"));
  PRINTING_CHECK("                1024",    ==, stdio_sprintf, buffer, "%20d", 1024);
  PRINTING_CHECK("               -1024",    ==, stdio_sprintf, buffer, "%20d", -1024);
  PRINTING_CHECK("                1024",    ==, stdio_sprintf, buffer, "%20i", 1024);
  PRINTING_CHECK("               -1024",    ==, stdio_sprintf, buffer, "%20i", -1024);
  PRINTING_CHECK("                   0",    ==, stdio_sprintf, buffer, "%20i", 0);
  PRINTING_CHECK("                1024",    ==, stdio_sprintf, buffer, "%20u", 1024);
#ifdef PRINTF_WITH_MSVC_STYLE_INTEGER_SPECIFIERS
  PRINTING_CHECK("                1024",    ==, stdio_sprintf, buffer, "%20I16u", (uint16_t) 1024);
  PRINTING_CHECK("                1024",    ==, stdio_sprintf, buffer, "%20I32u", (uint32_t) 1024);
  PRINTING_CHECK("                1024",    ==, stdio_sprintf, buffer, "%20I64u", (uint64_t) 1024);
#endif
  PRINTING_CHECK("          4294966272",    ==, stdio_sprintf, buffer, "%20u", 4294966272U);
#ifdef PRINTF_WITH_MSVC_STYLE_INTEGER_SPECIFIERS
  PRINTING_CHECK("          4294966272",    ==, stdio_sprintf, buffer, "%20I32u", (uint32_t) 4294966272U);
  PRINTING_CHECK("          4294966272",    ==, stdio_sprintf, buffer, "%20I64u", (uint64_t) 4294966272U);
#endif
  PRINTING_CHECK("                 777",    ==, stdio_sprintf, buffer, "%20o", 511);
  PRINTING_CHECK("         37777777001",    ==, stdio_sprintf, buffer, "%20o", 4294966785U);
  PRINTING_CHECK("            1234abcd",    ==, stdio_sprintf, buffer, "%20x", 305441741);
  PRINTING_CHECK("            edcb5433",    ==, stdio_sprintf, buffer, "%20x", 3989525555U);
  PRINTING_CHECK("            1234ABCD",    ==, stdio_sprintf, buffer, "%20X", 305441741);
  PRINTING_CHECK("            EDCB5433",    ==, stdio_sprintf, buffer, "%20X", 3989525555U);
  PRINTING_CHECK("                   0",    ==, stdio_sprintf, buffer, "%20X", 0);
  PRINTING_CHECK("                   0",    ==, stdio_sprintf, buffer, "%20X", 0U);
#if RCSW_CONFIG_STDIO_PRINTF_WITH_LL
  PRINTING_CHECK("                   0",    ==, stdio_sprintf, buffer, "%20llX", 0ULL);
#endif
  PRINTING_CHECK("                   x",    ==, stdio_sprintf, buffer, "%20c", 'x');
}


PRINTF_TEST_CASE(width_asterisk_20)
{
  char buffer[base_buffer_size];
  PRINTING_CHECK("               Hello",    ==, stdio_sprintf, buffer, "%*s", 20, mkstr("Hello"));
  PRINTING_CHECK("                1024",    ==, stdio_sprintf, buffer, "%*d", 20, 1024);
  PRINTING_CHECK("               -1024",    ==, stdio_sprintf, buffer, "%*d", 20, -1024);
  PRINTING_CHECK("                1024",    ==, stdio_sprintf, buffer, "%*i", 20, 1024);
  PRINTING_CHECK("               -1024",    ==, stdio_sprintf, buffer, "%*i", 20, -1024);
  PRINTING_CHECK("                1024",    ==, stdio_sprintf, buffer, "%*u", 20, 1024);
#ifdef PRINTF_WITH_MSVC_STYLE_INTEGER_SPECIFIERS
  PRINTING_CHECK("                1024",    ==, stdio_sprintf, buffer, "%*I16u", 20, (uint16_t) 1024);
  PRINTING_CHECK("                1024",    ==, stdio_sprintf, buffer, "%*I32u", 20, (uint32_t) 1024);
  PRINTING_CHECK("                1024",    ==, stdio_sprintf, buffer, "%*I64u", 20, (uint64_t) 1024);
#endif
  PRINTING_CHECK("          4294966272",    ==, stdio_sprintf, buffer, "%*u", 20, 4294966272U);
#ifdef PRINTF_WITH_MSVC_STYLE_INTEGER_SPECIFIERS
  PRINTING_CHECK("          4294966272",    ==, stdio_sprintf, buffer, "%*I32u", 20, (uint32_t) 4294966272U);
  PRINTING_CHECK("          4294966272",    ==, stdio_sprintf, buffer, "%*I64u", 20, (uint64_t) 4294966272U);
#endif
  PRINTING_CHECK("                 777",    ==, stdio_sprintf, buffer, "%*o", 20, 511);
  PRINTING_CHECK("         37777777001",    ==, stdio_sprintf, buffer, "%*o", 20, 4294966785U);
  PRINTING_CHECK("            1234abcd",    ==, stdio_sprintf, buffer, "%*x", 20, 305441741);
  PRINTING_CHECK("            edcb5433",    ==, stdio_sprintf, buffer, "%*x", 20, 3989525555U);
  PRINTING_CHECK("            1234ABCD",    ==, stdio_sprintf, buffer, "%*X", 20, 305441741);
  PRINTING_CHECK("            EDCB5433",    ==, stdio_sprintf, buffer, "%*X", 20, 3989525555U);
  PRINTING_CHECK("                   x",    ==, stdio_sprintf, buffer, "%*c", 20, 'x');
}


PRINTF_TEST_CASE(width_minus_20)
{
  char buffer[base_buffer_size];
  PRINTING_CHECK("Hello               ",    ==, stdio_sprintf, buffer, "%-20s", mkstr("Hello"));
  PRINTING_CHECK("1024                ",    ==, stdio_sprintf, buffer, "%-20d", 1024);
  PRINTING_CHECK("-1024               ",    ==, stdio_sprintf, buffer, "%-20d", -1024);
  PRINTING_CHECK("1024                ",    ==, stdio_sprintf, buffer, "%-20i", 1024);
  PRINTING_CHECK("-1024               ",    ==, stdio_sprintf, buffer, "%-20i", -1024);
  PRINTING_CHECK("1024                ",    ==, stdio_sprintf, buffer, "%-20u", 1024);
#if RCSW_CONFIG_STDIO_PRINTF_WITH_DEC
  PRINTING_CHECK("1024.1234           ",    ==, stdio_sprintf, buffer, "%-20.4f", 1024.1234);
#endif
  PRINTING_CHECK("4294966272          ",    ==, stdio_sprintf, buffer, "%-20u", 4294966272U);
  PRINTING_CHECK("777                 ",    ==, stdio_sprintf, buffer, "%-20o", 511);
  PRINTING_CHECK("37777777001         ",    ==, stdio_sprintf, buffer, "%-20o", 4294966785U);
  PRINTING_CHECK("1234abcd            ",    ==, stdio_sprintf, buffer, "%-20x", 305441741);
  PRINTING_CHECK("edcb5433            ",    ==, stdio_sprintf, buffer, "%-20x", 3989525555U);
  PRINTING_CHECK("1234ABCD            ",    ==, stdio_sprintf, buffer, "%-20X", 305441741);
  PRINTING_CHECK("EDCB5433            ",    ==, stdio_sprintf, buffer, "%-20X", 3989525555U);
  PRINTING_CHECK("x                   ",    ==, stdio_sprintf, buffer, "%-20c", 'x');
  PRINTING_CHECK("|    9| |9 | |    9|",    ==, stdio_sprintf, buffer, "|%5d| |%-2d| |%5d|", 9, 9, 9);
  PRINTING_CHECK("|   10| |10| |   10|",    ==, stdio_sprintf, buffer, "|%5d| |%-2d| |%5d|", 10, 10, 10);
  PRINTING_CHECK("|    9| |9           | |    9|", ==, stdio_sprintf, buffer, "|%5d| |%-12d| |%5d|", 9, 9, 9);
  PRINTING_CHECK("|   10| |10          | |   10|", ==, stdio_sprintf, buffer, "|%5d| |%-12d| |%5d|", 10, 10, 10);
}

#ifdef TEST_WITH_NON_STANDARD_FORMAT_STRINGS

PRINTF_TEST_CASE(width_0_minus_20)
{
  char buffer[base_buffer_size];
  PRINTING_CHECK("Hello               ",    ==, stdio_sprintf, buffer, "%0-20s", mkstr("Hello"));
  PRINTING_CHECK("1024                ",    ==, stdio_sprintf, buffer, "%0-20d", 1024);
  PRINTING_CHECK("-1024               ",    ==, stdio_sprintf, buffer, "%0-20d", -1024);
  PRINTING_CHECK("1024                ",    ==, stdio_sprintf, buffer, "%0-20i", 1024);
  PRINTING_CHECK("-1024               ",    ==, stdio_sprintf, buffer, "%0-20i", -1024);
  PRINTING_CHECK("1024                ",    ==, stdio_sprintf, buffer, "%0-20u", 1024);
  PRINTING_CHECK("4294966272          ",    ==, stdio_sprintf, buffer, "%0-20u", 4294966272U);
  PRINTING_CHECK("777                 ",    ==, stdio_sprintf, buffer, "%0-20o", 511);
  PRINTING_CHECK("37777777001         ",    ==, stdio_sprintf, buffer, "%0-20o", 4294966785U);
  PRINTING_CHECK("1234abcd            ",    ==, stdio_sprintf, buffer, "%0-20x", 305441741);
  PRINTING_CHECK("edcb5433            ",    ==, stdio_sprintf, buffer, "%0-20x", 3989525555U);
  PRINTING_CHECK("1234ABCD            ",    ==, stdio_sprintf, buffer, "%0-20X", 305441741);
  PRINTING_CHECK("EDCB5433            ",    ==, stdio_sprintf, buffer, "%0-20X", 3989525555U);
  PRINTING_CHECK("x                   ",    ==, stdio_sprintf, buffer, "%0-20c", 'x');
}

#endif

PRINTF_TEST_CASE(padding_20)
{
  char buffer[base_buffer_size];
  PRINTING_CHECK("00000000000000001024",    ==, stdio_sprintf, buffer, "%020d", 1024);
  PRINTING_CHECK("-0000000000000001024",    ==, stdio_sprintf, buffer, "%020d", -1024);
  PRINTING_CHECK("00000000000000001024",    ==, stdio_sprintf, buffer, "%020i", 1024);
  PRINTING_CHECK("-0000000000000001024",    ==, stdio_sprintf, buffer, "%020i", -1024);
  PRINTING_CHECK("00000000000000001024",    ==, stdio_sprintf, buffer, "%020u", 1024);
  PRINTING_CHECK("00000000004294966272",    ==, stdio_sprintf, buffer, "%020u", 4294966272U);
  PRINTING_CHECK("00000000000000000777",    ==, stdio_sprintf, buffer, "%020o", 511);
  PRINTING_CHECK("00000000037777777001",    ==, stdio_sprintf, buffer, "%020o", 4294966785U);
  PRINTING_CHECK("0000000000001234abcd",    ==, stdio_sprintf, buffer, "%020x", 305441741);
  PRINTING_CHECK("000000000000edcb5433",    ==, stdio_sprintf, buffer, "%020x", 3989525555U);
  PRINTING_CHECK("0000000000001234ABCD",    ==, stdio_sprintf, buffer, "%020X", 305441741);
  PRINTING_CHECK("000000000000EDCB5433",    ==, stdio_sprintf, buffer, "%020X", 3989525555U);
}


PRINTF_TEST_CASE(padding_dot_20)
{
  char buffer[base_buffer_size];
  PRINTING_CHECK("00000000000000001024",    ==, stdio_sprintf, buffer, "%.20d", 1024);
  PRINTING_CHECK("-00000000000000001024",   ==, stdio_sprintf, buffer, "%.20d", -1024);
  PRINTING_CHECK("00000000000000001024",    ==, stdio_sprintf, buffer, "%.20i", 1024);
  PRINTING_CHECK("-00000000000000001024",   ==, stdio_sprintf, buffer, "%.20i", -1024);
  PRINTING_CHECK("00000000000000001024",    ==, stdio_sprintf, buffer, "%.20u", 1024);
  PRINTING_CHECK("00000000004294966272",    ==, stdio_sprintf, buffer, "%.20u", 4294966272U);
  PRINTING_CHECK("00000000000000000777",    ==, stdio_sprintf, buffer, "%.20o", 511);
  PRINTING_CHECK("00000000037777777001",    ==, stdio_sprintf, buffer, "%.20o", 4294966785U);
  PRINTING_CHECK("0000000000001234abcd",    ==, stdio_sprintf, buffer, "%.20x", 305441741);
  PRINTING_CHECK("000000000000edcb5433",    ==, stdio_sprintf, buffer, "%.20x", 3989525555U);
  PRINTING_CHECK("0000000000001234ABCD",    ==, stdio_sprintf, buffer, "%.20X", 305441741);
  PRINTING_CHECK("000000000000EDCB5433",    ==, stdio_sprintf, buffer, "%.20X", 3989525555U);
}

#ifdef TEST_WITH_NON_STANDARD_FORMAT_STRINGS

PRINTF_TEST_CASE(padding_sharp_020__non_standard_format)
{
  char buffer[base_buffer_size];
  PRINTING_CHECK("00000000000000001024",    ==, stdio_sprintf, buffer, "%#020d", 1024);
  PRINTING_CHECK("-0000000000000001024",    ==, stdio_sprintf, buffer, "%#020d", -1024);
  PRINTING_CHECK("00000000000000001024",    ==, stdio_sprintf, buffer, "%#020i", 1024);
  PRINTING_CHECK("-0000000000000001024",    ==, stdio_sprintf, buffer, "%#020i", -1024);
  PRINTING_CHECK("00000000000000001024",    ==, stdio_sprintf, buffer, "%#020u", 1024);
  PRINTING_CHECK("00000000004294966272",    ==, stdio_sprintf, buffer, "%#020u", 4294966272U);
}

#endif

PRINTF_TEST_CASE(padding_sharp_020)
{
  char buffer[base_buffer_size];
  PRINTING_CHECK("00000000000000000777",    ==, stdio_sprintf, buffer, "%#020o", 511);
  PRINTING_CHECK("00000000037777777001",    ==, stdio_sprintf, buffer, "%#020o", 4294966785U);
  PRINTING_CHECK("0x00000000001234abcd",    ==, stdio_sprintf, buffer, "%#020x", 305441741);
  PRINTING_CHECK("0x0000000000edcb5433",    ==, stdio_sprintf, buffer, "%#020x", 3989525555U);
  PRINTING_CHECK("0X00000000001234ABCD",    ==, stdio_sprintf, buffer, "%#020X", 305441741);
  PRINTING_CHECK("0X0000000000EDCB5433",    ==, stdio_sprintf, buffer, "%#020X", 3989525555U);
}


#ifdef TEST_WITH_NON_STANDARD_FORMAT_STRINGS

PRINTF_TEST_CASE(padding_sharp_20__non_standard_format)
{
  char buffer[base_buffer_size];
  PRINTING_CHECK("                1024",    ==, stdio_sprintf, buffer, "%#20d", 1024);
  PRINTING_CHECK("               -1024",    ==, stdio_sprintf, buffer, "%#20d", -1024);
  PRINTING_CHECK("                1024",    ==, stdio_sprintf, buffer, "%#20i", 1024);
  PRINTING_CHECK("               -1024",    ==, stdio_sprintf, buffer, "%#20i", -1024);
  PRINTING_CHECK("                1024",    ==, stdio_sprintf, buffer, "%#20u", 1024);
  PRINTING_CHECK("          4294966272",    ==, stdio_sprintf, buffer, "%#20u", 4294966272U);
}

#endif

PRINTF_TEST_CASE(padding_sharp_20)
{
  char buffer[base_buffer_size];
  PRINTING_CHECK("                0777",    ==, stdio_sprintf, buffer, "%#20o", 511);
  PRINTING_CHECK("        037777777001",    ==, stdio_sprintf, buffer, "%#20o", 4294966785U);
  PRINTING_CHECK("          0x1234abcd",    ==, stdio_sprintf, buffer, "%#20x", 305441741);
  PRINTING_CHECK("          0xedcb5433",    ==, stdio_sprintf, buffer, "%#20x", 3989525555U);
  PRINTING_CHECK("          0X1234ABCD",    ==, stdio_sprintf, buffer, "%#20X", 305441741);
  PRINTING_CHECK("          0XEDCB5433",    ==, stdio_sprintf, buffer, "%#20X", 3989525555U);
}


PRINTF_TEST_CASE(padding_20_point_5)
{
  char buffer[base_buffer_size];
  PRINTING_CHECK("               01024",    ==, stdio_sprintf, buffer, "%20.5d", 1024);
  PRINTING_CHECK("              -01024",    ==, stdio_sprintf, buffer, "%20.5d", -1024);
  PRINTING_CHECK("               01024",    ==, stdio_sprintf, buffer, "%20.5i", 1024);
  PRINTING_CHECK("              -01024",    ==, stdio_sprintf, buffer, "%20.5i", -1024);
  PRINTING_CHECK("               01024",    ==, stdio_sprintf, buffer, "%20.5u", 1024);
  PRINTING_CHECK("          4294966272",    ==, stdio_sprintf, buffer, "%20.5u", 4294966272U);
  PRINTING_CHECK("               00777",    ==, stdio_sprintf, buffer, "%20.5o", 511);
  PRINTING_CHECK("         37777777001",    ==, stdio_sprintf, buffer, "%20.5o", 4294966785U);
  PRINTING_CHECK("            1234abcd",    ==, stdio_sprintf, buffer, "%20.5x", 305441741);
  PRINTING_CHECK("          00edcb5433",    ==, stdio_sprintf, buffer, "%20.10x", 3989525555U);
  PRINTING_CHECK("            1234ABCD",    ==, stdio_sprintf, buffer, "%20.5X", 305441741);
  PRINTING_CHECK("          00EDCB5433",    ==, stdio_sprintf, buffer, "%20.10X", 3989525555U);
}


PRINTF_TEST_CASE(padding_negative_numbers)
{
  char buffer[base_buffer_size];

// space padding
  PRINTING_CHECK("-5",                      ==, stdio_sprintf, buffer, "% 1d", -5);
  PRINTING_CHECK("-5",                      ==, stdio_sprintf, buffer, "% 2d", -5);
  PRINTING_CHECK(" -5",                     ==, stdio_sprintf, buffer, "% 3d", -5);
  PRINTING_CHECK("  -5",                    ==, stdio_sprintf, buffer, "% 4d", -5);

// zero padding
  PRINTING_CHECK("-5",                      ==, stdio_sprintf, buffer, "%01d", -5);
  PRINTING_CHECK("-5",                      ==, stdio_sprintf, buffer, "%02d", -5);
  PRINTING_CHECK("-05",                     ==, stdio_sprintf, buffer, "%03d", -5);
  PRINTING_CHECK("-005",                    ==, stdio_sprintf, buffer, "%04d", -5);
}


#if RCSW_CONFIG_STDIO_PRINTF_WITH_DEC || RCSW_CONFIG_STDIO_PRINTF_WITH_EXP

PRINTF_TEST_CASE(float_padding_negative_numbers)
{
  char buffer[base_buffer_size];

  // space padding
#if RCSW_CONFIG_STDIO_PRINTF_WITH_DEC
  PRINTING_CHECK("-5.0",                    ==, stdio_sprintf, buffer, "% 3.1f", -5.);
  PRINTING_CHECK("-5.0",                    ==, stdio_sprintf, buffer, "% 4.1f", -5.);
  PRINTING_CHECK(" -5.0",                   ==, stdio_sprintf, buffer, "% 5.1f", -5.);
#endif

#if RCSW_CONFIG_STDIO_PRINTF_WITH_EXP
  PRINTING_CHECK("    -5",                  ==, stdio_sprintf, buffer, "% 6.1g", -5.);
  PRINTING_CHECK("-5.0e+00",                ==, stdio_sprintf, buffer, "% 6.1e", -5.);
  PRINTING_CHECK("  -5.0e+00",              ==, stdio_sprintf, buffer, "% 10.1e", -5.);
#endif

  // zero padding
#if RCSW_CONFIG_STDIO_PRINTF_WITH_DEC
  PRINTING_CHECK("-5.0",                    ==, stdio_sprintf, buffer, "%03.1f", -5.);
  PRINTING_CHECK("-5.0",                    ==, stdio_sprintf, buffer, "%04.1f", -5.);
  PRINTING_CHECK("-05.0",                   ==, stdio_sprintf, buffer, "%05.1f", -5.);

  // zero padding no decimal point
  PRINTING_CHECK("-5",                      ==, stdio_sprintf, buffer, "%01.0f", -5.);
  PRINTING_CHECK("-5",                      ==, stdio_sprintf, buffer, "%02.0f", -5.);
  PRINTING_CHECK("-05",                     ==, stdio_sprintf, buffer, "%03.0f", -5.);
#endif

#if RCSW_CONFIG_STDIO_PRINTF_WITH_EXP
  PRINTING_CHECK("-005.0e+00",              ==, stdio_sprintf, buffer, "%010.1e", -5.);
  PRINTING_CHECK("-05E+00",                 ==, stdio_sprintf, buffer, "%07.0E", -5.);
  PRINTING_CHECK("-05",                     ==, stdio_sprintf, buffer, "%03.0g", -5.);
#endif
}

#endif // PRINTF_WITH_DECIMAL_SPECIFIERS || RCSW_CONFIG_STDIO_PRINTF_WITH_EXP

PRINTF_TEST_CASE(length)
{
  char buffer[base_buffer_size];
  PRINTING_CHECK("",                        ==, stdio_sprintf, buffer, "%.0s", mkstr("Hello testing"));
  PRINTING_CHECK("                    ",    ==, stdio_sprintf, buffer, "%20.0s", mkstr("Hello testing"));
  PRINTING_CHECK("",                        ==, stdio_sprintf, buffer, "%.s", mkstr("Hello testing"));
  PRINTING_CHECK("                    ",    ==, stdio_sprintf, buffer, "%20.s", mkstr("Hello testing"));
  PRINTING_CHECK("                1024",    ==, stdio_sprintf, buffer, "%20.0d", 1024);
  PRINTING_CHECK("               -1024",    ==, stdio_sprintf, buffer, "%20.0d", -1024);
  PRINTING_CHECK("                    ",    ==, stdio_sprintf, buffer, "%20.d", 0);
  PRINTING_CHECK("                1024",    ==, stdio_sprintf, buffer, "%20.0i", 1024);
  PRINTING_CHECK("               -1024",    ==, stdio_sprintf, buffer, "%20.i", -1024);
  PRINTING_CHECK("                    ",    ==, stdio_sprintf, buffer, "%20.i", 0);
  PRINTING_CHECK("                1024",    ==, stdio_sprintf, buffer, "%20.u", 1024);
  PRINTING_CHECK("          4294966272",    ==, stdio_sprintf, buffer, "%20.0u", 4294966272U);
  PRINTING_CHECK("                    ",    ==, stdio_sprintf, buffer, "%20.u", 0U);
  PRINTING_CHECK("                 777",    ==, stdio_sprintf, buffer, "%20.o", 511);
  PRINTING_CHECK("         37777777001",    ==, stdio_sprintf, buffer, "%20.0o", 4294966785U);
  PRINTING_CHECK("                    ",    ==, stdio_sprintf, buffer, "%20.o", 0U);
  PRINTING_CHECK("            1234abcd",    ==, stdio_sprintf, buffer, "%20.x", 305441741);
  PRINTING_CHECK("                                          1234abcd",
                                            ==, stdio_sprintf, buffer, "%50.x", 305441741);
  PRINTING_CHECK("                                          1234abcd     12345",
                                            ==, stdio_sprintf, buffer, "%50.x%10.u", 305441741, 12345);
  PRINTING_CHECK("            edcb5433",    ==, stdio_sprintf, buffer, "%20.0x", 3989525555U);
  PRINTING_CHECK("                    ",    ==, stdio_sprintf, buffer, "%20.x", 0U);
  PRINTING_CHECK("            1234ABCD",    ==, stdio_sprintf, buffer, "%20.X", 305441741);
  PRINTING_CHECK("            EDCB5433",    ==, stdio_sprintf, buffer, "%20.0X", 3989525555U);
  PRINTING_CHECK("                    ",    ==, stdio_sprintf, buffer, "%20.X", 0U);
}

#ifdef TEST_WITH_NON_STANDARD_FORMAT_STRINGS

PRINTF_TEST_CASE(length__non_standard_format)
{
  char buffer[base_buffer_size];
  PRINTING_CHECK("  ",                      ==, stdio_sprintf, buffer, "%02.0u", 0U);
  PRINTING_CHECK("  ",                      ==, stdio_sprintf, buffer, "%02.0d", 0);
}

#endif


#if RCSW_CONFIG_STDIO_PRINTF_WITH_DEC || PRINTF_WITH_EXP

PRINTF_TEST_CASE(infinity_and_not_a_number_values)
{
  char buffer[base_buffer_size];

  // test special-case floats using math.h macros
#if RCSW_CONFIG_STDIO_PRINTF_WITH_DEC
  PRINTING_CHECK("     nan",                ==, stdio_sprintf, buffer, "%8f", (double) NAN);
  PRINTING_CHECK("     inf",                ==, stdio_sprintf, buffer, "%8f", (double) INFINITY);
  PRINTING_CHECK("-inf    ",                ==, stdio_sprintf, buffer, "%-8f", (double) -INFINITY);
#endif
#if RCSW_CONFIG_STDIO_PRINTF_WITH_EXP
  PRINTING_CHECK("     nan",                ==, stdio_sprintf, buffer, "%8e", (double) NAN);
  PRINTING_CHECK("     inf",                ==, stdio_sprintf, buffer, "%8e", (double) INFINITY);
  PRINTING_CHECK("-inf    ",                ==, stdio_sprintf, buffer, "%-8e", (double) -INFINITY);
#endif
}

#endif // PRINTF_WITH_DECIMAL_SPECIFIERS || RCSW_CONFIG_STDIO_PRINTF_WITH_EXP

#if RCSW_CONFIG_STDIO_PRINTF_WITH_DEC

PRINTF_TEST_CASE(floating_point_specifiers_with_31_to_32_bit_integer_values)
{
  char buffer[base_buffer_size];
#if RCSW_CONFIG_STDIO_PRINTF_EXP_DIGIT_THRESH >= 10
  PRINTING_CHECK("2147483647",              ==, stdio_sprintf, buffer, "%.10f", 2147483647.0); // 2^31 - 1
  PRINTING_CHECK("2147483648",              ==, stdio_sprintf, buffer, "%.10f", 2147483648.0); // 2^31
  PRINTING_CHECK("4294967295",              ==, stdio_sprintf, buffer, "%.10f", 4294967295.0); // 2^32 - 1
  PRINTING_CHECK("4294967296",              ==, stdio_sprintf, buffer, "%.10f", 4294967296.0); // 2^32
#else
  PRINTING_CHECK("2.1474836470e+09",        ==, stdio_sprintf, buffer, "%.10f", 2147483647.0); // 2^31 - 1
  PRINTING_CHECK("2.1474836480e+09",        ==, stdio_sprintf, buffer, "%.10f", 2147483648.0); // 2^31
  PRINTING_CHECK("4.2949672950e+09",        ==, stdio_sprintf, buffer, "%.10f", 4294967295.0); // 2^32 - 1
  PRINTING_CHECK("4.2949672960e+09",        ==, stdio_sprintf, buffer, "%.10f", 4294967296.0); // 2^32
#endif
  PRINTING_CHECK("2147483647",              ==, stdio_sprintf, buffer, "%.10g", 2147483647.0); // 2^31 - 1
  PRINTING_CHECK("2147483648",              ==, stdio_sprintf, buffer, "%.10g", 2147483648.0); // 2^31
  PRINTING_CHECK("4294967295",              ==, stdio_sprintf, buffer, "%.10g", 4294967295.0); // 2^32 - 1
  PRINTING_CHECK("4294967296",              ==, stdio_sprintf, buffer, "%.10g", 4294967296.0); // 2^32
}

#endif

#if RCSW_CONFIG_STDIO_PRINTF_WITH_EXP

PRINTF_TEST_CASE(tiny_floating_point_values)
{
  char buffer[base_buffer_size];
//  boltzman_s_constant = 1.380651569e-23;

  PRINTING_CHECK("1e-23",                   ==, stdio_sprintf, buffer, "%.0e",  1.380651569e-23);
  PRINTING_CHECK("1.4e-23",                 ==, stdio_sprintf, buffer, "%.1e",  1.380651569e-23);
  PRINTING_CHECK("1.38e-23",                ==, stdio_sprintf, buffer, "%.2e",  1.380651569e-23);
  PRINTING_CHECK("1.381e-23",               ==, stdio_sprintf, buffer, "%.3e",  1.380651569e-23);
  PRINTING_CHECK("1.3807e-23",              ==, stdio_sprintf, buffer, "%.4e",  1.380651569e-23);
  PRINTING_CHECK("1.38065e-23",             ==, stdio_sprintf, buffer, "%.5e",  1.380651569e-23);
  PRINTING_CHECK("1.380652e-23",            ==, stdio_sprintf, buffer, "%.6e",  1.380651569e-23);
  PRINTING_CHECK("1.3806516e-23",           ==, stdio_sprintf, buffer, "%.7e",  1.380651569e-23);
  PRINTING_CHECK("1.38065157e-23",          ==, stdio_sprintf, buffer, "%.8e",  1.380651569e-23);
  PRINTING_CHECK("1.380651569e-23",         ==, stdio_sprintf, buffer, "%.9e",  1.380651569e-23);
  PRINTING_CHECK("1.3806515690e-23",        ==, stdio_sprintf, buffer, "%.10e", 1.380651569e-23);
  PRINTING_CHECK("1.38065156900e-23",       ==, stdio_sprintf, buffer, "%.11e", 1.380651569e-23);
// These go beyond our precision abilities; enable them, and they will likely fail.
//  PRINTING_CHECK("1.380651569000e-23",    ==, stdio_sprintf, buffer, "%.12e", 1.380651569e-23);
//  PRINTING_CHECK("1.3806515690000e-23",   ==, stdio_sprintf, buffer, "%.13e", 1.380651569e-23);
//  PRINTING_CHECK("1.38065156900000e-23",  ==, stdio_sprintf, buffer, "%.14e", 1.380651569e-23);
//  PRINTING_CHECK("1.380651569000000e-23", ==, stdio_sprintf, buffer, "%.15e", 1.380651569e-23);
//  PRINTING_CHECK("1.3806515690000000e-23", ==, stdio_sprintf, buffer, "%.16e", 1.380651569e-23);
}

#endif

#if RCSW_CONFIG_STDIO_PRINTF_WITH_DEC

PRINTF_TEST_CASE(fallback_from_decimal_to_exponential)
{
  char buffer[base_buffer_size];
  CAPTURE_AND_PRINT(stdio_sprintf, buffer, "%.0f", (double) ((int64_t) 1 * 1000));
  if (RCSW_CONFIG_STDIO_PRINTF_EXP_DIGIT_THRESH < 3) {
    CATCH_CHECK(!strcmp(buffer, "1e+3"));
  } else {
    CATCH_CHECK(!strcmp(buffer, "1000"));
  }

  CAPTURE_AND_PRINT(stdio_sprintf, buffer, "%.0f", (double) ((int64_t) 1 * 1000 * 1000));
  if (RCSW_CONFIG_STDIO_PRINTF_EXP_DIGIT_THRESH < 6) {
    CATCH_CHECK(!strcmp(buffer, "1e+6"));
  } else {
    CATCH_CHECK(!strcmp(buffer, "1000000"));
  }

  CAPTURE_AND_PRINT(stdio_sprintf, buffer, "%.0f", (double) ((int64_t) 1 * 1000 * 1000 * 1000));
  if (RCSW_CONFIG_STDIO_PRINTF_EXP_DIGIT_THRESH < 9) {
    CATCH_CHECK(!strcmp(buffer, "1e+9"));
  } else {
    CATCH_CHECK(!strcmp(buffer, "1000000000"));
  }

  CAPTURE_AND_PRINT(stdio_sprintf, buffer, "%.0f", (double) ((int64_t) 1 * 1000 * 1000 * 1000 * 1000));
  if (RCSW_CONFIG_STDIO_PRINTF_EXP_DIGIT_THRESH < 12) {
#if RCSW_CONFIG_STDIO_PRINTF_WITH_EXP
    CATCH_CHECK(!strcmp(buffer, "1e+12"));
#else
    CATCH_CHECK(!strcmp(buffer, ""));
#endif
  } else {
    CATCH_CHECK(!strcmp(buffer, "1000000000000"));
  }

  CAPTURE_AND_PRINT(stdio_sprintf, buffer, "%.0f", (double) ((int64_t) 1 * 1000 * 1000 * 1000 * 1000 * 1000));
  if (RCSW_CONFIG_STDIO_PRINTF_EXP_DIGIT_THRESH < 15) {
#if RCSW_CONFIG_STDIO_PRINTF_WITH_EXP
    CATCH_CHECK(!strcmp(buffer, "1e+15"));
#else
    CATCH_CHECK(!strcmp(buffer, ""));
#endif
  } else {
    CATCH_CHECK(!strcmp(buffer, "1000000000000000"));
  }

  // A value which should Should definitely be out of range for float
  CAPTURE_AND_PRINT(stdio_sprintf, buffer, "%.1f", 1E20);
#if RCSW_CONFIG_STDIO_PRINTF_WITH_EXP
  CATCH_CHECK(!strcmp(buffer, "1.0e+20"));
#else
  CATCH_CHECK(!strcmp(buffer, ""));
#endif

}

#endif // PRINTF_WITH_DECIMAL_SPECIFIERS

#if RCSW_CONFIG_STDIO_PRINTF_WITH_DEC || RCSW_CONFIG_STDIO_PRINTF_WITH_EXP

PRINTF_TEST_CASE(floating_point_specifiers_precision_and_flags)
{
  char buffer[base_buffer_size];
#if RCSW_CONFIG_STDIO_PRINTF_WITH_DEC
  PRINTING_CHECK("3.1415",                  ==, stdio_sprintf, buffer, "%.4f", 3.1415354);
  PRINTING_CHECK("30343.142",               ==, stdio_sprintf, buffer, "%.3f", 30343.1415354);
  PRINTING_CHECK("34",                      ==, stdio_sprintf, buffer, "%.0f", 34.1415354);
  PRINTING_CHECK("1",                       ==, stdio_sprintf, buffer, "%.0f", 1.3);
  PRINTING_CHECK("2",                       ==, stdio_sprintf, buffer, "%.0f", 1.55);
  PRINTING_CHECK("1.6",                     ==, stdio_sprintf, buffer, "%.1f", 1.64);
  PRINTING_CHECK("42.90",                   ==, stdio_sprintf, buffer, "%.2f", 42.8952);
  PRINTING_CHECK("42.895200000",            ==, stdio_sprintf, buffer, "%.9f", 42.8952);
  PRINTING_CHECK("42.8952230000",           ==, stdio_sprintf, buffer, "%.10f", 42.895223);
  PRINTING_CHECK("42.895223123457",         ==, stdio_sprintf, buffer, "%.12f", 42.89522312345678);
  PRINTING_CHECK("42477.371093750000000",   ==, stdio_sprintf, buffer, "%020.15f", 42477.37109375);
  PRINTING_CHECK("42.895223876543",         ==, stdio_sprintf, buffer, "%.12f", 42.89522387654321);
  PRINTING_CHECK(" 42.90",                  ==, stdio_sprintf, buffer, "%6.2f", 42.8952);
  PRINTING_CHECK("+42.90",                  ==, stdio_sprintf, buffer, "%+6.2f", 42.8952);
  PRINTING_CHECK("+42.9",                   ==, stdio_sprintf, buffer, "%+5.1f", 42.9252);
  PRINTING_CHECK("42.500000",               ==, stdio_sprintf, buffer, "%f", 42.5);
  PRINTING_CHECK("42.5",                    ==, stdio_sprintf, buffer, "%.1f", 42.5);
  PRINTING_CHECK("42167.000000",            ==, stdio_sprintf, buffer, "%f", 42167.0);
  PRINTING_CHECK("-12345.987654321",        ==, stdio_sprintf, buffer, "%.9f", -12345.987654321);
  PRINTING_CHECK("4.0",                     ==, stdio_sprintf, buffer, "%.1f", 3.999);
  PRINTING_CHECK("4",                       ==, stdio_sprintf, buffer, "%.0f", 3.5);
  PRINTING_CHECK("4",                       ==, stdio_sprintf, buffer, "%.0f", 4.5);
  PRINTING_CHECK("3",                       ==, stdio_sprintf, buffer, "%.0f", 3.49);
  PRINTING_CHECK("3.5",                     ==, stdio_sprintf, buffer, "%.1f", 3.49);
  PRINTING_CHECK("a0.5  ",                  ==, stdio_sprintf, buffer, "a%-5.1f", 0.5);
  PRINTING_CHECK("a0.5  end",               ==, stdio_sprintf, buffer, "a%-5.1fend", 0.5);
#endif
#if RCSW_CONFIG_STDIO_PRINTF_WITH_EXP
  PRINTING_CHECK("0.5",                     ==, stdio_sprintf, buffer, "%.4g", 0.5);
  PRINTING_CHECK("1",                       ==, stdio_sprintf, buffer, "%.4g", 1.0);
  PRINTING_CHECK("12345.7",                 ==, stdio_sprintf, buffer, "%G", 12345.678);
  PRINTING_CHECK("12345.68",                ==, stdio_sprintf, buffer, "%.7G", 12345.678);
  PRINTING_CHECK("1.2346E+08",              ==, stdio_sprintf, buffer, "%.5G", 123456789.);
  PRINTING_CHECK("12345",                   ==, stdio_sprintf, buffer, "%.6G", 12345.);
  PRINTING_CHECK("  +1.235e+08",            ==, stdio_sprintf, buffer, "%+12.4g", 123456789.);
  PRINTING_CHECK("0.0012",                  ==, stdio_sprintf, buffer, "%.2G", 0.001234);
  PRINTING_CHECK(" +0.001234",              ==, stdio_sprintf, buffer, "%+10.4G", 0.001234);
  PRINTING_CHECK("+001.234e-05",            ==, stdio_sprintf, buffer, "%+012.4g", 0.00001234);
  // Note: The following two values are _barely_ normal; make their mantissa 1.1 and they lose their normality.
  PRINTING_CHECK("-1.23e-308",              ==, stdio_sprintf, buffer, "%.3g", -1.2345e-308);
  PRINTING_CHECK("+1.230E+308",             ==, stdio_sprintf, buffer, "%+.3E", 1.23e+308);
  PRINTING_CHECK("1.000e+01",               ==, stdio_sprintf, buffer, "%.3e", 9.9996);
  PRINTING_CHECK("0",                       ==, stdio_sprintf, buffer, "%g", 0.);
  PRINTING_CHECK("-0",                      ==, stdio_sprintf, buffer, "%g", -0.);
  PRINTING_CHECK("+0",                      ==, stdio_sprintf, buffer, "%+g", 0.);
  PRINTING_CHECK("-0",                      ==, stdio_sprintf, buffer, "%+g", -0.);
  PRINTING_CHECK("-4e+04",                  ==, stdio_sprintf, buffer, "%.1g", -40661.5);
  PRINTING_CHECK("-4.e+04",                 ==, stdio_sprintf, buffer, "%#.1g", -40661.5);
  PRINTING_CHECK("100.",                    ==, stdio_sprintf, buffer, "%#.3g", 99.998580932617187500);
  // Note: The following value is _barely_ normal; make the mantissa 1.1 and it loses its normality.
  PRINTING_CHECK("1.2345678901e-308",       ==, stdio_sprintf, buffer, "%.10e", 1.2345678901e-308);
  // Rounding-focused checks
  PRINTING_CHECK("4.895512e+04",            ==, stdio_sprintf, buffer, "%e", 48955.125);
  PRINTING_CHECK("9.2524e+04",              ==, stdio_sprintf, buffer, "%.4e", 92523.5);
  PRINTING_CHECK("-8.380923438e+04",        ==, stdio_sprintf, buffer, "%.9e", -83809.234375);
#endif
}
#endif // PRINTF_WITH_DECIMAL_SPECIFIERS || RCSW_CONFIG_STDIO_PRINTF_WITH_EXP

PRINTF_TEST_CASE(integer_types)
{
  char buffer[base_buffer_size];
  PRINTING_CHECK("0",                       ==, stdio_sprintf, buffer, "%i", 0);
  PRINTING_CHECK("1234",                    ==, stdio_sprintf, buffer, "%i", 1234);
  PRINTING_CHECK("32767",                   ==, stdio_sprintf, buffer, "%i", 32767);
  PRINTING_CHECK("-32767",                  ==, stdio_sprintf, buffer, "%i", -32767);
  PRINTING_CHECK("30",                      ==, stdio_sprintf, buffer, "%li", 30L);
  PRINTING_CHECK("-2147483647",             ==, stdio_sprintf, buffer, "%li", -2147483647L);
  PRINTING_CHECK("2147483647",              ==, stdio_sprintf, buffer, "%li", 2147483647L);
#if RCSW_CONFIG_STDIO_PRINTF_WITH_LL
  PRINTING_CHECK("30",                      ==, stdio_sprintf, buffer, "%lli", 30LL);
  PRINTING_CHECK("-9223372036854775807",    ==, stdio_sprintf, buffer, "%lli", -9223372036854775807LL);
  PRINTING_CHECK("9223372036854775807",     ==, stdio_sprintf, buffer, "%lli", 9223372036854775807LL);
#endif
  PRINTING_CHECK("100000",                  ==, stdio_sprintf, buffer, "%lu", 100000L);
  PRINTING_CHECK("4294967295",              ==, stdio_sprintf, buffer, "%lu", 0xFFFFFFFFL);
#if RCSW_CONFIG_STDIO_PRINTF_WITH_LL
  PRINTING_CHECK("281474976710656",         ==, stdio_sprintf, buffer, "%llu", 281474976710656LLU);
  PRINTING_CHECK("18446744073709551615",    ==, stdio_sprintf, buffer, "%llu", 18446744073709551615LLU);
#endif
  PRINTING_CHECK("2147483647",              ==, stdio_sprintf, buffer, "%zu", (size_t) 2147483647UL);
  PRINTING_CHECK("2147483647",              ==, stdio_sprintf, buffer, "%zd", (size_t) 2147483647UL);
  PRINTING_CHECK("-2147483647",             ==, stdio_sprintf, buffer, "%zi", (ssize_t) -2147483647L);
  PRINTING_CHECK("165140",                  ==, stdio_sprintf, buffer, "%o", 60000);
  PRINTING_CHECK("57060516",                ==, stdio_sprintf, buffer, "%lo", 12345678L);
  PRINTING_CHECK("12345678",                ==, stdio_sprintf, buffer, "%lx", 0x12345678L);
#if RCSW_CONFIG_STDIO_PRINTF_WITH_LL
  PRINTING_CHECK("1234567891234567",        ==, stdio_sprintf, buffer, "%llx", 0x1234567891234567LLU);
#endif
  PRINTING_CHECK("abcdefab",                ==, stdio_sprintf, buffer, "%lx", 0xabcdefabL);
  PRINTING_CHECK("ABCDEFAB",                ==, stdio_sprintf, buffer, "%lX", 0xabcdefabL);
  PRINTING_CHECK("v",                       ==, stdio_sprintf, buffer, "%c", 'v');
  PRINTING_CHECK("wv",                      ==, stdio_sprintf, buffer, "%cv", 'w');
  PRINTING_CHECK("A Test",                  ==, stdio_sprintf, buffer, "%s", mkstr("A Test"));
  PRINTING_CHECK("255",                     ==, stdio_sprintf, buffer, "%hhu", (unsigned char) 0xFFU);
  PRINTING_CHECK("4660",                    ==, stdio_sprintf, buffer, "%hu", (unsigned short) 0x1234u);
  PRINTING_CHECK("Test100 65535",           ==, stdio_sprintf, buffer, "%s%hhi %hu", mkstr("Test"), (char) 100, (unsigned short) 0xFFFF);
  PRINTING_CHECK("a",                       ==, stdio_sprintf, buffer, "%tx", &buffer[10] - &buffer[0]);
  PRINTING_CHECK("-2147483647",             ==, stdio_sprintf, buffer, "%ji", (intmax_t) -2147483647L);
}

#ifdef TEST_WITH_NON_STANDARD_FORMAT_STRINGS

PRINTF_TEST_CASE(types__non_standard_format)
{
  char buffer[base_buffer_size];
  PRINTING_CHECK("1110101001100000",        ==, stdio_sprintf, buffer, "%b", 60000);
  PRINTING_CHECK("101111000110000101001110", ==, stdio_sprintf, buffer, "%lb", 12345678L);
}

#endif

PRINTF_TEST_CASE(pointer)
{
  char buffer[base_buffer_size];

  CAPTURE_AND_PRINT(stdio_sprintf, buffer, "%p", (void *) 0x1234U);
  if (sizeof(void *) == 4U) {
    CATCH_CHECK(!strcmp(buffer, "0x00001234"));
  } else {
    CATCH_CHECK(!strcmp(buffer, "0x0000000000001234"));
  }

  CAPTURE_AND_PRINT(stdio_sprintf, buffer, "%p", (void *) 0x12345678U);
  if (sizeof(void *) == 4U) {
    CATCH_CHECK(!strcmp(buffer, "0x12345678"));
  } else {
    CATCH_CHECK(!strcmp(buffer, "0x0000000012345678"));
  }

  CAPTURE_AND_PRINT(stdio_sprintf, buffer, "%p-%p", (void *) 0x12345678U, (void *) 0x7EDCBA98U);
  if (sizeof(void *) == 4U) {
    CATCH_CHECK(!strcmp(buffer, "0x12345678-0x7edcba98"));
  } else {
    CATCH_CHECK(!strcmp(buffer, "0x0000000012345678-0x000000007edcba98"));
  }

  CAPTURE_AND_PRINT(stdio_sprintf, buffer, "%p", (void *) (uintptr_t) 0xFFFFFFFFU);
  if (sizeof(uintptr_t) == sizeof(uint64_t)) {
    CATCH_CHECK(!strcmp(buffer, "0x00000000ffffffff"));
  } else {
    CATCH_CHECK(!strcmp(buffer, "0xffffffff"));
  }
  PRINTING_CHECK("(nil)",                   ==, stdio_sprintf, buffer, "%p", (const void *) NULL);
}

#ifdef TEST_WITH_NON_STANDARD_FORMAT_STRINGS

PRINTF_TEST_CASE(unknown_flag__non_standard_format)
{
  char buffer[base_buffer_size];
  PRINTING_CHECK("kmarco",                  ==, stdio_sprintf, buffer, "%kmarco", 42, 37);
}

#endif

PRINTF_TEST_CASE(string_length)
{
  char buffer[base_buffer_size];
  PRINTING_CHECK("This",                    ==, stdio_sprintf, buffer, "%.4s", mkstr("This is a test"));
  PRINTING_CHECK("test",                    ==, stdio_sprintf, buffer, "%.4s", mkstr("test"));
  PRINTING_CHECK("123",                     ==, stdio_sprintf, buffer, "%.7s", mkstr("123"));
  PRINTING_CHECK("",                        ==, stdio_sprintf, buffer, "%.7s", mkstr(""));
  PRINTING_CHECK("1234ab",                  ==, stdio_sprintf, buffer, "%.4s%.2s", mkstr("123456"), mkstr("abcdef"));
  PRINTING_CHECK("123",                     ==, stdio_sprintf, buffer, "%.*s", 3, mkstr("123456"));

  PRINTING_CHECK("(null)",                  ==, stdio_sprintf, buffer, "%.*s", 3, (const char *) NULL);
}

#ifdef TEST_WITH_NON_STANDARD_FORMAT_STRINGS

PRINTF_TEST_CASE(string_length__non_standard_format)
{
  char buffer[base_buffer_size];
  DISABLE_WARNING_PUSH
  DISABLE_WARNING_PRINTF_FORMAT
  DISABLE_WARNING_PRINTF_FORMAT_EXTRA_ARGS
  DISABLE_WARNING_PRINTF_FORMAT_INVALID_SPECIFIER
  PRINTING_CHECK(".2s",                     ==, stdio_sprintf, buffer, "%.4.2s", "123456");
  DISABLE_WARNING_POP
}

#endif


PRINTF_TEST_CASE(buffer_length)
{
  char buffer[base_buffer_size];
  int ret;

  CAPTURE_AND_PRINT_WITH_RETVAL(ret, stdio_snprintf, nullptr, 10, "%s", mkstr("Test"));
  CATCH_CHECK(ret == 4);
  CAPTURE_AND_PRINT_WITH_RETVAL(ret, stdio_snprintf, nullptr, (size_t) 0, "%s", mkstr("Test"));
  CATCH_CHECK(ret == 4);

  buffer[0] = (char) 0xA5;
  CAPTURE_AND_PRINT_WITH_RETVAL(ret, stdio_snprintf, buffer, (size_t) 0, "%s", mkstr("Test"));
  CATCH_CHECK(buffer[0] == (char) 0xA5);
  CATCH_CHECK(ret == 4);

  buffer[0] = (char) 0xCC;
  CAPTURE_AND_PRINT_WITH_RETVAL(ret, stdio_snprintf, buffer, 1, "%s", mkstr("Test"));
  CATCH_CHECK(buffer[0] == '\0');
  PRINTING_CHECK_WITH_BUF_SIZE("H",         ==, stdio_snprintf, buffer, 2, "%s", mkstr("Hello"));

  CAPTURE_AND_PRINT_WITH_RETVAL(ret, stdio_snprintf, buffer, 2, "%s", (const char *) NULL);
  CATCH_CHECK(!strcmp(buffer, "("));
}


PRINTF_TEST_CASE(misc)
{
  char buffer[base_buffer_size];
  PRINTING_CHECK("53000atest-20 bit",       ==, stdio_sprintf, buffer, "%u%u%ctest%d %s", 5, 3000, 'a', -20, mkstr("bit"));
#if RCSW_CONFIG_STDIO_PRINTF_WITH_DEC
  PRINTING_CHECK("0.33",                    ==, stdio_sprintf, buffer, "%.*f", 2, 0.33333333);
  PRINTING_CHECK("1",                       ==, stdio_sprintf, buffer, "%.*d", -1, 1);
  PRINTING_CHECK("foo",                     ==, stdio_sprintf, buffer, "%.3s", mkstr("foobar"));
  PRINTING_CHECK(" ",                       ==, stdio_sprintf, buffer, "% .0d", 0);
  PRINTING_CHECK("     00004",              ==, stdio_sprintf, buffer, "%10.5d", 4);
  PRINTING_CHECK("hi x",                    ==, stdio_sprintf, buffer, "%*sx", -3, mkstr("hi"));
  PRINTING_CHECK("00123               ",    ==, stdio_sprintf, buffer, "%-20.5i", 123);
  PRINTING_CHECK("-67224.546875000000000000", ==, stdio_sprintf, buffer, "%.18f", -67224.546875);
#endif
#if RCSW_CONFIG_STDIO_PRINTF_WITH_EXP
  PRINTING_CHECK("0.33",                    ==, stdio_sprintf, buffer, "%.*g", 2, 0.33333333);
  PRINTING_CHECK("3.33e-01",                ==, stdio_sprintf, buffer, "%.*e", 2, 0.33333333);
  PRINTING_CHECK("0.000000e+00",            ==, stdio_sprintf, buffer, "%e", 0.0);
  PRINTING_CHECK("-0.000000e+00",           ==, stdio_sprintf, buffer, "%e", -0.0);
#endif
}

PRINTF_TEST_CASE(extremal_signed_integer_values)
{
  char buffer[base_buffer_size];
  char expected[base_buffer_size];

  std::sprintf(expected, "%hhd", std::numeric_limits<char>::max());
  PRINTING_CHECK(expected,                  ==, stdio_sprintf, buffer, "%hhd", std::numeric_limits<char>::max());

  std::sprintf(expected, "%hd", std::numeric_limits<short int>::max());
  PRINTING_CHECK(expected,                  ==, stdio_sprintf, buffer, "%hd", std::numeric_limits<short int>::max());

  std::sprintf(expected, "%hd", std::numeric_limits<short int>::max());
  PRINTING_CHECK(expected,                  ==, stdio_sprintf, buffer, "%hd", std::numeric_limits<short int>::max());

  std::sprintf(expected, "%d", std::numeric_limits<int>::min());
  PRINTING_CHECK(expected,                  ==, stdio_sprintf, buffer, "%d", std::numeric_limits<int>::min());

  std::sprintf(expected, "%d", std::numeric_limits<int>::max());
  PRINTING_CHECK(expected,                  ==, stdio_sprintf, buffer, "%d", std::numeric_limits<int>::max());

  std::sprintf(expected, "%ld", std::numeric_limits<long int>::min());
  PRINTING_CHECK(expected,                  ==, stdio_sprintf, buffer, "%ld", std::numeric_limits<long int>::min());

  std::sprintf(expected, "%ld", std::numeric_limits<long int>::max());
  PRINTING_CHECK(expected,                  ==, stdio_sprintf, buffer, "%ld", std::numeric_limits<long int>::max());

#if RCSW_CONFIG_STDIO_PRINTF_WITH_LL
  std::sprintf(expected, "%lld", std::numeric_limits<long long int>::min());
  PRINTING_CHECK(expected,                  ==, stdio_sprintf, buffer, "%lld", std::numeric_limits<long long int>::min());

  std::sprintf(expected, "%lld", std::numeric_limits<long long int>::max());
  PRINTING_CHECK(expected,                  ==, stdio_sprintf, buffer, "%lld", std::numeric_limits<long long int>::max());
#endif
}

PRINTF_TEST_CASE(extremal_unsigned_integer_values)
{
  char buffer[base_buffer_size];
  char expected[base_buffer_size];

  std::sprintf(expected, "%hhu", std::numeric_limits<char unsigned>::max());
  PRINTING_CHECK(expected,                  ==, stdio_sprintf, buffer, "%hhu", std::numeric_limits<char unsigned>::max());

  std::sprintf(expected, "%hu", std::numeric_limits<short unsigned>::max());
  PRINTING_CHECK(expected,                  ==, stdio_sprintf, buffer, "%hu", std::numeric_limits<short unsigned>::max());

  std::sprintf(expected, "%u", std::numeric_limits<unsigned>::max());
  PRINTING_CHECK(expected,                  ==, stdio_sprintf, buffer, "%u", std::numeric_limits<unsigned>::max());

  std::sprintf(expected, "%lu", std::numeric_limits<long unsigned>::max());
  PRINTING_CHECK(expected,                  ==, stdio_sprintf, buffer, "%lu", std::numeric_limits<long unsigned>::max());

#if RCSW_CONFIG_STDIO_PRINTF_WITH_LL
  std::sprintf(expected, "%llu", std::numeric_limits<long long unsigned>::max());
  PRINTING_CHECK(expected,                  ==, stdio_sprintf, buffer, "%llu", std::numeric_limits<long long unsigned>::max());
#endif
}

#ifdef TEST_WITH_NON_STANDARD_FORMAT_STRINGS
DISABLE_WARNING_POP
#endif
