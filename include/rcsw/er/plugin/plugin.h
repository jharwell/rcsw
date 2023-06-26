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
#include "rcsw/er/plugin/dbg.h"
#elif RCSW_ER_PLUGIN == RCSW_ER_PLUGIN_LOG4CL
#include "rcsw/er/plugin/log4cl.h"
#elif RCSW_ER_PLUGIN == RCSW_ER_PLUGIN_LOG4C
#include "rcsw/er/plugin/log4c.h"
#elif RCSW_ER_PLUGIN == RCSW_ER_PLUGIN_CUSTOM
#error "Custom ER plugins not supported yet"
#endif

/*******************************************************************************
 * Constant Definitions
 ******************************************************************************/
#define RCSW_ER_OFF   RCSW_ER_PLUGIN_OFF
#define RCSW_ER_ERROR RCSW_ER_PLUGIN_ERROR
#define RCSW_ER_WARN  RCSW_ER_PLUGIN_WARN
#define RCSW_ER_INFO  RCSW_ER_PLUGIN_INFO
#define RCSW_ER_DEBUG RCSW_ER_PLUGIN_DEBUG
#define RCSW_ER_TRACE RCSW_ER_PLUGIN_TRACE

/*******************************************************************************
 * Macros
 ******************************************************************************/
#ifndef RCSW_ER_PLUGIN_PRINTF
#error "RCSW_ER_PLUGIN_PRINTF() not defined"
#endif

#define RCSW_ER_PRINTF RCSW_ER_PLUGIN_PRINTF

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
