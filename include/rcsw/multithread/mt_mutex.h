/**
 * \file mt_mutex.h
 * \ingroup multithread
 * \brief Wrapper for pthread_mutex_lock.
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
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
