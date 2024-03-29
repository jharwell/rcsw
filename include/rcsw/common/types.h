/**
 * \file types.h
 * \ingroup common
 * \brief Base type definitions for RCSW.
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

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
 * The OS preprocessor macros are defined automatically by the compiler. To see
 * what the default macros for a gcc-like compiler are, issue the command:
 *
 * gcc -dM -E - < /dev/NULL
 */

#if defined(__nos__) /* for bare metal */

/* typedefs */
typedef unsigned long uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

typedef long int32_t;
typedef short int16_t;
typedef char int8_t;

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


extern uint32_t errno;

#elif defined(__linux__) || defined(__arm__)

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
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
#error UNKNOWN OS: __nos__, __linux__ suppored
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
#if (RCSW_PTR_ALIGN == 4)
typedef uint32_t dptr_t;
#elif (RCSW_PTR_ALIGN == 2)
typedef uint16_t dptr_t;
#elif (RCSW_PTR_ALIGN == 1)
typedef uint8_t dptr_t;
#else
#error RCSW currently supports [1,2,4] byte pointer alignment
#endif
