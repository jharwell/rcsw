/**
 * \file flags.h
 *
 * \copyright 2023 John Harwell, All rights reserved.
 *
 * SPDX-License Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Constant Definitions
 ******************************************************************************/
/**
 * \brief Declare that no flags are passed.
 */
#define RCSW_NONE           0x0

/**
 * \brief Declare that space for some structure is provided by the application.
 *
 * Applicable to xx_init() functions with an \p xx_in parameter which is ignored
 * if you do not pass this flag (even if it is non-NULL).
 *
 * Applies to all RCSW modules. Whether or not it applies to a specific part of
 * a module will be documented for each module.
 */
#define RCSW_NOALLOC_HANDLE 0x1

/**
 * \brief Declare that the space for datablocks/data the THING will directly
 * manage is provided by the application.
 *
 * If passed, you must provide a handle to the application-allocated space as a
 * parameter.
 *
 * Applies to xx_init() functions in all RCSW modules. Whether or not it applies
 * to a specific part of a module will be documented for each module.
 */
#define RCSW_NOALLOC_DATA 0x2

/**
 * \brief Declare that space for the metadata of the THING needed to manage the
 * actual data is provided by the application.
 *
 * If passed, you must provide a handle to the application-allocated space as a
 * parameter.
 *
 * Applies generally to xx_init() functions in all RCSW modules. Whether or not
 * it applies to a specific part of a module will be documented for each module.
 */
#define RCSW_NOALLOC_META 0x4


#define RCSW_MODFLAGS_START (3)
