/**
 * \file
 *
 * \copyright 2017 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * \ingroup multithread
 *
 * \brief Condition variable.
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <pthread.h>  // NOLINT(misc-include-cleaner)
#include <time.h>

#include "rcsw/core/compilers.h"
#include "rcsw/core/core.h"
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
  pthread_cond_t impl;  // NOLINT(misc-include-cleaner)

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
 * \param cv_in Caller storage for the handle, used only if \ref
 *              RCSW_NOALLOC_HANDLE is passed; ignored (may be NULL)
 *              otherwise. See \rcswdoc{concepts/memory-model}.
 *
 * \param flags Configuration flags. See \ref condv.flags for valid flags.
 *
 * All other flags are ignored.
 *
 * \return The initialized signal condition, or NULL if an ERROR occurred.
 */
RCSW_API struct condv* condv_init(struct condv* cv_in, uint32_t flags);

/**
 * \brief Destroy the signal condition.
 *
 * \param cv The cv handle.
 */
RCSW_API void condv_destroy(struct condv* cv);

/**
 * \brief Signal on a condition variable.
 *
 * \param cv The cv handle.
 *
 * \return \ref status_t.
 */
RCSW_API status_t condv_signal(struct condv* cv);

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
RCSW_API status_t condv_broadcast(struct condv* cv);

/**
 * \brief Unconditional wait on a condition variable.
 *
 * \param cv The cv handle.
 * \param mtx The mutex the wait pairs with.
 *
 * \return \ref status_t.
 */
RCSW_API status_t condv_wait(struct condv* cv, struct mutex* mtx);

/**
 * \brief Timed wait on a condition variable.
 *
 * \param cv The cv handle.
 * \param mtx The mutex the wait pairs with.
 *
 * \param to A relative timeout. See
 *           \rcswdoc{concepts/concurrency/timeouts}.
 *
 * \return \ref status_t.
 */
RCSW_API status_t condv_timedwait(struct condv*          cv,
                                  struct mutex*          mtx,
                                  const struct timespec* to);

/**
 * \brief Timed wait on a condition variable, with an absolute deadline.
 *
 * For callers that wait in a loop (to handle spurious wakeups): a relative
 * timeout restarted on every iteration would never expire.
 *
 * \param cv The cv handle.
 * \param mtx The mutex the wait pairs with.
 * \param deadline An absolute \c CLOCK_REALTIME deadline. See
 *                 \rcswdoc{concepts/concurrency/timeouts}.
 *
 * \return \ref status_t. On timeout, errno is ETIMEDOUT.
 */
RCSW_API status_t condv_timedwait_abs(struct condv*          cv,
                                      struct mutex*          mtx,
                                      const struct timespec* deadline);

END_C_DECLS
