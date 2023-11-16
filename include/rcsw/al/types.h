/**
 * \file types.h
 * \ingroup al
 * \brief Base type definitions for RCSW.
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
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


#if defined(__bootstrap__) /* for stdlib-less bootstraps */

/*
 * These are OK to include because they are header-only, and don't rely on
 * stdlib. -nostdinc would make including these an error, but I don't think that
 * makes sense, as they provide much better/more accurate typedefs across ANY
 * bootstrap platform.
 */
#include <limits.h>
#include <stdint.h>

/*
 * The OS preprocessor macros are defined automatically by the compiler. To see
 * what the default macros for a gcc-like compiler are, issue the command:
 *
 * gcc -dM -E - < /dev/null
 */

/* typedefs */
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

typedef short int16_t;
typedef uint32_t size_t;
typedef signed long int64_t;
typedef long ptrdiff_t;
typedef long intmax_t;
typedef int64_t int_fast64_t;

extern uint32_t errno;

#define EINVAL -1

/* defines */
#ifndef NULL
#define NULL ((void *)0)
#endif /* NULL */

#if defined(true) || defined(false)
#undef true
#undef false
#endif

typedef enum {
  false = 0,
  true  = 1
} bool_t;

/*
 * For bare-metal applications we can still use the stdlib.
 */
#elif defined(__linux__) || defined(__baremetal__)

#include <stdint.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <stdbool.h>
#include <assert.h>

#if defined(DOXYGEN_DOCUMENTATION_BUILD)

#undef true
#undef false

/**
 * \brief A C boolean type.
 *
 * In linux, this is the same as bool; on embedded/baremetal platforms it maybe
 * defined differently.
 */
typedef enum {
  false = 0,
  true  = 1
} bool_t;
#else
#define bool_t bool
#endif


#else
#error Bad AL target: {__baremetal__, __linux__, __bootstrap__} supported
#endif

/*******************************************************************************
 * Custom Type Definitions
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
    OK       = 0,
    /** Return this when a function fails. */
    ERROR    = -1,
} status_t;


/**
 * \brief A runtime exec method switch for SOMETHING.
 */
enum exec_type {
  /**
   * Use a recursive runtime implementation.
   */
  ekEXEC_REC,
  /**
   * Use an iterative runtime implementation.
   */
  ekEXEC_ITER,
};

/**
 * \brief A C data pointer type.
 *
 * This is the pointer type to store all persistent runtime data in RCSW, which
 * enables casts like:
 *
 * \code
 * (struct mystruct*)node->data
 * \endcode
 *
 * work as intended on all targets. On some embedded targets like ARM, if \code
 * node->data \endcode has an alignment of 1 because it points to an address on
 * the stack, then casting to \code mystruct \endcode which may have alignment >
 * 1 can cause a hardware trap mecause the low-level instructions the compiler
 * generates to access \code mystruct \endcode assumes whatever is pointed at
 * has the expected alignment.
 *
 * This can be overriden at compile time.
 */
#if (RCSW_CONFIG_PTR_ALIGN == 4)
typedef uint32_t dptr_t;
#elif (RCSW_CONFIG_PTR_ALIGN == 2)
typedef uint16_t dptr_t;
#elif (RCSW_CONFIG_PTR_ALIGN == 1)
typedef uint8_t dptr_t;
#else
#error RCSW currently supports [1,2,4] byte pointer alignment
#endif
