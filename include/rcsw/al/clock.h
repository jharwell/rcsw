/**
 * \file clock.h
 *
 * \copyright 2023 John Harwell, All rights reserved.
 *
 * SPDX-License Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/al/al.h"

#if RCSW_CONFIG_AL_TARGET == RCSW_AL_TARGET_POSIX
#include "rcsw/al/posix/clock.h"
#elif RCSW_CONFIG_AL_TARGET == RCSW_AL_TARGET_BAREMETAL
#include "rcsw/al/baremetal/clock.h"
#else
#error "No supported clock for AL target"
#endif
