/**
 * \file
 *
 * \copyright 2023 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * \ingroup al
 *
 * \brief Clock access (monotonic and real time) for the configured platform.
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/al/al.h"

#if RCSW_CONFIG_PLATFORM == RCSW_CONFIG_PLATFORM_POSIX
#include "rcsw/al/posix/clock.h" /* IWYU pragma: export */
#elif RCSW_CONFIG_PLATFORM == RCSW_CONFIG_PLATFORM_BAREMETAL
#include "rcsw/al/baremetal/clock.h"
#else
#error "Bad target platform: must be {POSIX, BAREMETAL}."
#endif
