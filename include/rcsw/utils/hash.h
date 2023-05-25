/**
 * \file hash.h
 * \ingroup utils
 * \brief A collection of hashing algorithms.
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/common/common.h"

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Compute a hash over data
 *
 * Hash algorthim from Bob Jenkins on wikipedia.
 *
 * \param data The data to hash over
 * \param len # of bytes of data to hash
 *
 * \return The hash, or 0 if an ERROR occurred
 */
uint32_t hash_default(const void * data, size_t len) RCSW_PURE;

/**
 * \brief Compute a hash over data
 *
 * \param data The data to hash over
 * \param len # of bytes of data to hash
 *
 * Hash algorithm from Glenn Fowler, Phong Vo, and Landon Curt Noll.
 *
 * \return The hash, or 0 if an ERROR occurred
 *
 */
uint32_t hash_fnv1a(const void * data, size_t len) RCSW_PURE;

/**
 * \brief Compute a hash over data
 *
 * Hash algorithm from Dan J. Bernstein.
 *
 * \param data The data to hash over
 * \param len # of bytes of data to hash
 *
 * \return The hash, or 0 if an ERROR occurred
 *
 */
uint32_t hash_djb(const void * data, size_t len) RCSW_PURE;

END_C_DECLS

