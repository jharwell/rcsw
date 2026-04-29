/**
 * \file hash.h
 * \ingroup utils
 * \brief A collection of hashing algorithms.
 *
 * All functions take a data pointer and byte length and return a
 * \c uint32_t hash. Return 0 if an error occurs (NULL data or zero
 * length).
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
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

/**
 * \brief Compute a hash over \p data (Bob Jenkins / Wikipedia algorithm).
 *
 * \param data Pointer to the data to hash.
 * \param len  Number of bytes to hash.
 *
 * \return The hash value, or 0 on error.
 */
RCSW_API uint32_t hash_default(const void * data, size_t len) RCSW_PURE;

/**
 * \brief Compute a hash over \p data using FNV-1a (Fowler–Noll–Vo).
 *
 * Good general-purpose hash with low collision rates. This is the default
 * hash function used by \ref hashmap when no custom hash is supplied.
 *
 * \param data Pointer to the data to hash.
 * \param len  Number of bytes to hash.
 *
 * \return The hash value, or 0 on error.
 */
RCSW_API uint32_t hash_fnv1a(const void * data, size_t len) RCSW_PURE;

/**
 * \brief Compute a hash over \p data using DJB2 (Dan J. Bernstein).
 *
 * Fast; well-suited for short string keys.
 *
 * \param data Pointer to the data to hash.
 * \param len  Number of bytes to hash.
 *
 * \return The hash value, or 0 on error.
 */
RCSW_API uint32_t hash_djb(const void * data, size_t len) RCSW_PURE;

END_C_DECLS
