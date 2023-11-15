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

#if RCSW_CONFIG_AL_TARGET == RCSW_AL_TARGET_LINUX
#include "rcsw/al/posix/clock.h"
#elif RCSW_CONFIG_AL_TARGET == RCSW_AL_TARGET_NONE
#include "rcsw/al/none/clock.h"
#endif
