/**
 * \file mt_mutex.h
 * \ingroup multithread
 * \brief Wrapper for pthread_mutex_lock.
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

#ifndef INCLUDE_RCSW_MULTITHREAD_MT_MUTEX_H_
#define INCLUDE_RCSW_MULTITHREAD_MT_MUTEX_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <pthread.h>
#include "rcsw/common/common.h"
#include "rcsw/multithread/mt.h"

/*******************************************************************************
 * Type Definitions
 ******************************************************************************/
/**
 * \brief Wrapper around POSIX pthread mutexes variables. In the future, this
 * may incorporate mutexes from other operating systems.
 */
typedef struct {
    pthread_mutex_t mutex;
    uint32_t flags;
} mt_mutex_t;

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Initialize a mutex.
 *
 * \param mutex_in The mutex to initialize. Can be NULL if \ref
 * MT_APP_DOMAIN_MEM is not passed.
 * \param flags Configuration flags.
 *
 * \return The initialized mutex, or NULL if an ERROR occurred.
 */
mt_mutex_t* mt_mutex_init(mt_mutex_t *mutex_in, uint32_t flags);

/**
 * \brief Destroy a mutex.
 *
 * \param mutex The mutex handle.
 */
void mt_mutex_destroy(mt_mutex_t *mutex);

/**
 * \brief Acquire the lock.
 *
 * \param mutex The mutex handle.
 *
 * \return \ref status_t.
 */
status_t mt_mutex_lock(mt_mutex_t *mutex);

/**
 * \brief Release the lock.
 *
 * \param mutex The mutex handle.
 *
 * \return \ref status_t.
 */
status_t mt_mutex_unlock(mt_mutex_t *mutex);

END_C_DECLS

#endif /* INCLUDE_RCSW_MULTITHREAD_MT_MUTEX_H_ */
