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
#include "rcsw/rcsw.h"

/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS
#ifndef DOXYGEN_DOCUMENTATION_BUILD
/**
 * \brief Get the monotonic system time.
 *
 * \return The time.
 */
RCSW_API struct timespec clock_monotime(void);

/**
 * \brief Get the real system time.
 *
 * \return The time.
 */
RCSW_API struct timespec clock_realtime(void);

#endif
END_C_DECLS
