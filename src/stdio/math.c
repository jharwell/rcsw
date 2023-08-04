/**
 * \file math.c
 *
 * \author (c) Eyal Rozenberg <eyalroz1@gmx.com>
 *             2021-2022, Haifa, Palestine/Israel
 * \author (c) Marco Paland (info@paland.com)
 *             2014-2019, PALANDesign Hannover, Germany
 *
 * SPDX-License Identifier:
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/stdio/math.h"

#include "rcsw/stdio/printf_config.h"

/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS

double stdio_log10(double positive_number) {
  // The implementation follows David Gay (https://www.ampl.com/netlib/fp/dtoa.c).
  //
  // Since log_10 ( M * 2^x ) = log_10(M) + x , we can separate the components of
  // our input number, and need only solve log_10(M) for M between 1 and 2 (as
  // the base-2 mantissa is always 1-point-something). In that limited range, a
  // Taylor series expansion of log10(x) should serve us well enough; and we'll
  // take the mid-point, 1.5, as the point of expansion.

  double_with_bit_access dwba = get_bit_access(positive_number);
  // based on the algorithm by David Gay (https://www.ampl.com/netlib/fp/dtoa.c)
  int exp2 = stdio_exp2(dwba);
  // drop the exponent, so dwba.F comes into the range [1,2)
  dwba.U = (dwba.U & (((double_uint_t)(1) << DOUBLE_STORED_MANTISSA_BITS) - 1U)) |
           ((double_uint_t)DOUBLE_BASE_EXPONENT << DOUBLE_STORED_MANTISSA_BITS);
  double z = (dwba.F - 1.5);
  return (
      // Taylor expansion around 1.5:
      0.1760912590556812420 // Expansion term 0: ln(1.5)            / ln(10)
      + z * 0.2895296546021678851 // Expansion term 1: (M - 1.5)   * 2/3  / ln(10)
#if RCSW_CONFIG_STDIO_MATH_LOG10_TERMS > 2
      - z * z * 0.0965098848673892950 // Expansion term 2: (M - 1.5)^2 * 2/9  /
  // ln(10)
#if RCSW_CONFIG_STDIO_MATH_LOG10_TERMS > 3
      + z * z * z * 0.0428932821632841311 // Expansion term 2: (M - 1.5)^3 * 8/81
  // / ln(10)
#endif
#endif
      // exact log_2 of the exponent x, with logarithm base change
      + exp2 * 0.30102999566398119521 // = exp2 * log_10(2) = exp2 * ln(2)/ln(10)
  );
}

int stdio_floor(double x) {
  if (x >= 0) {
    return (int)x;
  }
  int n = (int)x;

  RCSW_WARNING_DISABLE_PUSH()
  RCSW_WARNING_DISABLE_FLOAT_EQUAL()
  return (((double)n) == x) ? n : n - 1;
  RCSW_WARNING_DISABLE_POP()
}

double stdio_pow10(int floored_exp10) {
  // A crude hack for avoiding undesired behavior with barely-normal or
  // slightly-subnormal values.
  if (floored_exp10 == DOUBLE_MAX_SUBNORMAL_EXPONENT_OF_10) {
    return DOUBLE_MAX_SUBNORMAL_POWER_OF_10;
  }
  // Compute 10^(floored_exp10) but (try to) make sure that doesn't overflow
  double_with_bit_access dwba;
  int exp2 = stdio_floor(floored_exp10 * 3.321928094887362 + 0.5);
  const double z = floored_exp10 * 2.302585092994046 - exp2 * 0.6931471805599453;
  const double z2 = z * z;
  dwba.U = ((double_uint_t)(exp2) + DOUBLE_BASE_EXPONENT)
           << DOUBLE_STORED_MANTISSA_BITS;
  // compute exp(z) using continued fractions,
  // see
  // https://en.wikipedia.org/wiki/Exponential_function#Continued_fractions_for_ex
  dwba.F *= 1 + 2 * z / (2 - z + (z2 / (6 + (z2 / (10 + z2 / 14)))));
  return dwba.F;
}

END_C_DECLS
