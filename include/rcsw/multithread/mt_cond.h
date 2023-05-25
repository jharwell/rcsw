/**
 * \file mt_cond.h
 * \ingroup multithread
 * \brief Wrapper for pthread_cond_t.
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
#include "rcsw/multithread/mt.h"
#include "rcsw/multithread/mt_mutex.h"
#include "rcsw/multithread/mt_cond.h"
#include "rcsw/rcsw.h"

/*******************************************************************************
 * Type Definitions
 ******************************************************************************/
/**
 * \brief Wrapper around POSIX pthread condition variables. In the future, this
 * may incorporate condition variables from other operating systems.
 */
typedef struct {
    pthread_cond_t cv;
    uint32_t flags;
} mt_cond_t;

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Initialize the signal condition.
 *
 * \param cv_in cv to initialize. Can be NULL if \ref MT_APP_DOMAIN_MEM passed
 * \param flags Configuration flags.
 *
 * \return The initialized signal condition, or NULL if an ERROR occurred.
 */
mt_cond_t* mt_cond_init(mt_cond_t * cv_in, uint32_t flags);

/**
 * \brief Destroy the signal condition.
 *
 * \param cv The cv handle.
 */
void mt_cond_destroy(mt_cond_t *cv);

/**
 * \brief Signal on a condition variable.
 *
 * \param cv The cv handle.
 *
 * \return \ref status_t.
 */
status_t mt_cond_signal(mt_cond_t * cv);

/**
 * \brief Broadcast to everyone waiting on a condition variable.
 *
 * This function unblocks all threads currently blocked on the condition
 * variable. Each thread, upon its return from cond_wait() or cond_timedwait()
 * will own the mutex it entered its waiting function with.
 *
 * \param cv The cv handle.
 *
 * \return \ref status_t.
 */
status_t mt_cond_broadcast(mt_cond_t * cv);

/**
 * \brief Unconditional wait on a condition variable.
 *
 * \param cv The cv handle.
 * \param mutex The mutex the wait pairs with.
 *
 * \return \ref status_t.
 */
status_t mt_cond_wait(mt_cond_t * cv, mt_mutex_t * mutex);

/**
 * \brief Timed wait on a condition variable.
 *
 * \param cv The cv handle.
 * \param mutex The mutex the wait pairs with.
 * \param to A RELATIVE timeout, NOT an ABSOLUTE timeout, as the POSIX standard
 * specifies. This function converts the relative timeout to absolute timeout
 * required.
 *
 * \return \ref status_t.
 */
status_t mt_cond_timedwait(mt_cond_t * cv, mt_mutex_t * mutex,
                           const struct timespec * to);

END_C_DECLS

