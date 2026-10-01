/**
 * \file
 *
 * \copyright 2019 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * \ingroup core
 *
 * \brief Compiler portability: attributes, warning control, symbol visibility,
 * and atomics.
 */

#pragma once

/*******************************************************************************
 * Warning Disable Macros
 ******************************************************************************/
/*
 * GCC and Clang both support _Pragma(). Intel oneAPI (ICX), which replaced
 * Intel C++ Compiler Classic (ICC), is LLVM/Clang-based and defines __clang__,
 * so it is handled by the Clang branch below - no separate Intel path is
 * needed. ICC (which defined __INTEL_COMPILER but not __clang__) is
 * end-of-life and no longer supported.
 */
#if defined(__clang__) || defined(__GNUC__)

/**
 * \def RCSW_PRAGMA(X)
 *
 * Indirection layer to pass a token sequence to \c _Pragma as a string.
 */
#define RCSW_PRAGMA(X) _Pragma(#X)

#else
#error Unknown compiler: only GCC and Clang (including Intel oneAPI ICX) are supported.
#endif

/**
 * \def RCSW_WARNING_DISABLE_PUSH()
 *
 * \brief Save the compiler's diagnostic state. Pair with
 * \ref RCSW_WARNING_DISABLE_POP().
 */

/**
 * \def RCSW_WARNING_DISABLE_POP()
 *
 * \brief Restore the diagnostic state saved by the matching
 * \ref RCSW_WARNING_DISABLE_PUSH().
 */

/**
 * \def RCSW_WARNING_DISABLE(X)
 *
 * \brief Suppress the compiler diagnostic named \a X (e.g.
 * \c -Wfloat-equal) until the matching \ref RCSW_WARNING_DISABLE_POP().
 *
 * Prefer the named helpers below (\ref RCSW_WARNING_DISABLE_QUAL(), ...),
 * which use the right diagnostic name for each compiler and expand to nothing
 * where it doesn't have the warning.
 */

/*
 * Clang defines both __clang__ and __GNUC__ for compatibility. Check
 * __clang__ first so that ICX (which is Clang-based) is identified correctly.
 */
#if defined(__clang__)

#define RCSW_WARNING_DISABLE_PUSH() RCSW_PRAGMA(clang diagnostic push)
#define RCSW_WARNING_DISABLE_POP() RCSW_PRAGMA(clang diagnostic pop)

/*
 * Even though Clang accepts GCC diagnostic names, it also has diagnostics that
 * GCC does not, so we keep the Clang branch separate.
 */
#define RCSW_WARNING_DISABLE(X) RCSW_PRAGMA(clang diagnostic ignored #X)

#elif defined(__GNUC__)

#define RCSW_WARNING_DISABLE_PUSH() RCSW_PRAGMA(GCC diagnostic push)
#define RCSW_WARNING_DISABLE_POP() RCSW_PRAGMA(GCC diagnostic pop)

#define RCSW_WARNING_DISABLE(X) RCSW_PRAGMA(GCC diagnostic ignored #X)

#endif /* __clang__ / __GNUC__ */

/**
 * \def RCSW_WARNING_DISABLE_QUAL()
 *
 * \brief Suppress warnings about discarding qualifiers (e.g. const). Use
 * between \ref RCSW_WARNING_DISABLE_PUSH() and \ref RCSW_WARNING_DISABLE_POP().
 */

/**
 * \def RCSW_WARNING_DISABLE_VLA()
 *
 * \brief Suppress warnings about variable-length arrays. Use between \ref
 * RCSW_WARNING_DISABLE_PUSH() and \ref RCSW_WARNING_DISABLE_POP().
 */

/**
 * \def RCSW_WARNING_DISABLE_FUNC_CAST()
 *
 * \brief Suppress warnings about casting a function call result (GCC only). Use
 * between \ref RCSW_WARNING_DISABLE_PUSH() and \ref RCSW_WARNING_DISABLE_POP().
 */

/**
 * \def RCSW_WARNING_DISABLE_STRICT_PROTO()
 *
 * \brief Suppress warnings about non-prototype declarations (GCC, C only). Use
 * between \ref RCSW_WARNING_DISABLE_PUSH() and \ref RCSW_WARNING_DISABLE_POP().
 */

/**
 * \def RCSW_WARNING_DISABLE_FLOAT_EQUAL()
 *
 * \brief Suppress warnings about comparing floating-point values for equality
 * (GCC only). Use between \ref RCSW_WARNING_DISABLE_PUSH() and \ref
 * RCSW_WARNING_DISABLE_POP().
 */

/**
 * \def RCSW_WARNING_DISABLE_REDUNDANT_DECLS()
 *
 * \brief Suppress warnings about redundant declarations (GCC only). Use between
 * \ref RCSW_WARNING_DISABLE_PUSH() and \ref RCSW_WARNING_DISABLE_POP().
 */

/**
 * \def RCSW_WARNING_DISABLE_DOCUMENTATION()
 *
 * \brief Suppress warnings about malformed documentation comments (Clang only).
 * Use between \ref RCSW_WARNING_DISABLE_PUSH() and \ref
 * RCSW_WARNING_DISABLE_POP().
 */

/*
 * Compiler-agnostic warning suppression helpers.
 * Each macro expands to the appropriate compiler-specific diagnostic name,
 * or to nothing on compilers where the warning does not exist.
 */
#if defined(__clang__)

/* clang-format off */
#define RCSW_WARNING_DISABLE_QUAL(...) \
  RCSW_WARNING_DISABLE(-Wincompatible-pointer-types-discards-qualifiers)
/* clang-format on */
/* clang-format off */
#define RCSW_WARNING_DISABLE_VLA(...) \
  RCSW_WARNING_DISABLE(-Wvla)
/* clang-format on */
#define RCSW_WARNING_DISABLE_FUNC_CAST(...)
#define RCSW_WARNING_DISABLE_STRICT_PROTO(...)
#define RCSW_WARNING_DISABLE_FLOAT_EQUAL(...)
#define RCSW_WARNING_DISABLE_REDUNDANT_DECLS(...)
#define RCSW_WARNING_DISABLE_DOCUMENTATION(...) \
  RCSW_WARNING_DISABLE(-Wdocumentation)

#elif defined(__GNUC__)

/* clang-format off */
#define RCSW_WARNING_DISABLE_REDUNDANT_DECLS(...) \
  RCSW_WARNING_DISABLE(-Wredundant-decls)
/* clang-format on */
/* clang-format off */
#define RCSW_WARNING_DISABLE_QUAL(...) \
  RCSW_WARNING_DISABLE(-Wdiscarded-qualifiers)
/* clang-format on */
/* clang-format off */
#define RCSW_WARNING_DISABLE_VLA(...) \
  RCSW_WARNING_DISABLE(-Wstack-protector)
/* clang-format on */
/* clang-format off */
#define RCSW_WARNING_DISABLE_FUNC_CAST(...) \
  RCSW_WARNING_DISABLE(-Wbad-function-cast)
#define RCSW_WARNING_DISABLE_DOCUMENTATION(...)

/* clang-format on */
/*
 * Needed when compiling C++ unit tests for C code to suppress spurious
 * warnings.
 */
#if !defined(__cplusplus)
/* clang-format off */
#define RCSW_WARNING_DISABLE_STRICT_PROTO(...) \
  RCSW_WARNING_DISABLE(-Wstrict-prototypes)
/* clang-format on */
#else
#define RCSW_WARNING_DISABLE_STRICT_PROTO(...)
#endif /* __cplusplus */

/* clang-format off */
#define RCSW_WARNING_DISABLE_FLOAT_EQUAL(...) \
  RCSW_WARNING_DISABLE(-Wfloat-equal)
/* clang-format on */

#endif /* __clang__ / __GNUC__ */

/*******************************************************************************
 * Attribute Macros
 ******************************************************************************/
#if defined(RCSW_ATTR)
#error RCSW_ATTR already defined - check for a conflicting header.
#else

/**
 * \def RCSW_ATTR(...)
 *
 * Attach a parenthesised GCC/Clang attribute list to a declaration.
 */
#define RCSW_ATTR(...) __attribute__((__VA_ARGS__))
#endif

#if defined(RCSW_ISR)
#error RCSW_ISR already defined - check for a conflicting header.
#else

/**
 * \def RCSW_ISR
 *
 * Mark a function as an interrupt service routine.
 */
#define RCSW_ISR __attribute__((interrupt))
#endif

#if defined(RCSW_SECTION)
#error RCSW_SECTION already defined - check for a conflicting header.
#else

/**
 * \def RCSW_SECTION(SECTION)
 *
 * Place a symbol into the named linker section \a SECTION.
 */
#define RCSW_SECTION(SECTION) __attribute__((section(SECTION)))
#endif

#if defined(RCSW_CONST)
#error RCSW_CONST already defined - check for a conflicting header.
#else

/**
 * \def RCSW_CONST
 *
 * Mark a function as depending only on its arguments (no global memory reads).
 * Stronger than \ref RCSW_PURE.
 */
#define RCSW_CONST __attribute__((const))
#endif

#if defined(RCSW_EXPORT)
#error RCSW_EXPORT already defined - check for a conflicting header.
#else
/**
 * \def RCSW_EXPORT
 *
 * Mark a symbol as publicly visible in a shared library build.
 */
#define RCSW_EXPORT __attribute__((visibility("default")))
#endif

#if defined(RCSW_HIDDEN)
#error RCSW_HIDDEN already defined - check for a conflicting header.
#else
/**
 * \def RCSW_HIDDEN
 *
 * Mark a symbol as private (not exported) in a shared library build.
 */
#define RCSW_HIDDEN __attribute__((visibility("hidden")))
#endif

/**
 * \def RCSW_LIB_INIT
 *
 * Mark a function to be run automatically when the library is loaded.
 */
#define RCSW_LIB_INIT __attribute__((constructor))

/**
 * \def RCSW_LIB_FINI
 *
 * Mark a function to be run automatically when the library is unloaded.
 */
#define RCSW_LIB_FINI __attribute__((destructor))

#if defined(RCSW_COLD)
#error RCSW_COLD already defined - check for a conflicting header.
#else

/**
 * \def RCSW_COLD
 *
 * Hint that a function is unlikely to be called (e.g. error paths).
 * The compiler will optimise for size and place it in a cold section.
 */
#define RCSW_COLD __attribute__((cold))
#endif

#if defined(RCSW_UNUSED)
#error RCSW_UNUSED already defined - check for a conflicting header.
#else

/**
 * \def RCSW_UNUSED
 *
 * Suppress unused-variable / unused-parameter warnings for a symbol that is
 * intentionally unused in all build configurations.
 *
 * Do \e not use this to silence warnings for symbols that are only unused in
 * some configurations (e.g. debug-only variables); in those cases the warning
 * is useful and should be resolved structurally.
 */
#if defined(NDEBUG)
#define RCSW_UNUSED __attribute__((unused))
#else
#define RCSW_UNUSED
#endif /* NDEBUG */

#endif /* RCSW_UNUSED */

#if defined(RCSW_WUR)
#error RCSW_WUR already defined - check for a conflicting header.
#else

/**
 * \def RCSW_WUR
 *
 * Warn at call sites that discard the return value of this function.
 */
#define RCSW_WUR __attribute__((warn_unused_result))
#endif

#if defined(RCSW_PURE)
#error RCSW_PURE already defined - check for a conflicting header.
#else

/**
 * \def RCSW_PURE
 *
 * Mark a function as depending only on its arguments and (possibly) global
 * memory reads, but with no side effects. Weaker than \ref RCSW_CONST.
 */
#define RCSW_PURE __attribute__((pure))
#endif

#if defined(RCSW_NORETURN)
#error RCSW_NORETURN already defined - check for a conflicting header.
#else
/**
 * \def RCSW_NORETURN
 *
 * Mark a function that never returns (e.g. calls \c abort() or loops forever).
 */
#define RCSW_NORETURN __attribute__((noreturn))
#endif

#if defined(RCSW_WEAK)
#error RCSW_WEAK already defined - check for a conflicting header.
#else
/**
 * \def RCSW_WEAK
 *
 * Mark a function as a weak symbol, allowing it to be overridden at link time
 * by a strong definition in another translation unit.
 */
#define RCSW_WEAK __attribute__((weak))
#endif

#if defined(RCSW_LIKELY)
#error RCSW_LIKELY already defined - check for a conflicting header.
#else
/**
 * \def RCSW_LIKELY(x)
 *
 * Hint to the compiler that the condition \a x is almost always true.
 * Use sparingly and only where profiling confirms it matters.
 */
#define RCSW_LIKELY(x) __builtin_expect(!!(x), 1)
#endif

#if defined(RCSW_UNLIKELY)
#error RCSW_UNLIKELY already defined - check for a conflicting header.
#else
/**
 * \def RCSW_UNLIKELY(x)
 *
 * Hint to the compiler that the condition \a x is almost always false.
 * Use sparingly and only where profiling confirms it matters.
 */
#define RCSW_UNLIKELY(x) __builtin_expect(!!(x), 0)
#endif

#if defined(RCSW_ATTR_PRINTF)
#error RCSW_ATTR_PRINTF already defined - check for a conflicting header.
#endif

#if defined(__clang__)
#define RCSW_ATTR_PRINTF(...) RCSW_ATTR(format(printf, __VA_ARGS__))

#elif defined(__GNUC__)
#define RCSW_ATTR_PRINTF(...) RCSW_ATTR(format(gnu_printf, __VA_ARGS__))

#endif /* __clang__ */

/*******************************************************************************
 * Visibility Macros
 ******************************************************************************/
#if defined(RCSW_EXPORTS)
/* Building RCSW as a shared library. */
#define RCSW_API RCSW_EXPORT
#define RCSW_LOCAL RCSW_HIDDEN
#else
/* Building RCSW as a static library. */
#define RCSW_API
#define RCSW_LOCAL
#endif

/*******************************************************************************
 * Other Macros
 ******************************************************************************/
#if defined(__GNUC__) && !defined(__clang__)
/**
 * \def __FILE_NAME__
 *
 * Only recent GCC versions provide \c __FILE_NAME__ (basename only).
 * Fall back to \c __FILE__ on older versions.
 */
#if !defined(__FILE_NAME__)
#define __FILE_NAME__ __FILE__
#endif
#endif

#ifndef typeof
#define typeof __typeof__
#endif
/*******************************************************************************
 * C++ Compatibility
 ******************************************************************************/
#ifdef __cplusplus

#ifndef BEGIN_C_DECLS

/**
 * \def BEGIN_C_DECLS
 *
 * Opens an \c extern "C" block for C++ translation units.
 */
#define BEGIN_C_DECLS extern "C" {
#endif

#ifndef END_C_DECLS

/**
 * \def END_C_DECLS
 *
 * Closes the \c extern "C" block opened by \ref BEGIN_C_DECLS.
 */
#define END_C_DECLS }
#endif /* END_C_DECLS */

#else

#define BEGIN_C_DECLS
#define END_C_DECLS

#endif /* __cplusplus */

/*******************************************************************************
 * Atomics
 ******************************************************************************/
/*
 * Thin wrappers around the GCC/Clang __atomic builtins. These are used instead
 * of C11 <stdatomic.h> so that the same struct definitions are usable from C
 * and C++, and so that the fields stay plain (non-_Atomic) types. Only
 * word-sized loads/stores/exchanges are used, which are lock-free on every
 * target RCSW supports (including Cortex-M0, which has no LDREX/STREX, for
 * loads and stores).
 */

/**
 * \def RCSW_ATOMIC_LOAD_ACQ(PTR)
 *
 * Atomically load \a *PTR with acquire ordering.
 */
#define RCSW_ATOMIC_LOAD_ACQ(PTR) __atomic_load_n((PTR), __ATOMIC_ACQUIRE)

/**
 * \def RCSW_ATOMIC_STORE_REL(PTR, VAL)
 *
 * Atomically store \a VAL to \a *PTR with release ordering.
 */
#define RCSW_ATOMIC_STORE_REL(PTR, VAL) \
  __atomic_store_n((PTR), (VAL), __ATOMIC_RELEASE)

/**
 * \def RCSW_ATOMIC_XCHG_ACQ(PTR, VAL)
 *
 * Atomically store \a VAL to \a *PTR and return the previous value, with
 * acquire ordering.
 */
#define RCSW_ATOMIC_XCHG_ACQ(PTR, VAL) \
  __atomic_exchange_n((PTR), (VAL), __ATOMIC_ACQUIRE)
