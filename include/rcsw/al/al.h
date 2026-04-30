/**
 * \file
 *
 * \copyright 2023 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Constants
 ******************************************************************************/
/**
 * \brief Configuration setting to build the abstraction layer for/against POSIX
 * OSes.
 */
#define RCSW_CONFIG_PLATFORM_POSIX 0

/**
 * \brief Configuration setting to build the abstraction layer for bare-metal
 * applications.
 */
#define RCSW_CONFIG_PLATFORM_BAREMETAL 1

#if !defined(RCSW_CONFIG_PLATFORM)
#error RCSW_CONFIG_PLATFORM is not defined. \
  Set it to RCSW_CONFIG_PLATFORM_POSIX or RCSW_CONFIG_TARGET_BAREMETAL.
#endif

#if RCSW_CONFIG_PLATFORM == RCSW_CONFIG_PLATFORM_POSIX
#define RCSW_PLATFORM_POSIX
#elif RCSW_CONFIG_PLATFORM == RCSW_CONFIG_PLATFORM_BAREMETAL
#define RCSW_PLATFORM_BAREMETAL

#endif
