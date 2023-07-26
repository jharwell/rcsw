/**
 * \file mutex.h
 * \ingroup multithread
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <pthread.h>
#include "rcsw/rcsw.h"

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
  pthread_mutex_t impl;

  /**
   * Valid flags are:
   *
   * - \ref RCSW_NOALLOC_HANDLE
   *
   * All other flags are ignored.
   */
  uint32_t flags;
};

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Initialize a mutex.
 *
 * \param mutex_in The mutex to initialize. Can be NULL if \ref
 *                 RCSW_NOALLOC_HANDLE is not passed.
 *
 * \param flags Configuration flags.
 *
 * \return The initialized mutex, or NULL if an ERROR occurred.
 */
struct mutex* mutex_init(struct mutex *mutex_in, uint32_t flags);

/**
 * \brief Destroy a mutex.
 *
 * \param mutex The mutex handle.
 */
void mutex_destroy(struct mutex *mutex);

/**
 * \brief Acquire the lock.
 *
 * \param mutex The mutex handle.
 *
 * \return \ref status_t.
 */
status_t mutex_lock(struct mutex *mutex);

/**
 * \brief Release the lock.
 *
 * \param mutex The mutex handle.
 *
 * \return \ref status_t.
 */
status_t mutex_unlock(struct mutex *mutex);

END_C_DECLS
