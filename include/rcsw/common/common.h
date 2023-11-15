/**
 * \file common.h
 * \ingroup common
 * \brief Definitions, etc. common to all of RCSW.
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/al/types.h"
#include "rcsw/common/compilers.h"

/*******************************************************************************
 * Constants
 ******************************************************************************/
/** 1E9 */
#define ONEE9 1000000000

/** 1E6 */
#define ONEE6 1000000

/** 1E3 */
#define ONEE3 1000

/**
 * \brief For comparison with 0.0 for floats which gives reasonable results.
 */
#define RCSW_FLOAT_EPSILON 0.00000001

/**
 * \brief For comparison with 0.0 for doubles which gives reasonable results.
 */
#define RCSW_DOUBLE_EPSILON 0.00000000001

/*******************************************************************************
 * String Macros
 ******************************************************************************/
/* \cond INTERNAL */
#define RCSW_XSTR_(x) #x /* don't use this one */
#define RCSW_JOIN_(x, y) x##y /* don't use this one */
/* \endcond */

/**
 * \def RCSW_XSTR(X) Stringification.
 *
 * Use when you need to stringify the result of a macro expansion.
 */
#define RCSW_XSTR(X) RCSW_XSTR_(X)

/**
 * \def RCSW_JOIN(x, y) Token pasting.
 *
 * Use when you need to join two tokens ``x`` and ``y`` together.
 */
#define RCSW_JOIN(x, y) RCSW_JOIN_(x, y)

/**
 * \def RCSW_JOIN(x, y, z) Token pasting (3 tokens).
 */
#define RCSW_JOIN3(x, y, z) RCSW_JOIN(RCSW_JOIN(x, y), z)

/**
 * \def RCSW_UNIQUE_ID(prefix)
 *
 * Generate a translation unit unique identifier using gcc's __COUNTER__.
 */
#define RCSW_UNIQUE_ID(prefix) RCSW_JOIN(RCSW_JOIN(__UNIQUE_ID_,        \
                                                   prefix),             \
                                         __COUNTER__)

/*******************************************************************************
 * Comparison Macros
 ******************************************************************************/
/**
 * \def RCSW_MIN_(t1, t2, min1, min2, a, b)
 *
 * Gets the minimum of (\a a, \a b) while also performing a type comparison. If
 * the arguments do not have the same type, a compiler warning will be
 * issued. You have to EXPLICITLY cast, which is a good thing. Don't want to get
 * weird behavior when taking the max/min of different types.The type checking
 * will be compiled away at high optimization levels.
 */
#define RCSW_MIN_(t1, t2, min1, min2, a, b) ({  \
      t1 min1 = (a);                            \
      t2 min2 = (b);                            \
      (void) (&(min1) == &(min2));              \
      (min1) < (min2) ? (min1) : (min2);        \
    })

/**
 * \def RCSW_MAX_(t1, t2, min1, min2, a, b)
 *
 * Gets the maximum of (\a a, \a b) while also performing a type comparison. If
 * the arguments do not have the same type, a compiler warning will be
 * issued. You have to EXPLICITLY cast, which is a good thing. Don't want to get
 * weird behavior when taking the max/min of different types.The type checking
 * will be compiled away at high optimization levels.
 */
#define RCSW_MAX_(t1, t2, max1, max2, a, b) ({  \
      t1 max1 = (a);                            \
      t2 max2 = (b);                            \
      (void) (&(max1) == &(max2));              \
      (max1) > (max2) ? (max1) : (max2);        \
    })

/**
 * \def RCSW_MIN(a, b)
 *
 * Returns a type-same minimum of its arguments (compiler warnings for unsafe
 * comparisons of different types).
 */
#define RCSW_MIN(a, b)                                          \
  RCSW_MIN_(typeof(a), typeof(b),                               \
            RCSW_UNIQUE_ID(min1_), RCSW_UNIQUE_ID(min2_),       \
            a, b)

/**
 * \def RCSW_MIN3(a, b, c)
 *
 * Returns a type-same minimum of its arguments (compiler warnings for unsafe
 * comparisons of different types).
 */
#define RCSW_MIN3(a, b, c) RCSW_MIN((typeof(a))RCSW_MIN(a, b), c)

/**
 * \def RCSW_MAX(a, b)
 *
 * Returns a type-same maximum of its arguments (compiler warnings for unsafe
 * comparisons of different types).
 */
#define RCSW_MAX(a, b)                                          \
  RCSW_MAX_(typeof(a), typeof(b),                               \
            RCSW_UNIQUE_ID(max1_), RCSW_UNIQUE_ID(max2_),       \
            a, b)


/**
 * \def RCSW_MAX3(a, b, c)
 *
 * Returns a type-same maximum of its arguments (compiler warnings for unsafe
 * comparisons of different types).
 */
#define RCSW_MAX3(a, b, c) RCSW_MAX((typeof(a))RCSW_MAX(a, b), c)

/**
 * \def RCSW_IS_ODD(n)
 *
 * Readability macro for determining if something is odd.
 *
 * You can obviously do this without a macro, but this is (1) more self
 * documenting, and (2) less error prone.
 */
#define RCSW_IS_ODD(n) ((n)&1U)

/**
 * \def RCSW_IS_EVEN(n)
 *
 * Readability macro for determining if something is even.
 *
 * You can obviously do this without a macro, but this is (1) more self
 * documenting, and (2) less error prone.
 */
#define RCSW_IS_EVEN(n) (!RCSW_IS_ODD((n)))

/**
 * \def RCSW_IS_BETWEENC(n, low, high)
 *
 * Readability macro for determining if something is between an upper and lower
 * bound (closed=inclusive).
 *
 * You can obviously do this without a macro, but this is (1) more self
 * documenting, and (2) less error prone.
 */
#define RCSW_IS_BETWEENC(n, low, high) ((n) >= (low) && (n) <= (high))

/**
 * \def RCSW_IS_BETWEENO(n, low, high)
 *
 * Readability macro for determining if something is between an upper and lower
 * bound (open=exclusive).
 *
 * You can obviously do this without a macro, but this is (1) more self
 * documenting, and (2) less error prone.
 */
#define RCSW_IS_BETWEENO(n, low, high) ((n) > (low) && (n) < (high))

/**
 * \def RCSW_IS_BETWEENHO(n, low, high)
 *
 * Readability macro for determining if something is between an upper and lower
 * bound (half open=includes lb but not ub).
 *
 * You can obviously do this without a macro, but this is (1) more self
 * documenting, and (2) less error prone.
 */
#define RCSW_IS_BETWEENHO(n, low, high) ((n) >= (low) && (n) < (high))

/*******************************************************************************
 * Misc. Macros
 ******************************************************************************/
/**
 * \def RCSW_LIKELY(x)
 *
 * Inform the compiler that a conditional branch is almost certain to be taken,
 * for optimization purposes.
 */
#define RCSW_LIKELY(x) __builtin_expect((x), 1)

/**
 * \def RCSW_UNLIKELY(x)
 *
 * Inform the compiler that a conditional branch is almost certain not to be
 * taken, for optimization purposes.
 */
#define RCSW_UNLIKELY(x) __builtin_expect((x), 0)

/**
 * \def RCSW_CLAMP(val, min, max)
 *
 * Clamp a value between two setpoints.
 */
#define RCSW_CLAMP(val, min, max) ((val) < (min) ? (min) : ((val) > (max) ? (max): (val)))

/**
 * \def RCSW_ARRAY_ELTS(arr)
 *
 * Get the size of an array (NOT a pointer to an array) in units (not bytes).
 *
 * \note If you try this on arrays allocated dynamically (VLAs, heap,...) it
 * will not work.
 */
#define RCSW_ARRAY_ELTS(arr) (sizeof(arr) / sizeof((arr)[0]))

/**
 * \def RCSW_CONTAINER_OF(ptr, type, member)
 *
 * Mostly stolen from the linux kernel. VERY cool macro for getting a pointer to
 * a struct given a pointer to a member within the struct.
 */
#define RCSW_CONTAINER_OF(ptr, type, member) ({                 \
      const typeof(((type *)0)->member) *__mptr = (ptr);        \
      (type *)( (char *)__mptr - offsetof(type, member) );})

/**
 * \def RCSW_FIELD_SIZEOF(t, f)
 * Get the size of a field in a struct \a t named \a f.
 */
#define RCSW_FIELD_SIZEOF(t, f) (sizeof(((t *)0)->f))

/**
 * \def RCSW_CHECK(cond) Check a condition in a function.
 *
 * If condition is not true, go to the error/bailout section for function (you
 * must have a label called \c error in your function).
 */
#define RCSW_CHECK(cond)                        \
  if (RCSW_UNLIKELY(!(cond))) {                 \
    goto error;                                 \
  }

/**
 * \def RCSW_CHECK_PTR(ptr) Check a pointer \a ptr in a function.
 *
 * If \a ptr is NULL, go to the error/bailout section for function (you
 * must have a label called \c error in your function).
 */
#define RCSW_CHECK_PTR(ptr) RCSW_CHECK(NULL != (ptr))

/**
 * \def RCSW_CHECK_FD(fd) Check a file descriptor \a fd in a function.
 *
 * If the descriptor \a fd is invalid (i.e. < 0), go to the error/bailout
 * section for function (you must have a label called \c error in your
 * function).
 */
#define RCSW_CHECK_FD(fd) RCSW_CHECK((fd) >= 0)

/*******************************************************************************
 * Tricky Variadac Macro Manipulation
 ******************************************************************************/
/* \cond INTERNAL */
#define RCSW_VAR_NARG_(...) RCSW_VAR_ARG_N(__VA_ARGS__)
#define RCSW_VAR_ARG_N(                                                 \
    _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, \
    _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, \
    _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, \
    _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, _61, \
    _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, \
    _77, _78, _79, _80, _81, _82, _83, _84, _85, _86, _87, _88, _89, _90, _91, \
    _92, _93, _94, _95, _96, _97, _98, _99, _100, N, ...)               \

#define RCSW_VAR_RSEQ_N()                                               \
  100, 99, 98, 97, 96, 95, 94, 93, 92, 91, 90, 89, 88, 87, 86, 85, 84, 83, \
    82, 81, 80, 79, 78, 77, 76, 75, 74, 73, 72, 71, 70, 69, 68, 67, 66, \
    65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, \
    48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, \
    31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, \
    14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0

#define RCSW_XFE1_1_NOTAIL(WHAT, X) X
#define RCSW_XFE1_2_NOTAIL(WHAT, X, ...) WHAT(X) RCSW_XFE1_1_NOTAIL(WHAT, __VA_ARGS__)
#define RCSW_XFE1_3_NOTAIL(WHAT, X, ...) WHAT(X) RCSW_XFE1_2_NOTAIL(WHAT, __VA_ARGS__)
#define RCSW_XFE1_4_NOTAIL(WHAT, X, ...) WHAT(X) RCSW_XFE1_3_NOTAIL(WHAT, __VA_ARGS__)
#define RCSW_XFE1_5_NOTAIL(WHAT, X, ...) WHAT(X) RCSW_XFE1_4_NOTAIL(WHAT, __VA_ARGS__)
#define RCSW_XFE1_6_NOTAIL(WHAT, X, ...) WHAT(X) RCSW_XFE1_5_NOTAIL(WHAT, __VA_ARGS__)
#define RCSW_XFE1_7_NOTAIL(WHAT, X, ...) WHAT(X) RCSW_XFE1_6_NOTAIL(WHAT, __VA_ARGS__)
#define RCSW_XFE1_8_NOTAIL(WHAT, X, ...) WHAT(X) RCSW_XFE1_7_NOTAIL(WHAT, __VA_ARGS__)
#define RCSW_XFE1_9_NOTAIL(WHAT, X, ...) WHAT(X) RCSW_XFE1_8_NOTAIL(WHAT, __VA_ARGS__)
#define RCSW_XFE1_10_NOTAIL(WHAT, X, ...) WHAT(X) RCSW_XFE1_9_NOTAIL(WHAT, __VA_ARGS__)
#define RCSW_XFE1_11_NOTAIL(WHAT, X, ...) WHAT(X) RCSW_XFE1_10_NOTAIL(WHAT, __VA_ARGS__)
#define RCSW_XFE1_12_NOTAIL(WHAT, X, ...) WHAT(X) RCSW_XFE1_11_NOTAIL(WHAT, __VA_ARGS__)
#define RCSW_XFE1_13_NOTAIL(WHAT, X, ...) WHAT(X) RCSW_XFE1_12_NOTAIL(WHAT, __VA_ARGS__)
#define RCSW_XFE1_14_NOTAIL(WHAT, X, ...) WHAT(X) RCSW_XFE1_13_NOTAIL(WHAT, __VA_ARGS__)
#define RCSW_XFE1_15_NOTAIL(WHAT, X, ...) WHAT(X) RCSW_XFE1_14_NOTAIL(WHAT, __VA_ARGS__)
#define RCSW_XFE1_16_NOTAIL(WHAT, X, ...) WHAT(X) RCSW_XFE1_15_NOTAIL(WHAT, __VA_ARGS__)
#define RCSW_XFE1_17_NOTAIL(WHAT, X, ...) WHAT(X) RCSW_XFE1_16_NOTAIL(WHAT, __VA_ARGS__)
#define RCSW_XFE1_18_NOTAIL(WHAT, X, ...) WHAT(X) RCSW_XFE1_17_NOTAIL(WHAT, __VA_ARGS__)
#define RCSW_XFE1_19_NOTAIL(WHAT, X, ...) WHAT(X) RCSW_XFE1_18_NOTAIL(WHAT, __VA_ARGS__)
#define RCSW_XFE1_20_NOTAIL(WHAT, X, ...) WHAT(X) RCSW_XFE1_19_NOTAIL(WHAT, __VA_ARGS__)
#define RCSW_XFE1_21_NOTAIL(WHAT, X, ...) WHAT(X) RCSW_XFE1_20_NOTAIL(WHAT, __VA_ARGS__)
#define RCSW_XFE1_22_NOTAIL(WHAT, X, ...) WHAT(X) RCSW_XFE1_21_NOTAIL(WHAT, __VA_ARGS__)
#define RCSW_XFE1_23_NOTAIL(WHAT, X, ...) WHAT(X) RCSW_XFE1_22_NOTAIL(WHAT, __VA_ARGS__)
#define RCSW_XFE1_24_NOTAIL(WHAT, X, ...) WHAT(X) RCSW_XFE1_23_NOTAIL(WHAT, __VA_ARGS__)
#define RCSW_XFE1_25_NOTAIL(WHAT, X, ...) WHAT(X) RCSW_XFE1_24_NOTAIL(WHAT, __VA_ARGS__)
#define RCSW_XFE1_26_NOTAIL(WHAT, X, ...) WHAT(X) RCSW_XFE1_25_NOTAIL(WHAT, __VA_ARGS__)
#define RCSW_XFE1_27_NOTAIL(WHAT, X, ...) WHAT(X) RCSW_XFE1_26_NOTAIL(WHAT, __VA_ARGS__)
#define RCSW_XFE1_28_NOTAIL(WHAT, X, ...) WHAT(X) RCSW_XFE1_27_NOTAIL(WHAT, __VA_ARGS__)
#define RCSW_XFE1_29_NOTAIL(WHAT, X, ...) WHAT(X) RCSW_XFE1_28_NOTAIL(WHAT, __VA_ARGS__)
#define RCSW_XFE1_30_NOTAIL(WHAT, X, ...) WHAT(X) RCSW_XFE1_29_NOTAIL(WHAT, __VA_ARGS__)
#define RCSW_XFE1_31_NOTAIL(WHAT, X, ...) WHAT(X) RCSW_XFE1_30_NOTAIL(WHAT, __VA_ARGS__)
#define RCSW_XFE1_32_NOTAIL(WHAT, X, ...) WHAT(X) RCSW_XFE1_31_NOTAIL(WHAT, __VA_ARGS__)
#define RCSW_XFE1_33_NOTAIL(WHAT, X, ...) WHAT(X) RCSW_XFE1_32_NOTAIL(WHAT, __VA_ARGS__)
#define RCSW_XFE1_34_NOTAIL(WHAT, X, ...) WHAT(X) RCSW_XFE1_33_NOTAIL(WHAT, __VA_ARGS__)
#define RCSW_XFE1_35_NOTAIL(WHAT, X, ...) WHAT(X) RCSW_XFE1_34_NOTAIL(WHAT, __VA_ARGS__)
#define RCSW_XFE1_36_NOTAIL(WHAT, X, ...) WHAT(X) RCSW_XFE1_35_NOTAIL(WHAT, __VA_ARGS__)
#define RCSW_XFE1_37_NOTAIL(WHAT, X, ...) WHAT(X) RCSW_XFE1_36_NOTAIL(WHAT, __VA_ARGS__)
#define RCSW_XFE1_38_NOTAIL(WHAT, X, ...) WHAT(X) RCSW_XFE1_37_NOTAIL(WHAT, __VA_ARGS__)
#define RCSW_XFE1_39_NOTAIL(WHAT, X, ...) WHAT(X) RCSW_XFE1_38_NOTAIL(WHAT, __VA_ARGS__)
#define RCSW_XFE1_40_NOTAIL(WHAT, X, ...) WHAT(X) RCSW_XFE1_39_NOTAIL(WHAT, __VA_ARGS__)
#define RCSW_XFE1_41_NOTAIL(WHAT, X, ...) WHAT(X) RCSW_XFE1_40_NOTAIL(WHAT, __VA_ARGS__)
#define RCSW_XFE1_42_NOTAIL(WHAT, X, ...) WHAT(X) RCSW_XFE1_41_NOTAIL(WHAT, __VA_ARGS__)
#define RCSW_XFE1_43_NOTAIL(WHAT, X, ...) WHAT(X) RCSW_XFE1_42_NOTAIL(WHAT, __VA_ARGS__)
#define RCSW_XFE1_44_NOTAIL(WHAT, X, ...) WHAT(X) RCSW_XFE1_43_NOTAIL(WHAT, __VA_ARGS__)
#define RCSW_XFE1_45_NOTAIL(WHAT, X, ...) WHAT(X) RCSW_XFE1_44_NOTAIL(WHAT, __VA_ARGS__)
#define RCSW_XFE1_46_NOTAIL(WHAT, X, ...) WHAT(X) RCSW_XFE1_45_NOTAIL(WHAT, __VA_ARGS__)
#define RCSW_XFE1_47_NOTAIL(WHAT, X, ...) WHAT(X) RCSW_XFE1_46_NOTAIL(WHAT, __VA_ARGS__)
#define RCSW_XFE1_48_NOTAIL(WHAT, X, ...) WHAT(X) RCSW_XFE1_47_NOTAIL(WHAT, __VA_ARGS__)
#define RCSW_XFE1_49_NOTAIL(WHAT, X, ...) WHAT(X) RCSW_XFE1_48_NOTAIL(WHAT, __VA_ARGS__)
#define RCSW_XFE1_50_NOTAIL(WHAT, X, ...) WHAT(X) RCSW_XFE1_49_NOTAIL(WHAT, __VA_ARGS__)

#define RCSW_XFE1_1(WHAT, X) WHAT(X)
#define RCSW_XFE1_2(WHAT, X, ...) WHAT(X) RCSW_XFE1_1(WHAT, __VA_ARGS__)
#define RCSW_XFE1_3(WHAT, X, ...) WHAT(X) RCSW_XFE1_2(WHAT, __VA_ARGS__)
#define RCSW_XFE1_4(WHAT, X, ...) WHAT(X) RCSW_XFE1_3(WHAT, __VA_ARGS__)
#define RCSW_XFE1_5(WHAT, X, ...) WHAT(X) RCSW_XFE1_4(WHAT, __VA_ARGS__)
#define RCSW_XFE1_6(WHAT, X, ...) WHAT(X) RCSW_XFE1_5(WHAT, __VA_ARGS__)
#define RCSW_XFE1_7(WHAT, X, ...) WHAT(X) RCSW_XFE1_6(WHAT, __VA_ARGS__)
#define RCSW_XFE1_8(WHAT, X, ...) WHAT(X) RCSW_XFE1_7(WHAT, __VA_ARGS__)
#define RCSW_XFE1_9(WHAT, X, ...) WHAT(X) RCSW_XFE1_8(WHAT, __VA_ARGS__)
#define RCSW_XFE1_10(WHAT, X, ...) WHAT(X) RCSW_XFE1_9(WHAT, __VA_ARGS__)
#define RCSW_XFE1_11(WHAT, X, ...) WHAT(X) RCSW_XFE1_10(WHAT, __VA_ARGS__)
#define RCSW_XFE1_12(WHAT, X, ...) WHAT(X) RCSW_XFE1_11(WHAT, __VA_ARGS__)
#define RCSW_XFE1_13(WHAT, X, ...) WHAT(X) RCSW_XFE1_12(WHAT, __VA_ARGS__)
#define RCSW_XFE1_14(WHAT, X, ...) WHAT(X) RCSW_XFE1_13(WHAT, __VA_ARGS__)
#define RCSW_XFE1_15(WHAT, X, ...) WHAT(X) RCSW_XFE1_14(WHAT, __VA_ARGS__)
#define RCSW_XFE1_16(WHAT, X, ...) WHAT(X) RCSW_XFE1_15(WHAT, __VA_ARGS__)
#define RCSW_XFE1_17(WHAT, X, ...) WHAT(X) RCSW_XFE1_16(WHAT, __VA_ARGS__)
#define RCSW_XFE1_18(WHAT, X, ...) WHAT(X) RCSW_XFE1_17(WHAT, __VA_ARGS__)
#define RCSW_XFE1_19(WHAT, X, ...) WHAT(X) RCSW_XFE1_18(WHAT, __VA_ARGS__)
#define RCSW_XFE1_20(WHAT, X, ...) WHAT(X) RCSW_XFE1_19(WHAT, __VA_ARGS__)
#define RCSW_XFE1_21(WHAT, X, ...) WHAT(X) RCSW_XFE1_20(WHAT, __VA_ARGS__)
#define RCSW_XFE1_22(WHAT, X, ...) WHAT(X) RCSW_XFE1_21(WHAT, __VA_ARGS__)
#define RCSW_XFE1_23(WHAT, X, ...) WHAT(X) RCSW_XFE1_22(WHAT, __VA_ARGS__)
#define RCSW_XFE1_24(WHAT, X, ...) WHAT(X) RCSW_XFE1_23(WHAT, __VA_ARGS__)
#define RCSW_XFE1_25(WHAT, X, ...) WHAT(X) RCSW_XFE1_24(WHAT, __VA_ARGS__)
#define RCSW_XFE1_26(WHAT, X, ...) WHAT(X) RCSW_XFE1_25(WHAT, __VA_ARGS__)
#define RCSW_XFE1_27(WHAT, X, ...) WHAT(X) RCSW_XFE1_26(WHAT, __VA_ARGS__)
#define RCSW_XFE1_28(WHAT, X, ...) WHAT(X) RCSW_XFE1_27(WHAT, __VA_ARGS__)
#define RCSW_XFE1_29(WHAT, X, ...) WHAT(X) RCSW_XFE1_28(WHAT, __VA_ARGS__)
#define RCSW_XFE1_30(WHAT, X, ...) WHAT(X) RCSW_XFE1_29(WHAT, __VA_ARGS__)
#define RCSW_XFE1_31(WHAT, X, ...) WHAT(X) RCSW_XFE1_30(WHAT, __VA_ARGS__)
#define RCSW_XFE1_32(WHAT, X, ...) WHAT(X) RCSW_XFE1_31(WHAT, __VA_ARGS__)
#define RCSW_XFE1_33(WHAT, X, ...) WHAT(X) RCSW_XFE1_32(WHAT, __VA_ARGS__)
#define RCSW_XFE1_34(WHAT, X, ...) WHAT(X) RCSW_XFE1_33(WHAT, __VA_ARGS__)
#define RCSW_XFE1_35(WHAT, X, ...) WHAT(X) RCSW_XFE1_34(WHAT, __VA_ARGS__)
#define RCSW_XFE1_36(WHAT, X, ...) WHAT(X) RCSW_XFE1_35(WHAT, __VA_ARGS__)
#define RCSW_XFE1_37(WHAT, X, ...) WHAT(X) RCSW_XFE1_36(WHAT, __VA_ARGS__)
#define RCSW_XFE1_38(WHAT, X, ...) WHAT(X) RCSW_XFE1_37(WHAT, __VA_ARGS__)
#define RCSW_XFE1_39(WHAT, X, ...) WHAT(X) RCSW_XFE1_38(WHAT, __VA_ARGS__)
#define RCSW_XFE1_40(WHAT, X, ...) WHAT(X) RCSW_XFE1_39(WHAT, __VA_ARGS__)
#define RCSW_XFE1_41(WHAT, X, ...) WHAT(X) RCSW_XFE1_40(WHAT, __VA_ARGS__)
#define RCSW_XFE1_42(WHAT, X, ...) WHAT(X) RCSW_XFE1_41(WHAT, __VA_ARGS__)
#define RCSW_XFE1_43(WHAT, X, ...) WHAT(X) RCSW_XFE1_42(WHAT, __VA_ARGS__)
#define RCSW_XFE1_44(WHAT, X, ...) WHAT(X) RCSW_XFE1_43(WHAT, __VA_ARGS__)
#define RCSW_XFE1_45(WHAT, X, ...) WHAT(X) RCSW_XFE1_44(WHAT, __VA_ARGS__)
#define RCSW_XFE1_46(WHAT, X, ...) WHAT(X) RCSW_XFE1_45(WHAT, __VA_ARGS__)
#define RCSW_XFE1_47(WHAT, X, ...) WHAT(X) RCSW_XFE1_46(WHAT, __VA_ARGS__)
#define RCSW_XFE1_48(WHAT, X, ...) WHAT(X) RCSW_XFE1_47(WHAT, __VA_ARGS__)
#define RCSW_XFE1_49(WHAT, X, ...) WHAT(X) RCSW_XFE1_48(WHAT, __VA_ARGS__)
#define RCSW_XFE1_50(WHAT, X, ...) WHAT(X) RCSW_XFE1_49(WHAT, __VA_ARGS__)

#define RCSW_XFE2_1(WHAT, v, X) WHAT(X, v)
#define RCSW_XFE2_2(WHAT, v, X, ...) WHAT(X, v) RCSW_XFE2_1(WHAT, v, __VA_ARGS__)
#define RCSW_XFE2_3(WHAT, v, X, ...) WHAT(X, v) RCSW_XFE2_2(WHAT, v, __VA_ARGS__)
#define RCSW_XFE2_4(WHAT, v, X, ...) WHAT(X, v) RCSW_XFE2_3(WHAT, v, __VA_ARGS__)
#define RCSW_XFE2_5(WHAT, v, X, ...) WHAT(X, v) RCSW_XFE2_4(WHAT, v, __VA_ARGS__)
#define RCSW_XFE2_6(WHAT, v, X, ...) WHAT(X, v) RCSW_XFE2_5(WHAT, v, __VA_ARGS__)
#define RCSW_XFE2_7(WHAT, v, X, ...) WHAT(X, v) RCSW_XFE2_6(WHAT, v, __VA_ARGS__)
#define RCSW_XFE2_8(WHAT, v, X, ...) WHAT(X, v) RCSW_XFE2_7(WHAT, v, __VA_ARGS__)
#define RCSW_XFE2_9(WHAT, v, X, ...) WHAT(X, v) RCSW_XFE2_8(WHAT, v, __VA_ARGS__)
#define RCSW_XFE2_10(WHAT, v, X, ...) WHAT(X, v) RCSW_XFE2_9(WHAT, v, __VA_ARGS__)
#define RCSW_XFE2_11(WHAT, v, X, ...) WHAT(X, v) RCSW_XFE2_10(WHAT, v, __VA_ARGS__)
#define RCSW_XFE2_12(WHAT, v, X, ...) WHAT(X, v) RCSW_XFE2_11(WHAT, v, __VA_ARGS__)
#define RCSW_XFE2_13(WHAT, v, X, ...) WHAT(X, v) RCSW_XFE2_12(WHAT, v, __VA_ARGS__)
#define RCSW_XFE2_14(WHAT, v, X, ...) WHAT(X, v) RCSW_XFE2_13(WHAT, v, __VA_ARGS__)
#define RCSW_XFE2_15(WHAT, v, X, ...) WHAT(X, v) RCSW_XFE2_14(WHAT, v, __VA_ARGS__)
#define RCSW_XFE2_16(WHAT, v, X, ...) WHAT(X, v) RCSW_XFE2_15(WHAT, v, __VA_ARGS__)
#define RCSW_XFE2_17(WHAT, v, X, ...) WHAT(X, v) RCSW_XFE2_16(WHAT, v, __VA_ARGS__)
#define RCSW_XFE2_18(WHAT, v, X, ...) WHAT(X, v) RCSW_XFE2_17(WHAT, v, __VA_ARGS__)
#define RCSW_XFE2_19(WHAT, v, X, ...) WHAT(X, v) RCSW_XFE2_18(WHAT, v, __VA_ARGS__)
#define RCSW_XFE2_20(WHAT, v, X, ...) WHAT(X, v) RCSW_XFE2_19(WHAT, v, __VA_ARGS__)
#define RCSW_XFE2_21(WHAT, v, X, ...) WHAT(X, v) RCSW_XFE2_20(WHAT, v, __VA_ARGS__)
#define RCSW_XFE2_22(WHAT, v, X, ...) WHAT(X, v) RCSW_XFE2_21(WHAT, v, __VA_ARGS__)
#define RCSW_XFE2_23(WHAT, v, X, ...) WHAT(X, v) RCSW_XFE2_22(WHAT, v, __VA_ARGS__)
#define RCSW_XFE2_24(WHAT, v, X, ...) WHAT(X, v) RCSW_XFE2_23(WHAT, v, __VA_ARGS__)
#define RCSW_XFE2_25(WHAT, v, X, ...) WHAT(X, v) RCSW_XFE2_24(WHAT, v, __VA_ARGS__)
#define RCSW_XFE2_26(WHAT, v, X, ...) WHAT(X, v) RCSW_XFE2_25(WHAT, v, __VA_ARGS__)
#define RCSW_XFE2_27(WHAT, v, X, ...) WHAT(X, v) RCSW_XFE2_26(WHAT, v, __VA_ARGS__)
#define RCSW_XFE2_28(WHAT, v, X, ...) WHAT(X, v) RCSW_XFE2_27(WHAT, v, __VA_ARGS__)
#define RCSW_XFE2_29(WHAT, v, X, ...) WHAT(X, v) RCSW_XFE2_28(WHAT, v, __VA_ARGS__)
#define RCSW_XFE2_30(WHAT, v, X, ...) WHAT(X, v) RCSW_XFE2_29(WHAT, v, __VA_ARGS__)
#define RCSW_XFE2_31(WHAT, v, X, ...) WHAT(X, v) RCSW_XFE2_30(WHAT, v, __VA_ARGS__)
#define RCSW_XFE2_32(WHAT, v, X, ...) WHAT(X, v) RCSW_XFE2_31(WHAT, v, __VA_ARGS__)
#define RCSW_XFE2_33(WHAT, v, X, ...) WHAT(X, v) RCSW_XFE2_32(WHAT, v, __VA_ARGS__)
#define RCSW_XFE2_34(WHAT, v, X, ...) WHAT(X, v) RCSW_XFE2_33(WHAT, v, __VA_ARGS__)
#define RCSW_XFE2_35(WHAT, v, X, ...) WHAT(X, v) RCSW_XFE2_34(WHAT, v, __VA_ARGS__)
#define RCSW_XFE2_36(WHAT, v, X, ...) WHAT(X, v) RCSW_XFE2_35(WHAT, v, __VA_ARGS__)
#define RCSW_XFE2_37(WHAT, v, X, ...) WHAT(X, v) RCSW_XFE2_36(WHAT, v, __VA_ARGS__)
#define RCSW_XFE2_38(WHAT, v, X, ...) WHAT(X, v) RCSW_XFE2_37(WHAT, v, __VA_ARGS__)
#define RCSW_XFE2_39(WHAT, v, X, ...) WHAT(X, v) RCSW_XFE2_38(WHAT, v, __VA_ARGS__)
#define RCSW_XFE2_40(WHAT, v, X, ...) WHAT(X, v) RCSW_XFE2_39(WHAT, v, __VA_ARGS__)
#define RCSW_XFE2_41(WHAT, v, X, ...) WHAT(X, v) RCSW_XFE2_40(WHAT, v, __VA_ARGS__)
#define RCSW_XFE2_42(WHAT, v, X, ...) WHAT(X, v) RCSW_XFE2_41(WHAT, v, __VA_ARGS__)
#define RCSW_XFE2_43(WHAT, v, X, ...) WHAT(X, v) RCSW_XFE2_42(WHAT, v, __VA_ARGS__)
#define RCSW_XFE2_44(WHAT, v, X, ...) WHAT(X, v) RCSW_XFE2_43(WHAT, v, __VA_ARGS__)
#define RCSW_XFE2_45(WHAT, v, X, ...) WHAT(X, v) RCSW_XFE2_44(WHAT, v, __VA_ARGS__)
#define RCSW_XFE2_46(WHAT, v, X, ...) WHAT(X, v) RCSW_XFE2_45(WHAT, v, __VA_ARGS__)
#define RCSW_XFE2_47(WHAT, v, X, ...) WHAT(X, v) RCSW_XFE2_46(WHAT, v, __VA_ARGS__)
#define RCSW_XFE2_48(WHAT, v, X, ...) WHAT(X, v) RCSW_XFE2_47(WHAT, v, __VA_ARGS__)
#define RCSW_XFE2_49(WHAT, v, X, ...) WHAT(X, v) RCSW_XFE2_48(WHAT, v, __VA_ARGS__)
#define RCSW_XFE2_50(WHAT, v, X, ...) WHAT(X, v) RCSW_XFE2_49(WHAT, v, __VA_ARGS__)

/* \endcond */

/**
 * \brief Helper macro to get the name of the RCSW_XFE[1,2]() helper
 * macro for the current iteration. Don't ever use it.
 */
#define RCSW_XGET_MACRO(                                                \
    _1, _2, _3, _4, _5, _6, _7, _8, _9, _10,                            \
    _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,                   \
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30,                   \
    _31, _32, _33, _34, _35, _36, _37, _38, _39, _40,                   \
    _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, NAME, ...)        \
  NAME

/**
 * \brief Call \p action on the current THING passed in the varargs list.
 *
 * \p action needs to have the following signature (as a macro or function):
 *
 * \code
 * mymacro(X)
 * \endcode
 *
 * where \p X is the current THING passed in varargs.
 */
#define RCSW_XFOR_EACH1(action, ...)                                    \
  RCSW_XGET_MACRO(                                                      \
      __VA_ARGS__,                                                      \
      RCSW_XFE1_50, RCSW_XFE1_49, RCSW_XFE1_48, RCSW_XFE1_47, RCSW_XFE1_46, \
      RCSW_XFE1_45, RCSW_XFE1_44,RCSW_XFE1_43, RCSW_XFE1_42, RCSW_XFE1_41, \
      RCSW_XFE1_40, RCSW_XFE1_39, RCSW_XFE1_38, RCSW_XFE1_37, RCSW_XFE1_36, \
      RCSW_XFE1_35,RCSW_XFE1_34, RCSW_XFE1_33, RCSW_XFE1_32, RCSW_XFE1_31, \
      RCSW_XFE1_30, RCSW_XFE1_29, RCSW_XFE1_28, RCSW_XFE1_27, RCSW_XFE1_26, \
      RCSW_XFE1_25, RCSW_XFE1_24, RCSW_XFE1_23, RCSW_XFE1_22, RCSW_XFE1_21, \
      RCSW_XFE1_20, RCSW_XFE1_19, RCSW_XFE1_18, RCSW_XFE1_17, RCSW_XFE1_16, \
      RCSW_XFE1_15, RCSW_XFE1_14, RCSW_XFE1_13, RCSW_XFE1_12, RCSW_XFE1_11, \
      RCSW_XFE1_10, RCSW_XFE1_9, RCSW_XFE1_8, RCSW_XFE1_7, RCSW_XFE1_6, \
      RCSW_XFE1_5, RCSW_XFE1_4, RCSW_XFE1_3, RCSW_XFE1_2, RCSW_XFE1_1)  \
  (action, __VA_ARGS__)

/**
 * \brief Call \p action on the current THING passed in the varargs list.
 *
 * \p action needs to have the following signature (as a macro or function):
 *
 * \code
 * mymacro(X)
 * \endcode
 *
 * where \p X is the current THING passed in varargs. Identical to \ref
 * RCSW_XFOR_EACH1 EXCEPT the last item in the varargs list will NOT have the \p
 * action called on it. This results in:
 *
 * \code
 * "foo" "_" "bar" "_" "baz"
 * \endcode
 *
 * generated from \code foo,bar,baz\endcode. Using \ref RCSW_XFOR_EACH1 you
 * would get:
 *
 * \code
 * "foo" "_" "bar" "_" "baz" "_"
 * \endcode
 */
#define RCSW_XFOR_EACH1_NOTAIL(action, ...)                             \
  RCSW_XGET_MACRO(                                                      \
      __VA_ARGS__,                                                      \
      RCSW_XFE1_50_NOTAIL, RCSW_XFE1_49_NOTAIL, RCSW_XFE1_48_NOTAIL,    \
      RCSW_XFE1_47_NOTAIL, RCSW_XFE1_46_NOTAIL, RCSW_XFE1_45_NOTAIL,    \
      RCSW_XFE1_44_NOTAIL, RCSW_XFE1_43_NOTAIL, RCSW_XFE1_42_NOTAIL,    \
      RCSW_XFE1_41_NOTAIL, RCSW_XFE1_40_NOTAIL, RCSW_XFE1_39_NOTAIL,    \
      RCSW_XFE1_38_NOTAIL, RCSW_XFE1_37_NOTAIL, RCSW_XFE1_36_NOTAIL,    \
      RCSW_XFE1_35_NOTAIL, RCSW_XFE1_34_NOTAIL, RCSW_XFE1_33_NOTAIL,    \
      RCSW_XFE1_32_NOTAIL, RCSW_XFE1_31_NOTAIL, RCSW_XFE1_30_NOTAIL,    \
      RCSW_XFE1_29_NOTAIL, RCSW_XFE1_28_NOTAIL, RCSW_XFE1_27_NOTAIL,    \
      RCSW_XFE1_26_NOTAIL, RCSW_XFE1_25_NOTAIL, RCSW_XFE1_24_NOTAIL,    \
      RCSW_XFE1_23_NOTAIL, RCSW_XFE1_22_NOTAIL, RCSW_XFE1_21_NOTAIL,    \
      RCSW_XFE1_20_NOTAIL, RCSW_XFE1_19_NOTAIL, RCSW_XFE1_18_NOTAIL,    \
      RCSW_XFE1_17_NOTAIL, RCSW_XFE1_16_NOTAIL, RCSW_XFE1_15_NOTAIL,    \
      RCSW_XFE1_14_NOTAIL, RCSW_XFE1_13_NOTAIL, RCSW_XFE1_12_NOTAIL,    \
      RCSW_XFE1_11_NOTAIL, RCSW_XFE1_10_NOTAIL, RCSW_XFE1_9_NOTAIL,     \
      RCSW_XFE1_8_NOTAIL, RCSW_XFE1_7_NOTAIL, RCSW_XFE1_6_NOTAIL,       \
      RCSW_XFE1_5_NOTAIL, RCSW_XFE1_4_NOTAIL, RCSW_XFE1_3_NOTAIL,       \
      RCSW_XFE1_2_NOTAIL, RCSW_XFE1_1_NOTAIL)                           \
  (action, __VA_ARGS__)

/**
 * \brief Call \p action on the current THING passed in the varargs list.
 *
 * An additional argument \p X is passed to each invocation, so \p action needs
 * to have the following signature (as a macro or function):
 *
 * \code
 * mymacro(X,V)
 * \endcode
 *
 * where \p X is the current THING passed in varargs, and \p v is the additional
 * parameter.
 */
#define RCSW_XFOR_EACH2(action, v, ...)                                      \
  RCSW_XGET_MACRO(                                                           \
      __VA_ARGS__,                                                      \
      RCSW_XFE2_50, RCSW_XFE2_49, RCSW_XFE2_48, RCSW_XFE2_47, RCSW_XFE2_46, \
      RCSW_XFE2_45, RCSW_XFE2_44,RCSW_XFE2_43, RCSW_XFE2_42, RCSW_XFE2_41, \
      RCSW_XFE2_40, RCSW_XFE2_39, RCSW_XFE2_38, RCSW_XFE2_37, RCSW_XFE2_36, \
      RCSW_XFE2_35,RCSW_XFE2_34, RCSW_XFE2_33, RCSW_XFE2_32, RCSW_XFE2_31, \
      RCSW_XFE2_30, RCSW_XFE2_29, RCSW_XFE2_28, RCSW_XFE2_27, RCSW_XFE2_26, \
      RCSW_XFE2_25, RCSW_XFE2_24, RCSW_XFE2_23, RCSW_XFE2_22, RCSW_XFE2_21, \
      RCSW_XFE2_20, RCSW_XFE2_19, RCSW_XFE2_18, RCSW_XFE2_17, RCSW_XFE2_16, \
      RCSW_XFE2_15, RCSW_XFE2_14, RCSW_XFE2_13, RCSW_XFE2_12, RCSW_XFE2_11, \
      RCSW_XFE2_10, RCSW_XFE2_9, RCSW_XFE2_8, RCSW_XFE2_7, RCSW_XFE2_6, \
      RCSW_XFE2_5, RCSW_XFE2_4, RCSW_XFE2_3, RCSW_XFE2_2, RCSW_XFE2_1)  \
      (action, v, __VA_ARGS__)

/**
 * \brief Tricky preprocessor awesomeness to get the # of arguments passed to a
 * variadac macro.
 *
 * It works by using the fact that __VA_ARGS__ is a comma-separated list of
 * arguments to determine an offset within the list of #'s that corresponds to
 * the # of arguments passed to the macro. Works with up to 100 arguments.
 */
#define RCSW_VAR_NARG(...) RCSW_VAR_NARG_(__VA_ARGS__, RCSW_VAR_RSEQ_N())

/*******************************************************************************
 * Table Generation Macros
 ******************************************************************************/
/**
 * \brief RCSW_XGEN_STR(X)
 *
 * A string repr of a token passed to \ref RCSW_XFOR_EACH1(). Used by \ref
 * RCSW_XTABLE_STR(). Don't use this directly.
 */
#define RCSW_XGEN_STR(X) STR(X),

/**
 * \brief RCSW_XGEN_SEQ_ENUM(X)
 *
 * Passed to \ref RCSW_XFOR_EACH1() by \ref RCSW_XTABLE_SEQ_ENUM. Don't use this
 * directly.
 */
#define RCSW_XGEN_SEQ_ENUM(X) X,

/**
 * \def RCSW_XTABLE_STR(...) Generate arrays of strings from tokens.
 *
 * If you have a define like this:
 *
 * \code
 * #define myents A,B,C,D,E,F,G,H
 * \endcode
 *
 * Then you can use this macro to generate an array/table of strings like so:
 *
 * \code
 * char * foo [] = { RCSW_XTABLE_STR(myents) };
 * \endcode
 *
 * which will result in:
 *
 * \code
 * char* foo[] = {"A", "B", "C", "D", "E", "F", "G", "H"}
 * \endcode

 * If you need more than 50 entries in a table, then you can do:
 *
 * \code
 * char * foo [] = {
 *                   RCSW_XTABLE_STR(myents1)
 *                   RCSW_XTABLE_STR(myents2)
 *                   RCSW_XTABLE_STR(myents3)
 *                   ...
 *                 };
 * \endcode
 *
 * in order to get the # of entries you need.
 */
#define RCSW_XTABLE_STR(...) RCSW_XFOR_EACH1(RCSW_XGEN_STR, __VA_ARGS__)

/**
 * \def RCSW_XTABLE_SEQ_ENUM(...) Generate sequential enums from tokens.
 *
 * If you have a define like this:
 *
 * \code
 * #define myents A,B,C,D,E,F,G,H
 * \endcode
 *
 * Then you can use this macro to generate an enum like so:
 *
 * \code
 * enum foo {RCSW_XTABLE_SEQ_ENUM(myents) };
 * \endcode
 *
 * which will result in:
 *
 * \code
 * enum foo {A = 0, B = 1, C = 2, D = 3, E = 4, F = 5, G = 6, H = 7}
 * \endcode
 *
 * If you need more than 50 entries, then you can do:
 *
 * \code
 * enum foo {
 *            RCSW_XTABLE_SEQ_ENUM(myents1)
 *            RCSW_XTABLE_SEQ_ENUM(myents2)
 *            RCSW_XTABLE_SEQ_ENUM(myents3)
 * };
 * \endcode
 *
 * in order to get the # of entries you need.
 */
#define RCSW_XTABLE_SEQ_ENUM(...) \
  RCSW_XFOR_EACH1(RCSW_XGEN_SEQ_ENUM, __VA_ARGS__)

/*******************************************************************************
 * Other Macros
 ******************************************************************************/
#ifndef __cplusplus

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)

/**
 * \def RCSW_STATIC_ASSERT Compile time asserts for size, alignment, etc.
 */
#define RCSW_STATIC_ASSERT(...) _Static_assert(__VA_ARGS__)
#else

#define RCSW_STATIC_ASSERT(expr, msg)                                   \
  enum { RCSW_XSTR(RCSW_STATIC_ASSERT, __LINE__) = 1 / (msg && (expr)) }
        /* extern int (*assert_function__(void)) [RCSW_STATIC_ASSERT_HELPER(expr, msg)] */

#endif /* __STDC_VERSION__ */

#endif /* __cplusplus */
