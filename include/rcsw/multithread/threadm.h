/**
 * \file
 *
 * \copyright 2017 John Harwell
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
#include <pthread.h>  // NOLINT(misc-include-cleaner)
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
 * \return \ref status_t. ERROR with errno=EINVAL if \p core >= CPU_SETSIZE or
 * is not a core the thread may run on; otherwise errno is the error code from
 * pthread_setaffinity_np().
 */
/* NOLINTNEXTLINE(misc-include-cleaner) */
RCSW_API status_t threadm_core_lock(pthread_t thread, size_t core);

END_C_DECLS
