/**
 * \file hash.h
 * \ingroup utils
 * \brief A collection of hashing algorithms.
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * This file is part of RCSW.
 *
 * RCSW is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * RCSW is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * RCSW.  If not, see <http://www.gnu.org/licenses/
 */

#ifndef INCLUDE_RCSW_UTILS_HASH_H_
#define INCLUDE_RCSW_UTILS_HASH_H_

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

#endif /* INCLUDE_RCSW_UTILS_HASH_H_ */
