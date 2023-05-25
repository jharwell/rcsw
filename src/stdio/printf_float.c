/**
 * \file printf_float.c
 *
 * \copyright 2023 John Harwell, All rights reserved.
 *
 * SPDX-License Identifier:
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/stdio/printf.h"
#include "rcsw/stdio/printf_float.h"
#include "rcsw/stdio/printf_internal.h"
#include "rcsw/stdio/math.h"

/*******************************************************************************
 * Misc. Configuration
 ******************************************************************************/
// The following will convert the number-of-digits into an exponential-notation
// literal
#define PRINTF_FLOAT_NOTATION_THRESHOLD                 \
  RCSW_JOIN(1e, PRINTF_MAX_INTEGRAL_DIGITS_FOR_DECIMAL)

#define NUM_DECIMAL_DIGITS_IN_INT64_T 18
#define PRINTF_MAX_PRECOMPUTED_POWER_OF_10  NUM_DECIMAL_DIGITS_IN_INT64_T
#define PRINTF_MAX_SUPPORTED_PRECISION NUM_DECIMAL_DIGITS_IN_INT64_T - 1

/*******************************************************************************
 * Type Definitions
 ******************************************************************************/
// Stores a fixed-precision representation of a double relative
// to a fixed precision (which cannot be determined by examining this structure)
struct double_components {
  int_fast64_t integral;
  int_fast64_t fractional;
  // ... truncation of the actual fractional part of the double value, scaled
  // by the precision value
  bool_t is_negative;
};

/*******************************************************************************
 * Global Variables
 ******************************************************************************/
static const double powers_of_10[NUM_DECIMAL_DIGITS_IN_INT64_T] = {
  1e00, 1e01, 1e02, 1e03, 1e04, 1e05, 1e06, 1e07, 1e08,
  1e09, 1e10, 1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17
};

/*******************************************************************************
 * Misc. Private Functions
 ******************************************************************************/
/**
 * \brief Break up a double number - which is known to be a finite non-negative
 * number - into its base-10 parts: integral - before the decimal point, and
 * fractional - after it.
 *
 * Taken the precision into account, but does not change it even internally.
 */
static struct double_components get_components(double number, printf_size_t precision)
{
  struct double_components number_;
  number_.is_negative = stdio_sign_bit(number);
  double abs_number = (number_.is_negative) ? -number : number;
  number_.integral = (int_fast64_t)abs_number;
  double remainder = (abs_number - (double) number_.integral) * powers_of_10[precision];
  number_.fractional = (int_fast64_t)remainder;

  remainder -= (double) number_.fractional;

  if (remainder > 0.5) {
    ++number_.fractional;
    // handle rollover, e.g. case 0.99 with precision 1 is 1.0
    if ((double) number_.fractional >= powers_of_10[precision]) {
      number_.fractional = 0;
      ++number_.integral;
    }
  }
  else if ((remainder == 0.5) && ((number_.fractional == 0U) || (number_.fractional & 1U))) {
    // if halfway, round up if odd OR if last digit is 0
    ++number_.fractional;
  }

  if (precision == 0U) {
    remainder = abs_number - (double) number_.integral;
    if ((!(remainder < 0.5) || (remainder > 0.5)) && (number_.integral & 1)) {
      // exactly 0.5 and ODD, then round up
      // 1.5 -> 2, but 2.5 -> 2
      ++number_.integral;
    }
  }
  return number_;
}

static void print_broken_up_decimal(struct double_components number_,
                                    struct printf_output_gadget* output,
                                    printf_size_t precision,
                                    printf_size_t width,
                                    printf_flags_t flags,
                                    char *buf,
                                    printf_size_t len)
{
  if (precision != 0U) {
    // do fractional part, as an unsigned number

    printf_size_t count = precision;

    // %g/%G mandates we skip the trailing 0 digits...
    if ((flags & FLAGS_ADAPT_EXP) && !(flags & FLAGS_HASH) && (number_.fractional > 0)) {
      while(TRUE) {
        int_fast64_t digit = number_.fractional % 10U;
        if (digit != 0) {
          break;
        }
        --count;
        number_.fractional /= 10U;

      }
      // ... and even the decimal point if there are no
      // non-zero fractional part digits (see below)
    }

    if (number_.fractional > 0 || !(flags & FLAGS_ADAPT_EXP) || (flags & FLAGS_HASH) ) {
      while (len < PRINTF_DECIMAL_BUFFER_SIZE) {
        --count;
        buf[len++] = (char)('0' + number_.fractional % 10U);
        if (!(number_.fractional /= 10U)) {
          break;
        }
      }
      // add extra 0s
      while ((len < PRINTF_DECIMAL_BUFFER_SIZE) && (count > 0U)) {
        buf[len++] = '0';
        --count;
      }
      if (len < PRINTF_DECIMAL_BUFFER_SIZE) {
        buf[len++] = '.';
      }
    }
  }
  else {
    if ((flags & FLAGS_HASH) && (len < PRINTF_DECIMAL_BUFFER_SIZE)) {
      buf[len++] = '.';
    }
  }

  // Write the integer part of the number (it comes after the fractional
  // since the character order is reversed)
  while (len < PRINTF_DECIMAL_BUFFER_SIZE) {
    buf[len++] = (char)('0' + (number_.integral % 10));
    if (!(number_.integral /= 10)) {
      break;
    }
  }

  // pad leading zeros
  if (!(flags & FLAGS_LEFT) && (flags & FLAGS_ZEROPAD)) {
    if (width && (number_.is_negative || (flags & (FLAGS_PLUS | FLAGS_SPACE)))) {
      width--;
    }
    while ((len < width) && (len < PRINTF_DECIMAL_BUFFER_SIZE)) {
      buf[len++] = '0';
    }
  }

  if (len < PRINTF_DECIMAL_BUFFER_SIZE) {
    if (number_.is_negative) {
      buf[len++] = '-';
    }
    else if (flags & FLAGS_PLUS) {
      buf[len++] = '+';  // ignore the space if the '+' exists
    }
    else if (flags & FLAGS_SPACE) {
      buf[len++] = ' ';
    }
  }

  out_rev_(output, buf, len, width, flags);
}

// internal ftoa for fixed decimal floating point
static void print_decimal_number(struct printf_output_gadget* output,
                          double number,
                          printf_size_t precision,
                          printf_size_t width,
                          printf_flags_t flags,
                          char* buf,
                          printf_size_t len) {
  struct double_components value_ = get_components(number, precision);
  print_broken_up_decimal(value_, output, precision, width, flags, buf, len);
}

/*******************************************************************************
 * Exponential Support
 ******************************************************************************/
#if PRINTF_SUPPORT_EXPONENTIAL_SPECIFIERS
struct scaling_factor {
  double raw_factor;
  bool_t multiply; // if true, need to multiply by raw_factor; otherwise need to divide by it
};

static double apply_scaling(double num, struct scaling_factor normalization)
{
  return normalization.multiply ? num * normalization.raw_factor : num / normalization.raw_factor;
}

static double unapply_scaling(double normalized, struct scaling_factor normalization)
{
  return normalization.multiply ? normalized / normalization.raw_factor : normalized * normalization.raw_factor;
}

static struct scaling_factor update_normalization(struct scaling_factor sf, double extra_multiplicative_factor)
{
  struct scaling_factor result;
  if (sf.multiply) {
    result.multiply = TRUE;
    result.raw_factor = sf.raw_factor * extra_multiplicative_factor;
  }
  else {
    int factor_exp2 = stdio_exp2(get_bit_access(sf.raw_factor));
    int extra_factor_exp2 = stdio_exp2(get_bit_access(extra_multiplicative_factor));

    // Divide the larger-exponent raw raw_factor by the smaller
    if (PRINTF_ABS(factor_exp2) > PRINTF_ABS(extra_factor_exp2)) {
      result.multiply = FALSE;
      result.raw_factor = sf.raw_factor / extra_multiplicative_factor;
    }
    else {
      result.multiply = TRUE;
      result.raw_factor = extra_multiplicative_factor / sf.raw_factor;
    }
  }
  return result;
}

static struct double_components get_normalized_components(bool_t negative, printf_size_t precision, double non_normalized, struct scaling_factor normalization, int floored_exp10)
{
  struct double_components components;
  components.is_negative = negative;
  double scaled = apply_scaling(non_normalized, normalization);

  bool_t close_to_representation_extremum = ( (-floored_exp10 + (int) precision) >= DBL_MAX_10_EXP - 1 );
  if (close_to_representation_extremum) {
    // We can't have a normalization factor which also accounts for the precision, i.e. moves
    // some decimal digits into the mantissa, since it's unrepresentable, or nearly unrepresentable.
    // So, we'll give up early on getting extra precision...
    return get_components(negative ? -scaled : scaled, precision);
  }
  components.integral = (int_fast64_t) scaled;
  double remainder = non_normalized - unapply_scaling((double) components.integral, normalization);
  double prec_power_of_10 = powers_of_10[precision];
  struct scaling_factor account_for_precision = update_normalization(normalization, prec_power_of_10);
  double scaled_remainder = apply_scaling(remainder, account_for_precision);
  double rounding_threshold = 0.5;

  components.fractional = (int_fast64_t) scaled_remainder; // when precision == 0, the assigned value should be 0
  scaled_remainder -= (double) components.fractional; //when precision == 0, this will not change scaled_remainder

  components.fractional += (scaled_remainder >= rounding_threshold);
  if (scaled_remainder == rounding_threshold) {
    // banker's rounding: Round towards the even number (making the mean error 0)
    components.fractional &= ~((int_fast64_t) 0x1);
  }
  // handle rollover, e.g. the case of 0.99 with precision 1 becoming (0,100),
  // and must then be corrected into (1, 0).
  // Note: for precision = 0, this will "translate" the rounding effect from
  // the fractional part to the integral part where it should actually be
  // felt (as prec_power_of_10 is 1)
  if ((double) components.fractional >= prec_power_of_10) {
    components.fractional = 0;
    ++components.integral;
  }
  return components;
}


static void print_exponential_number(
struct printf_output_gadget* output, double number, printf_size_t precision, printf_size_t width, printf_flags_t flags, char* buf, printf_size_t len)
{
  const bool_t negative = stdio_sign_bit(number);
  // This number will decrease gradually (by factors of 10) as we "extract" the exponent out of it
  double abs_number =  negative ? -number : number;

  int floored_exp10;
  bool_t abs_exp10_covered_by_powers_table;
  struct scaling_factor normalization;


  // Determine the decimal exponent
  if (abs_number == 0.0) {
    // TODO: This is a special-case for 0.0 (and -0.0); but proper handling is required for denormals more generally.
    floored_exp10 = 0; // ... and no need to set a normalization factor or check the powers table
  }
  else  {
    double exp10 = stdio_log10(abs_number);
    floored_exp10 = stdio_floor(exp10);
    double p10 = stdio_pow10(floored_exp10);
    // correct for rounding errors
    if (abs_number < p10) {
      floored_exp10--;
      p10 /= 10;
    }
    abs_exp10_covered_by_powers_table = PRINTF_ABS(floored_exp10) < PRINTF_MAX_PRECOMPUTED_POWER_OF_10;
    normalization.raw_factor = abs_exp10_covered_by_powers_table ? powers_of_10[PRINTF_ABS(floored_exp10)] : p10;
  }

  // We now begin accounting for the widths of the two parts of our printed field:
  // the decimal part after decimal exponent extraction, and the base-10 exponent part.
  // For both of these, the value of 0 has a special meaning, but not the same one:
  // a 0 exponent-part width means "don't print the exponent"; a 0 decimal-part width
  // means "use as many characters as necessary".

  bool_t fall_back_to_decimal_only_mode = FALSE;
  if (flags & FLAGS_ADAPT_EXP) {
    int required_significant_digits = (precision == 0) ? 1 : (int) precision;
    // Should we want to fall-back to "%f" mode, and only print the decimal part?
    fall_back_to_decimal_only_mode = (floored_exp10 >= -4 && floored_exp10 < required_significant_digits);
    // Now, let's adjust the precision
    // This also decided how we adjust the precision value - as in "%g" mode,
    // "precision" is the number of _significant digits_, and this is when we "translate"
    // the precision value to an actual number of decimal digits.
    int precision_ = fall_back_to_decimal_only_mode ?
                     (int) precision - 1 - floored_exp10 :
        (int) precision - 1; // the presence of the exponent ensures only one significant digit comes before the decimal point
    precision = (precision_ > 0 ? (unsigned) precision_ : 0U);
    flags |= FLAGS_PRECISION;   // make sure print_broken_up_decimal respects our choice above
  }

  normalization.multiply = (floored_exp10 < 0 && abs_exp10_covered_by_powers_table);
  bool_t should_skip_normalization = (fall_back_to_decimal_only_mode || floored_exp10 == 0);
  struct double_components decimal_part_components =
    should_skip_normalization ?
    get_components(negative ? -abs_number : abs_number, precision) :
    get_normalized_components(negative, precision, abs_number, normalization, floored_exp10);

  // Account for roll-over, e.g. rounding from 9.99 to 100.0 - which effects
  // the exponent and may require additional tweaking of the parts
  if (fall_back_to_decimal_only_mode) {
    if ((flags & FLAGS_ADAPT_EXP) && floored_exp10 >= -1 && decimal_part_components.integral == powers_of_10[floored_exp10 + 1]) {
      floored_exp10++; // Not strictly necessary, since floored_exp10 is no longer really used
      precision--;
      // ... and it should already be the case that decimal_part_components.fractional == 0
    }
    // TODO: What about rollover strictly within the fractional part?
  }
  else {
    if (decimal_part_components.integral >= 10) {
      floored_exp10++;
      decimal_part_components.integral = 1;
      decimal_part_components.fractional = 0;
    }
  }

  // the floored_exp10 format is "E%+03d" and largest possible floored_exp10 value for a 64-bit double
  // is "307" (for 2^1023), so we set aside 4-5 characters overall
  printf_size_t exp10_part_width = fall_back_to_decimal_only_mode ? 0U : (PRINTF_ABS(floored_exp10) < 100) ? 4U : 5U;

  printf_size_t decimal_part_width =
    ((flags & FLAGS_LEFT) && exp10_part_width) ?
      // We're padding on the right, so the width constraint is the exponent part's
      // problem, not the decimal part's, so we'll use as many characters as we need:
      0U :
      // We're padding on the left; so the width constraint is the decimal part's
      // problem. Well, can both the decimal part and the exponent part fit within our overall width?
      ((width > exp10_part_width) ?
        // Yes, so we limit our decimal part's width.
        // (Note this is trivially valid even if we've fallen back to "%f" mode)
        width - exp10_part_width :
        // No; we just give up on any restriction on the decimal part and use as many
        // characters as we need
        0U);

  const printf_size_t printed_exponential_start_pos = output->pos;
  print_broken_up_decimal(decimal_part_components, output, precision, decimal_part_width, flags, buf, len);

  if (! fall_back_to_decimal_only_mode) {
    putchar_via_gadget(output, (flags & FLAGS_UPPERCASE) ? 'E' : 'e');
    print_integer(output,
                  ABS_FOR_PRINTING(floored_exp10),
                  floored_exp10 < 0, 10, 0, exp10_part_width - 1,
                FLAGS_ZEROPAD | FLAGS_PLUS);
    if (flags & FLAGS_LEFT) {
      // We need to right-pad with spaces to meet the width requirement
      while (output->pos - printed_exponential_start_pos < width) {
        putchar_via_gadget(output, ' ');
      }
    }
  }
}
#endif  // PRINTF_SUPPORT_EXPONENTIAL_SPECIFIERS

/*******************************************************************************
 * API Functions
 ******************************************************************************/
void print_floating_point(
struct printf_output_gadget* output, double value, printf_size_t precision, printf_size_t width, printf_flags_t flags, bool_t prefer_exponential)
{
  char buf[PRINTF_DECIMAL_BUFFER_SIZE];
  printf_size_t len = 0U;

  // test for special values
  if (value != value) {
    out_rev_(output, "nan", 3, width, flags);
    return;
  }
  if (value < -DBL_MAX) {
    out_rev_(output, "fni-", 4, width, flags);
    return;
  }
  if (value > DBL_MAX) {
    out_rev_(output, (flags & FLAGS_PLUS) ? "fni+" : "fni", (flags & FLAGS_PLUS) ? 4U : 3U, width, flags);
    return;
  }

  if (!prefer_exponential &&
      ((value > PRINTF_FLOAT_NOTATION_THRESHOLD) || (value < -PRINTF_FLOAT_NOTATION_THRESHOLD))) {
    // The required behavior of standard printf is to print _every_ integral-part digit -- which could mean
    // printing hundreds of characters, overflowing any fixed internal buffer and necessitating a more complicated
    // implementation.
#if PRINTF_SUPPORT_EXPONENTIAL_SPECIFIERS
    print_exponential_number(output, value, precision, width, flags, buf, len);
#endif
    return;
  }

  // set default precision, if not set explicitly
  if (!(flags & FLAGS_PRECISION)) {
    precision = PRINTF_DEFAULT_FLOAT_PRECISION;
  }

  // limit precision so that our integer holding the fractional part does not overflow
  while ((len < PRINTF_DECIMAL_BUFFER_SIZE) && (precision > PRINTF_MAX_SUPPORTED_PRECISION)) {
    buf[len++] = '0'; // This respects the precision in terms of result length only
    precision--;
  }

#if PRINTF_SUPPORT_EXPONENTIAL_SPECIFIERS
  if (prefer_exponential)
    print_exponential_number(output, value, precision, width, flags, buf, len);
  else
#endif
    print_decimal_number(output, value, precision, width, flags, buf, len);
}
