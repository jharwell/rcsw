/**
 * \file alloc.h
 *
 * \copyright 2023 John Harwell, All rights reserved.
 *
 * SPDX-License Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/common/common.h"

/*******************************************************************************
 * API Functions
 ******************************************************************************/
/**
 * \brief Allocate \p n_bytes of memory using malloc()/calloc()/etc.
 *
 * This function is the one place in RCSW where memory allocation is done,
 * making it easy and clean for RCSW to be built to:
 *
 * - Never use malloc() for anything (i.e., set \ref RCSW_NOALLOC for all
 *   modules). Useful in applications where malloc() is disallowed/not
 *   available, such as space and bootstraps without stdlib.
 *
 * - Zero allocated memory (i.e., behave as calloc() and set \ref RCSW_ZALLOC
 *   for all modules).
 *
 * \param ptr Pointer to \p n_bytes of application-allocated memory. Can be
 *            NULL if malloc()/calloc() is to be used.
 *
 * \param n_bytes The number of bytes to allocate.
 *
 * \param flags Run-time flags. Valid flags are:
 *
 * - \ref RCSW_NOALLOC_META
 * - \ref RCSW_NOALLOC_DATA
 * - \ref RCSW_NOALLOC_HANDLE
 * - \ref RCSW_ZALLOC
 *
 */
void* rcsw_alloc(void* ptr, size_t n_bytes, uint32_t flags);

/**
 * \brief Free memory previously allocated with \ref rcsw_alloc().
 */
void rcsw_free(void* ptr, uint32_t flags);
