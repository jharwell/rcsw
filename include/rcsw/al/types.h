/**
 * \file
 *
 * \copyright 2017 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * \ingroup al
 *
 * \brief Base type definitions for RCSW.
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
/*
 * These are OK to include because unconditionally because they are header-only,
 * and don't rely on stdlib. -nostdinc would make including these an error, but
 * I don't think that makes sense, as they provide much better/more accurate
 * typedefs across ANY bootstrap platform.
 */
#include <limits.h>   // IWYU pragma: export, keep
#include <stdbool.h>  // IWYU pragma: export, keep
#include <stddef.h>   // IWYU pragma: export, keep
#include <stdint.h>   // IWYU pragma: export, keep

#include "rcsw/al/al.h"

/*******************************************************************************
 * Basic Type Definitions
 ******************************************************************************/
/*
 * Necessary because (apparently putting __linux__=1 in the PREDEFINED list in
 * the doxyfile doesn't work with nested #if statements).
 */
#if defined(DOXYGEN_DOCUMENTATION_BUILD)
#define __linux__ 1
#endif

/*
 * For Linux applications.
 */
#if defined(RCSW_PLATFORM_POSIX)

#if defined(DOXYGEN_DOCUMENTATION_BUILD)

#undef true
#undef false

/**
 * \brief A C boolean type.
 *
 * In linux, this is the same as bool; on embedded/baremetal platforms it maybe
 * defined differently.
 */
typedef enum { false = 0, true = 1 } bool_t;
#else
#define bool_t bool
#endif

/*
 * For bare-metal applications with no OS (can still use stdlib)
 */
#elif defined(RCSW_PLATFORM_BAREMETAL)

#if defined(RCSW_NOSTDLIB)

/*
 * The OS preprocessor macros are defined automatically by the compiler. To see
 * what the default macros for a gcc-like compiler are, issue the command:
 *
 * gcc -dM -E - < /dev/null
 */

/* defines */
#ifndef NULL
#define NULL ((void*)0)
#endif /* NULL */

#endif /* RCSW_NO_STDLIB */

#define bool_t bool

#endif

/*******************************************************************************
 * Custom Type Definitions
 ******************************************************************************/
/**
 * \brief A C data pointer type.
 *
 * All persistent runtime data in RCSW is stored via this type so that casts
 * like:
 *
 * \code
 * (struct mystruct*)node->data
 * \endcode
 *
 * work as intended on all targets. On some embedded targets (e.g., ARM), if
 * \c node->data points to a stack address with alignment 1, casting to a
 * struct with alignment > 1 can cause a hardware trap because the compiler
 * generates load/store instructions that assume the required alignment.
 *
 * \par Choosing a value
 * Use the alignment of the most strictly aligned type stored in RCSW-managed
 * memory. On 64-bit targets which store pointers (or structs containing them)
 * in caller-provided data structure memory, that is 8.
 *
 * \par Implementation
 * Alignment is enforced by choosing an integer typedef whose natural alignment
 * matches \c RCSW_CONFIG_PTR_ALIGN, so that array elements of that type are
 * aligned to it. This is checked when the library is built: some ABIs (e.g.,
 * 32-bit x86) align \c uint64_t to 4 bytes, and 8 is rejected there.
 *
 * \note A C11 \c _Alignas approach was considered but rejected: \c _Alignas
 * is not permitted on a \c typedef (C11 §6.7.5), and placing it on a
 * single-member struct wrapper breaks call-site transparency between the C11
 * and pre-C11 paths. The integer-width trick is already sufficient — \c
 * _Alignof(uint32_t)==4 guarantees 4-byte-aligned array elements just as
 * reliably as \c _Alignas(4) would.
 *
 */
#if !defined(RCSW_CONFIG_PTR_ALIGN)
#error RCSW_CONFIG_PTR_ALIGN is not defined. \
  Define it to 1, 2, 4, or 8.
#endif

#if (RCSW_CONFIG_PTR_ALIGN != 1) && (RCSW_CONFIG_PTR_ALIGN != 2) && \
  (RCSW_CONFIG_PTR_ALIGN != 4) && (RCSW_CONFIG_PTR_ALIGN != 8)
#error RCSW_CONFIG_PTR_ALIGN must be 1, 2, 4, or 8.
#endif

#if (RCSW_CONFIG_PTR_ALIGN == 8)
typedef uint64_t dptr_t;
#elif (RCSW_CONFIG_PTR_ALIGN == 4)
typedef uint32_t dptr_t;
#elif (RCSW_CONFIG_PTR_ALIGN == 2)
typedef uint16_t dptr_t;
#else /* RCSW_CONFIG_PTR_ALIGN == 1 */
typedef uint8_t dptr_t;
#endif
