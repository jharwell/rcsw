/**
 * \file mt_cvm.h
 * \ingroup multithread
 * \brief Conditional variable-mutex pair, (since they are frequently
 * used together).
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

#ifndef INCLUDE_RCSW_MULTITHREAD_MT_CVM_H_
#define INCLUDE_RCSW_MULTITHREAD_MT_CVM_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <pthread.h>
#include "rcsw/multithread/mt.h"
#include "rcsw/multithread/mt_mutex.h"
#include "rcsw/multithread/mt_cond.h"
#include "rcsw/common/common.h"

/*******************************************************************************
 * Type Definitions
 ******************************************************************************/
/**
 * \brief Pairing of condition variables and a corresponding mutex, because they
 * are frequently used together.
 */
typedef struct {
    mt_mutex_t mutex;
    mt_cond_t cv;
    uint32_t flags;
} mt_cvm_t;

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Initialize the signal condition (variable and mutex).
 *
 * \param cvm_in CVM to initialize. Can be NULL if \ref MT_APP_DOMAIN_MEM passed.
 * \param flags Configuration flags.
 *
 * \return The initialized CVM, or NULL if an ERROR occurred.
 */
mt_cvm_t* cvm_init(mt_cvm_t * cvm_in, uint32_t flags);

/**
 * \brief Destroy the signal condition.
 *
 * \param cvm The CVM handle.
 */
void mt_cvm_destroy(mt_cvm_t * cvm);

/**
 * \brief Signal on a condition variable while holding a mutex.
 *
 * \param cvm The CVM handle.
 *
 * \return \ref status_t.
 */
status_t cvm_signal(mt_cvm_t * cvm);

/**
 * \brief Broadcast to everyone waiting on a condition variable.
 *
 * This function unblocks all threads currently blocked on the condition
 * variable. Each thread, upon its return from \ref cvm_wait() or \ref
 * cvm_timedwait() will own the mutex it entered its waiting function with.
 *
 * \param cvm The CVM handle.
 *
 * \return \ref status_t.
 */
status_t cvm_broadcast(mt_cvm_t * cvm);

/**
 * \brief  Wait on a condition variable while holding a mutex.
 *
 * \param cvm The CVM handle.
 *
 * \return \ref status_t.
 */
status_t cvm_wait(mt_cvm_t * cvm);

/**
 * \brief Timed wait on a condition variable while holding a mutex.
 *
 * \param cvm The CVM handle.
 * \param to  A RELATIVE timeout, NOT an ABSOLUTE timeout, as the POSIX standard
 * specifies. This function converts the relative timeout to absolute timeout
 * required.
 *
 * \return \ref status_t
 */
status_t cvm_timedwait(mt_cvm_t * cvm, const struct timespec * to);

END_C_DECLS

#endif /* INCLUDE_RCSW_MULTITHREAD_MT_CVM_H_ */
