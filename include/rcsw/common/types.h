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

extern uint32_t errno;

/* includes */
#elif defined(__linux__)

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#else
#error UNKNOWN OS: __nos__, __linux__ suppored
#endif

/*******************************************************************************
 * Custom Type Definitions
 ******************************************************************************/
/**
 * \brief The basic unit of determining if a function has succeeded or not.
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
 * \brief A C boolean type.
 */
#if defined(TRUE) || defined(FALSE)
#undef TRUE
#undef FALSE
#endif

typedef enum {
    FALSE = 0,
    TRUE  = 1
} bool_t;

