/**
 * \file plugin.h
 *
 * \copyright 2023 John Harwell, All rights reserved.
 *
 * SPDX-License Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/er/er.h"

#if RCSW_ER_PLUGIN == RCSW_ER_PLUGIN_SIMPLE
#include "rcsw/er/plugin/simple.h"
#elif RCSW_ER_PLUGIN == RCSW_ER_PLUGIN_LOG4CL
#include "rcsw/er/plugin/log4cl.h"
#elif RCSW_ER_PLUGIN == RCSW_ER_PLUGIN_LOG4C
#include "rcsw/er/plugin/log4c.h"
#else
/* assume a custom ER plugin */
#endif

/*******************************************************************************
 * Macros
 ******************************************************************************/

#ifndef RCSW_ER_PLUGIN_PRINTF
#error "RCSW_ER_PLUGIN_PRINTF() not defined"
#endif

#ifndef RCSW_ER_PLUGIN_INIT
#error "RCSW_ER_PLUGIN_INIT() not defined"
#endif

#ifndef RCSW_ER_PLUGIN_DEINIT
#error "RCSW_ER_PLUGIN_DEINIT() not defined"
#endif

#ifndef RCSW_ER_PLUGIN_REPORT
#error "RCSW_ER_PLUGIN_REPORT() not defined"
#endif

#ifndef RCSW_ER_PLUGIN_INSMOD
#error "RCSW_ER_PLUGIN_INSMOD() not defined"
#endif

#ifndef RCSW_ER_PLUGIN_HANDLE
#error "RCSW_ER_PLUGIN_HANDLE() not defined"
#endif

#ifndef RCSW_ER_PLUGIN_LVL_CHECK
#error "RCSW_ER_PLUGIN_LVL_CHECK() not defined"
#endif
