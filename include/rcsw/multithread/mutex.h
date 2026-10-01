/**
 * \file
 *
 * \copyright 2017 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * \ingroup multithread
 *
 * \brief Mutex.
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <pthread.h>  // NOLINT(misc-include-cleaner)
#include <stdint.h>

#include "rcsw/core/compilers.h"
#include "rcsw/core/core.h"

/*******************************************************************************
 * Type Definitions
 ******************************************************************************/
/**
 * \brief Wrapper around mutexes from various implementations.
 *
 * Currently supports:
 *
 * - POSIX mutexes
 */
struct mutex {
  pthread_mutex_t impl;  // NOLINT(misc-include-cleaner)

  /**
   * Valid flags are:
   *
   * - \ref RCSW_ZALLOC
   * - \ref RCSW_NOALLOC_HANDLE
   *
   * All other flags are ignored.
   */
  uint32_t flags;
};

/*******************************************************************************
 * Public API
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Initialize a mutex.
 *
 * \param mutex_in Caller storage for the handle, used only if \ref
 *                 RCSW_NOALLOC_HANDLE is passed; ignored (may be NULL)
 *                 otherwise. See \rcswdoc{concepts/memory-model}.
 *
 * \param flags Configuration flags. See \ref mutex.flags for valid flags.
 *
 * \return The initialized mutex, or NULL if an ERROR occurred.
 */
RCSW_API struct mutex* mutex_init(struct mutex* mutex_in, uint32_t flags);

/**
 * \brief Destroy a mutex.
 *
 * \param mutex The mutex handle.
 */
RCSW_API void mutex_destroy(struct mutex* mutex);

/**
 * \brief Acquire the lock.
 *
 * \param mutex The mutex handle.
 *
 * \return \ref status_t.
 */
RCSW_API status_t mutex_lock(struct mutex* mutex);

/**
 * \brief Release the lock.
 *
 * \param mutex The mutex handle.
 *
 * \return \ref status_t.
 */
RCSW_API status_t mutex_unlock(struct mutex* mutex);

END_C_DECLS
