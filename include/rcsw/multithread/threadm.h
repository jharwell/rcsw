/**
 * \file
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * \ingroup multithread
 *
 * \brief Various thread management tools
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <pthread.h>
#include <stddef.h>

#include "rcsw/core/compilers.h"
#include "rcsw/core/core.h"

/*******************************************************************************
 * Public API
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Lock a thread to a core.
 *
 * \param thread The thread handle.
 *
 * \param core The core to lock to, 0-indexed.
 *
 * \return \ref status_t.
 */
RCSW_API status_t threadm_core_lock(pthread_t thread, size_t core);

END_C_DECLS
