/**
 * \file compilers.h
 *
 * \copyright 2019 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 */

#pragma once

/*******************************************************************************
 * Warning Disable Macros
 ******************************************************************************/
/*
 * clang, GCC, Intel support the same _Pragma() construct, so we handle them all
 * at once.
 */
#if defined(__GNUC__) || defined(__clang__) || defined(__INTEL_COMPILER)

/**
 * \def RCSW_PRAGMA(X) Indirection to get the argument to the pragma as a
 * string.
 */
#define RCSW_PRAGMA(X) _Pragma(RCSW_XSTR(X))

#else
#error Unknown compiler

#endif /* __GNUC__ || __clang__ || __INTEL_COMPILER */

/*
 * The intel compilers defines both __INTEL_COMPILER and __GNUC__ for
 * compability so we need to check for __INTEL_COMPILER first to correctly
 * detect it.
 */
#if defined(__INTEL_COMPILER)

#define RCSW_WARNING_DISABLE_PUSH(...)          \
  RCSW_PRAGMA(warning push)

#define RCSW_WARNING_DISABLE_POP(...)           \
  RCSW_PRAGMA(warning pop)

#define RCSW_WARNING_DISABLE(X) RCSW_PRAGMA(warning disable X)

/*
 * Clang defines both __clang__ and __GNUC__ for compatability, so we need to
 * check for __clang__ first in order to correctly detect it.
 */
#elif defined(__clang__)

#define RCSW_WARNING_DISABLE_PUSH(...)          \
  RCSW_PRAGMA(clang diagnostic push)

#define RCSW_WARNING_DISABLE_POP(...)           \
  RCSW_PRAGMA(clang diagnostic pop)

/*
 * Even though clang supports GCC diagnostics, there are ADDITIONAL diagnostics
 * that GCC does *NOT* support, so we need a separate case.
 */
#define RCSW_WARNING_DISABLE(X) \
  RCSW_PRAGMA(clang diagnostic ignored RCSW_XSTR(X))

#elif defined(__GNUC__)

#define RCSW_WARNING_DISABLE_PUSH(...)                \
  RCSW_PRAGMA(GCC diagnostic push)

#define RCSW_WARNING_DISABLE_POP(...)            \
  RCSW_PRAGMA(GCC diagnostic pop)

/**
 * \def RCSW_WARNING_DISABLE_END(X) Specify a compiler warning to disable
 * (additional #defines needed to define compiler-agnostic names for warnings),
 * ending scope. This should *NOT* be used willy-nilly; 99% of the time
 * compilers emit warnings for a good reason.
 */
#define RCSW_WARNING_DISABLE(X) RCSW_PRAGMA(GCC diagnostic ignored RCSW_XSTR(X))

#endif /* __GNUC__ */

/*
 * Now define common warning disable macros in a compiler agnostic way.
 */
#if defined(__INTEL_COMPILER)

#define RCSW_WARNING_DISABLE_QUAL(...)
#define RCSW_WARNING_DISABLE_VLA(...)
#define RCSW_WARNING_DISABLE_FUNC_CAST(...)
#define RCSW_WARNING_DISABLE_STRICT_PROTO(...)
#define RCSW_WARNING_DISABLE_FLOAT_EQUAL(...)

#elif defined(__clang__)

#define RCSW_WARNING_DISABLE_QUAL(...) \
  RCSW_WARNING_DISABLE(-Wincompatible-pointer-types-discards-qualifiers)
#define RCSW_WARNING_DISABLE_VLA(...) \
  RCSW_WARNING_DISABLE(-Wvla)
#define RCSW_WARNING_DISABLE_FUNC_CAST(...)
#define RCSW_WARNING_DISABLE_STRICT_PROTO(...)
#define RCSW_WARNING_DISABLE_FLOAT_EQUAL(...)

#elif defined(__GNUC__)

#define RCSW_WARNING_DISABLE_QUAL(...) \
  RCSW_WARNING_DISABLE(-Wdiscarded-qualifiers)
#define RCSW_WARNING_DISABLE_VLA(...) \
    RCSW_WARNING_DISABLE(-Wstack-protector)
#define RCSW_WARNING_DISABLE_FUNC_CAST(...) \
  RCSW_WARNING_DISABLE(-Wbad-function-cast)

/*
 * Needed when compiling C++ unit tests for C code to suppress
 * spurious warnings.
*/
#if !defined(__cplusplus)
#define RCSW_WARNING_DISABLE_STRICT_PROTO(...) \
  RCSW_WARNING_DISABLE(-Wstrict-prototypes)
#else
#define RCSW_WARNING_DISABLE_STRICT_PROTO(...)
#endif /* __cplusplus */

#define RCSW_WARNING_DISABLE_FLOAT_EQUAL(...) \
  RCSW_WARNING_DISABLE(-Wfloat-equal)

#endif

/*******************************************************************************
 * Attribute Macros
 ******************************************************************************/
/*
 * Attribute macros supported by [gcc, clang, intel] first
 */
#if defined (RCSW_ATTR)
#error "RCSW_ATTR defined!"

#else

/**
 * \def RCSW_ATTR() for attaching a list of attributes to a
 * function/variable/etc.
 */
#define RCSW_ATTR(...) __attribute__((__VA_ARGS__))

#endif /* RCSW_ATTR */

#if defined(RCSW_CONST)
#error "RCSW_CONST defined!"

#else

/**
 * \def RCSW_CONST Shorthand for marked a function as purely function of its
 * input parameters only (no global memory access allowed).
 */
#define RCSW_CONST __attribute__((const))

#endif /* RCSW_CONST */

#if defined(RCSW_EXPORT)
#error "RCSW_EXPORT defined!"

#else
/**
 * \def RCSW_EXPORT
 *
 * Shorthand for marked a function as publicly visible in a shared library.
 */
#define RCSW_EXPORT __attribute__((visibility("default")))

#endif /* RCSW_EXPORT */

/*
 * Intel compiler (as of version 19) does not support these attribute macros
 */
#if defined(__INTEL_COMPILER)

#define RCSW_COLD
#define RCSW_UNUSED
#define RCSW_PURE
#define RCSW_DEAD
#define RCSW_CHECK_RET

#elif defined(__clang__) || defined(__GNUC__)

#if defined(RCSW_COLD)
#error "RCSW_COLD defined!"

#else

/**
 * \def RCSW_COLD Shorthand for declaring something as unlikely to be executed.
 */
#define RCSW_COLD __attribute__((cold))

#endif /* RCSW_COLD */

#if defined(RCSW_UNUSED)
#error "RCSW_UNUSED defined!"

#else

/**
 * \def RCSW_UNUSED Shorthand for declaring something unused. We only need it to
 * stop spurious compiler warnings when optimizations are on. When they are not,
 * then such warnings are should keep showing up to remind developers that
 * something is (potentially) off with their function(s).
 */
#if defined(NDEBUG)
#define RCSW_UNUSED __attribute__((unused))
#else
#define RCSW_UNUSED
#endif /* NDEBUG */

#endif /* RCSW_UNUSED */

#if defined(RCSW_CHECK_RET)
#error "RCSW_CHECK_RET defined!"

#else

/**
 * \def RCSW_CHECK_RET Shorthand for enhancing compile checking of return value
 * usage.
 */
#define RCSW_CHECK_RET __attribute__((warn_unused_result))

#endif /* RCSW_CHECK_RET */

#if defined(RCSW_PURE)
#error "RCSW_PURE defined!"

#else

/**
 * \def RCSW_PURE Shorthand for marked a function as purely function of its
 * input parameters and (possibly) global data.
 */
#define RCSW_PURE __attribute__((pure))

#endif /* RCSW_PURE */


#if defined(RCSW_DEAD)
#error "RCSW_DEAD defined!"

#else
/**
 * \def RCSW_DEAD Shorthand for marked a function as one that will not return.
 */
#define RCSW_DEAD __attribute__((noreturn))

#endif /* RCSW_DEAD */

#endif /* __clang__ || __GNUC__ */

#ifdef __cplusplus

#ifndef BEGIN_C_DECLS
/**
 * \def BEGIN_C_DECLS Standard C++ compatability wrapper for C code (start)
 */
#define BEGIN_C_DECLS extern "C" {
#endif /* BEGIN_C_DECLS */

#ifndef END_C_DECLS
/**
 * \def END_C_DECLS Standard C++ compatability wrapper for C code (end)
 */
#define END_C_DECLS }
#endif /* END_C_DECLS */

#else

#define BEGIN_C_DECLS
#define END_C_DECLS

#endif /* __cplusplus */
