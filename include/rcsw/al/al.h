/**
 * \file al.h
 *
 * \copyright 2023 John Harwell, All rights reserved.
 *
 * SPDX-License Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Constants
 ******************************************************************************/
/**
 * \brief Configuration setting to build the abstraction layer for/against POSIX
 * OSes.
 */
#define RCSW_AL_TARGET_POSIX 0

/**
 * \brief Configuration setting to build the abstraction layer for bare-metal
 * applications.
 */
#define RCSW_AL_TARGET_BAREMETAL 1

/**
 * \brief Configuration setting to build the abstraction layer for
 * bootstraps. Different from NONE in that nothing from stdlib is used/included.
 */
#define RCSW_AL_TARGET_BOOTSTRAP 2

#if RCSW_CONFIG_AL_TARGET == RCSW_AL_TARGET_BAREMETAL
#define __baremetal__
#endif

#if RCSW_CONFIG_AL_TARGET == RCSW_AL_TARGET_BOOTSTRAP
#define __bootstrap__
#endif
