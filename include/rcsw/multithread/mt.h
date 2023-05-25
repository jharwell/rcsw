/**
 * \file mt.h
 * \ingroup multithread
 * \brief Common multithread definitions.
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/

/*******************************************************************************
 * Constant Definitions
 ******************************************************************************/
/**
 * Passing this flag will cause  multithreaded locks/semaphores, etc. to
 * malloc() for their memory, as opposed to requiring the application to provide
 * it.
 */
#define MT_APP_DOMAIN_MEM 0x1

