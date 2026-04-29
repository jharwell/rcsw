/**
 * \file
 *
 * \copyright 2023 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/al/al.h"

#if RCSW_CONFIG_PLATFORM == RCSW_CONFIG_PLATFORM_POSIX
#include "rcsw/al/posix/clock.h"
#elif RCSW_CONFIG_PLATFORM == RCSW_CONFIG_PLATFORM_BAREMETAL
#include "rcsw/al/baremetal/clock.h"
#else
#error "Bad target platform: must be {POSIX, BAREMETAL}."
#endif
