/**
 * \file threadm.h
 * \ingroup multithread
 * \brief Various thread management tools
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/common/common.h"

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Lock a thread to a core.
 *
 * \param thread The thread handle.
 * \param core The core to lock to, 0-indexed.
 *
 * \return \ref status_t.
 */
status_t threadm_core_lock(pthread_t thread, size_t core);

END_C_DECLS

