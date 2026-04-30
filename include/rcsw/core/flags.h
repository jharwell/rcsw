/**
 * \file
 *
 * \copyright 2023 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/******************************************************************************
 * Includes
 ******************************************************************************/
#include <stdint.h>

/*******************************************************************************
 * Constant Definitions
 ******************************************************************************/
/**
 * \brief Declare that no flags are passed.
 */
#define RCSW_NONE UINT32_C(0x0)
/**
 * \brief Declare that space for some structure is provided by the application.
 *
 * Applicable to xx_init() functions with an \p xx_in parameter which is ignored
 * if you do not pass this flag (even if it is non-NULL).
 *
 * Applicable to all RCSW modules. Whether or not it applies to a specific part
 * of a module will be documented for each module.
 */
#define RCSW_NOALLOC_HANDLE UINT32_C(0x1)

/**
 * \brief Declare that the space for datablocks/data the THING will directly
 * manage is provided by the application.
 *
 * If passed, you must provide a handle to the application-allocated space as a
 * parameter.
 *
 * Applicable to xx_init() functions in all RCSW modules. Whether or not it
 * applies to a specific part of a module will be documented for each module.
 */
#define RCSW_NOALLOC_DATA UINT32_C(0x2)

/**
 * \brief Declare that space for the metadata of the THING needed to manage the
 * actual data is provided by the application.
 *
 * If passed, you must provide a handle to the application-allocated space as a
 * parameter.
 *
 * Applicable to xx_init() functions in all RCSW modules. Whether or not it
 * applies to a specific part of a module will be documented for each module.
 */
#define RCSW_NOALLOC_META UINT32_C(0x4)

/**
 * \brief Convenience constant for callers that provide all necessary memory.
 */
#define RCSW_NOALLOC_ALL \
  (RCSW_NOALLOC_HANDLE | RCSW_NOALLOC_DATA | RCSW_NOALLOC_META)

/**
 * \brief Declare that any allocated space, regardless of where it comes from,
 * should be zeroed before use.
 *
 * Applicable to all modules which do memory allocation via \ref rcsw_alloc().
 *
 * If dynamic memory allocation is allowed, this causes it to use calloc()
 * instead of malloc(). It memory allocation is disallowed, then provided
 * application-allocated memory will be zeroed via memset().
 */
#define RCSW_ZALLOC UINT32_C(0x8)

/**
 * \brief Necessary bit shift to get to unused bits for RCSW configuration flags.
 */
#define RCSW_MODFLAGS_SHIFT (4)

/**
 * \brief Used by modules in RCSW to start their list of bitmask flags
 *
 * Prevents conflicts with common flags broadly applicable everywhere.
 */
#define RCSW_MODFLAGS_BASE (1U << RCSW_MODFLAGS_SHIFT)
