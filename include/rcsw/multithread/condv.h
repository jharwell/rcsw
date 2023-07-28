/**
 * \file condv.h
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
#include "rcsw/common/flags.h"
#include "rcsw/multithread/mutex.h"

/*******************************************************************************
 * Type Definitions
 ******************************************************************************/
/**
 * \brief Wrapper around condition variables from various implementations.
 *
 * Currently supports:
 *
 * - POSIX condition variables
 */
struct condv {
  pthread_cond_t impl;

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
 * Function Prototypes
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Initialize the signal condition.
 *
 * \param cv_in cv to initialize. Can be NULL if \ref RCSW_NOALLOC_HANDLE
 *              passed.
 *
 * \param flags Configuration flags.Valid flags are:
 *
 *              - \ref RCSW_NOALLOC_HANDLE
 *
 *              All other flags are ignored.
 *
 * \return The initialized signal condition, or NULL if an ERROR occurred.
 */
struct condv* condv_init(struct condv * cv_in, uint32_t flags);

/**
 * \brief Destroy the signal condition.
 *
 * \param cv The cv handle.
 */
void condv_destroy(struct condv *cv);

/**
 * \brief Signal on a condition variable.
 *
 * \param cv The cv handle.
 *
 * \return \ref status_t.
 */
status_t condv_signal(struct condv * cv);

/**
 * \brief Broadcast to everyone waiting on a condition variable.
 *
 * This function unblocks all threads currently blocked on the condition
 * variable. Each thread, upon its return from \ref condv_wait() or \ref
 * condv_timedwait() will own the mutex it entered its waiting function with.
 *
 * \param cv The cv handle.
 *
 * \return \ref status_t.
 */
status_t condv_broadcast(struct condv * cv);

/**
 * \brief Unconditional wait on a condition variable.
 *
 * \param cv The cv handle.
 * \param mtx The mutex the wait pairs with.
 *
 * \return \ref status_t.
 */
status_t condv_wait(struct condv * cv, struct mutex * mtx);

/**
 * \brief Timed wait on a condition variable.
 *
 * \param cv The cv handle.
 * \param mtx The mutex the wait pairs with.
 *
 * \param to A RELATIVE timeout, NOT an ABSOLUTE timeout, as the POSIX standard
 *           specifies. This function converts the relative timeout to absolute
 *           timeout required.
 *
 * \return \ref status_t.
 */
status_t condv_timedwait(struct condv* cv,
                        struct mutex* mtx,
                        const struct timespec * to);

END_C_DECLS
