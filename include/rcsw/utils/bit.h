/**
 * \file
 *
 * \copyright 2017 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * \ingroup utils
 *
 * \brief Bit-width helper.
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <limits.h>
#include <stddef.h>

/*******************************************************************************
 * Macros
 ******************************************************************************/
/**
 * \brief Width in bits of type \p t. A char is 8 bits on almost every
 * platform, but for some DSPs, it can be something different.
 */
#define RCSW_BIT_WIDTH(t) (sizeof(t) * CHAR_BIT)
