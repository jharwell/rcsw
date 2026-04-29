/**
 * \file
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * \ingroup utils
 *
 * \brief A collection of hashing algorithms.
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <stddef.h>
#include <stdint.h>

#include "rcsw/core/compilers.h"
#include "rcsw/core/core.h"

/*******************************************************************************
 * Public API
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Compute a hash over \p data (Bob Jenkins / Wikipedia algorithm).
 *
 * \param data Pointer to the data to hash.
 * \param len  Number of bytes to hash.
 * \param hash The computed hash.
 *
 * \return \ref status_t
 */
RCSW_API status_t utils_hash_default(const void* data,
                                     size_t      len,
                                     uint32_t*   hash) RCSW_PURE;

/**
 * \brief Compute a hash over \p data using FNV-1a (Fowler–Noll–Vo).
 *
 * Good general-purpose hash with low collision rates. This is the default
 * hash function used by \ref hashmap when no custom hash is supplied.
 *
 * \param data Pointer to the data to hash.
 * \param len  Number of bytes to hash.
 * \param hash The computed hash.
 *
 * \return \ref status_t.
 */
RCSW_API status_t utils_hash_fnv1a(const void* data,
                                   size_t      len,
                                   uint32_t*   hash) RCSW_PURE;

/**
 * \brief Compute a hash over \p data using DJB2 (Dan J. Bernstein).
 *
 * Fast; well-suited for short string keys.
 *
 * \param data Pointer to the data to hash.
 * \param len  Number of bytes to hash.
 * \param hash The computed hash.
 *
 * \return \ref status_t
 */
RCSW_API status_t utils_hash_djb(const void* data,
                                 size_t      len,
                                 uint32_t*   hash) RCSW_PURE;

END_C_DECLS
