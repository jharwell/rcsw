/**
 * \file
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * \ingroup common
 *
 * \brief Definitions, etc. common to all of RCSW.
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/core/compilers.h"
#include "rcsw/core/variadics.h"

/*******************************************************************************
 * Constants
 ******************************************************************************/
/** 1E9 */
#define RCSW_E9 1000000000

/** 1E6 */
#define RCSW_E6 1000000

/** 1E3 */
#define RCSW_E3 1000

/**
 * \brief For comparison with 0.0 for floats which gives reasonable results.
 */
#define RCSW_FLOAT_TOL 0.00000001

/**
 * \brief For comparison with 0.0 for doubles which gives reasonable results.
 */
#define RCSW_DOUBLE_TOL 0.00000000001

/******************************************************************************
 * Types
 ******************************************************************************/
/**
 * \brief A C status type.
 *
 * The basic unit of determining if a function has succeeded or not.
 */
#if defined(OK) || defined(ERROR)
#undef OK
#undef ERROR
#endif
typedef enum {
  /** Return this on function success. */
  OK = 0,
  /** Return this when a function fails. */
  ERROR = -1,
} status_t;

/*******************************************************************************
 * String Macros
 ******************************************************************************/
/* \cond INTERNAL */
#define RCSW_XSTR_(x) #x      /* don't use this one */
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
 * Generate a translation unit unique identifier using \c __COUNTER__.
 */
#define RCSW_UNIQUE_ID(prefix) RCSW_JOIN(prefix, __COUNTER__)

/*******************************************************************************
 * Comparison Macros
 ******************************************************************************/
/**
 * \def RCSW_MIN_(t1, t2, min1, min2, a, b)
 *
 * Gets the minimum of (\c a, \c b) while also performing a type comparison. If
 * the arguments do not have the same type, a compiler warning will be
 * issued. You have to EXPLICITLY cast, which is a good thing. Don't want to get
 * weird behavior when taking the max/min of different types.The type checking
 * will be compiled away at high optimization levels.
 */
#define RCSW_MIN_(t1, t2, min1, min2, a, b) \
  ({                                        \
    t1 min1 = (a);                          \
    t2 min2 = (b);                          \
    (void)(&(min1) == &(min2));             \
    (min1) < (min2) ? (min1) : (min2);      \
  })

/**
 * \def RCSW_MAX_(t1, t2, min1, min2, a, b)
 *
 * Gets the maximum of (\c a, \c b) while also performing a type comparison. If
 * the arguments do not have the same type, a compiler warning will be
 * issued. You have to EXPLICITLY cast, which is a good thing. Don't want to get
 * weird behavior when taking the max/min of different types.The type checking
 * will be compiled away at high optimization levels.
 */
#define RCSW_MAX_(t1, t2, max1, max2, a, b) \
  ({                                        \
    t1 max1 = (a);                          \
    t2 max2 = (b);                          \
    (void)(&(max1) == &(max2));             \
    (max1) > (max2) ? (max1) : (max2);      \
  })

/**
 * \def RCSW_MIN(a, b)
 *
 * Returns a type-same minimum of its arguments (compiler warnings for unsafe
 * comparisons of different types).
 */
#define RCSW_MIN(a, b)             \
  RCSW_MIN_(typeof(a),             \
            typeof(b),             \
            RCSW_UNIQUE_ID(min1_), \
            RCSW_UNIQUE_ID(min2_), \
            a,                     \
            b)

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
#define RCSW_MAX(a, b)             \
  RCSW_MAX_(typeof(a),             \
            typeof(b),             \
            RCSW_UNIQUE_ID(max1_), \
            RCSW_UNIQUE_ID(max2_), \
            a,                     \
            b)

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
#define RCSW_IS_ODD(n) ((n) & 1U)

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
/* clang-format off */
#define RCSW_IS_BETWEENC(n, low, high)         \
  ({                                           \
    typeof(n)    _n  = (n);                    \
    typeof(low)  _lo = (low);                  \
    typeof(high) _hi = (high);                 \
    (void)(&_n == &_lo); /* type-same check */ \
    (void)(&_n == &_hi);                       \
    _n >= _lo && _n <= _hi;                    \
  })
/* clang-format on */

/**
 * \def RCSW_IS_BETWEENO(n, low, high)
 *
 * Readability macro for determining if something is between an upper and lower
 * bound (open=exclusive).
 *
 * You can obviously do this without a macro, but this is (1) more self
 * documenting, and (2) less error prone.
 */
/* clang-format off */
#define RCSW_IS_BETWEENO(n, low, high)         \
  ({                                           \
    typeof(n)    _n  = (n);                    \
    typeof(low)  _lo = (low);                  \
    typeof(high) _hi = (high);                 \
    (void)(&_n == &_lo); /* type-same check */ \
    (void)(&_n == &_hi);                       \
    _n > _lo && _n < _hi;                      \
  })
/* clang-format on */

/**
 * \def RCSW_IS_BETWEENHO(n, low, high)
 *
 * Readability macro for determining if something is between an upper and lower
 * bound (half open=includes lb but not ub).
 *
 * You can obviously do this without a macro, but this is (1) more self
 * documenting, and (2) less error prone.
 */
/* clang-format off */
#define RCSW_IS_BETWEENHO(n, low, high)        \
  ({                                           \
    typeof(n)    _n  = (n);                    \
    typeof(low)  _lo = (low);                  \
    typeof(high) _hi = (high);                 \
    (void)(&_n == &_lo); /* type-same check */ \
    (void)(&_n == &_hi);                       \
    _n >= _lo && _n < _hi;                     \
  })
/* clang-format on */

/*******************************************************************************
 * Misc. Macros
 ******************************************************************************/

/* \cond INTERNAL */
#define RCSW_CLAMP_(tv, tlo, thi, v, lo, hi, uv, ulo, uhi)   \
  ({                                                         \
    tv  uv  = (v);                                           \
    tlo ulo = (lo);                                          \
    thi uhi = (hi);                                          \
    (void)(&(ulo) == &(uv)); /* same-type check lo vs val */ \
    (void)(&(uhi) == &(uv)); /* same-type check hi vs val */ \
    (uv) < (ulo) ? (ulo) : ((uv) > (uhi) ? (uhi) : (uv));    \
  })
/* \endcond */

/**
 * \def RCSW_CLAMP(val, min, max)
 *
 * Clamp a value between two setpoints.
 *
 * Safe against double evaluation of any argument: each of \a val, \a min,
 * and \a max is evaluated exactly once. A compiler warning is issued if the
 * three arguments do not share the same type (same policy as \ref RCSW_MIN
 * and \ref RCSW_MAX).
 */
#define RCSW_CLAMP(val, min, max)        \
  RCSW_CLAMP_(typeof(val),               \
              typeof(min),               \
              typeof(max),               \
              val,                       \
              min,                       \
              max,                       \
              RCSW_UNIQUE_ID(clamp_v_),  \
              RCSW_UNIQUE_ID(clamp_lo_), \
              RCSW_UNIQUE_ID(clamp_hi_))

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
#define RCSW_CONTAINER_OF(ptr, type, member) \
  ((type*)((char*)(ptr) - offsetof(type, member)))

/**
 * \def RCSW_FIELD_SIZEOF(t, f)
 * Get the size of a field in a struct \c t named \c f.
 */
#define RCSW_FIELD_SIZEOF(t, f) (sizeof(((t*)0)->f))

/**
 * \def RCSW_CHECK(cond) Check a condition in a function.
 *
 * If condition is not true, go to the error/bailout section for function (you
 * must have a label called \c error in your function). Use this when the
 * function has cleanup work to do on failure (freeing memory, closing handles,
 * etc.).
 *
 * For functions that have no cleanup on failure, prefer \ref RCSW_REQUIRE,
 * which returns directly without requiring an \c error label.
 */
#define RCSW_CHECK(cond)        \
  if (RCSW_UNLIKELY(!(cond))) { \
    goto error;                 \
  }

/**
 * \def RCSW_CHECK_PTR(ptr) Check a pointer \c ptr in a function.
 *
 * If \c ptr is NULL, go to the error/bailout section for function (you
 * must have a label called \c error in your function).
 */
#define RCSW_CHECK_PTR(ptr) RCSW_CHECK(NULL != (ptr))

/**
 * \def RCSW_CHECK_FD(fd) Check a file descriptor \c fd in a function.
 *
 * If the descriptor \c fd is invalid (i.e. < 0), go to the error/bailout
 * section for function (you must have a label called \c error in your
 * function).
 */
#define RCSW_CHECK_FD(fd) RCSW_CHECK((fd) >= 0)

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
#define RCSW_XTABLE_SEQ_ENUM(...) RCSW_XFOR_EACH1(RCSW_XGEN_SEQ_ENUM, __VA_ARGS__)

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

#define RCSW_STATIC_ASSERT(expr, msg)                                  \
  enum { RCSW_JOIN(RCSW_STATIC_ASSERT, __LINE__) = (1 / (!!(expr))) }; \
  (void)(sizeof(msg))

#endif /* __STDC_VERSION__ */

#endif /* __cplusplus */
