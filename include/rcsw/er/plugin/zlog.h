/**
 * \file zlog.h
 * \ingroup er
 * \brief Adaptor for using ZLOG with rcsw.
 *
 * \copyright 2023 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <stdio.h>

#include "rcsw/rcsw.h"

/* 2023-11-14 [JRH]: zlog does not come ready to interoperate with C++ :-(. */
BEGIN_C_DECLS

RCSW_WARNING_DISABLE_PUSH()
RCSW_WARNING_DISABLE_REDUNDANT_DECLS()
#include <zlog.h>
RCSW_WARNING_DISABLE_POP()

END_C_DECLS

/*******************************************************************************
 * RCSW ER Plugin Definitions
 ******************************************************************************/
/* \cond INTERNAL */

/*
 * 2023-11-14 [JRH]: zlog doesn't come with a TRACE level, so we add one as
 * documented in its developer docs.
 */
enum {
  ZLOG_LEVEL_TRACE = 10
};

#define zlog_trace(cat, format, ...)                    \
  zlog(cat, __FILE__, sizeof(__FILE__)-1,               \
       __func__, sizeof(__func__)-1, __LINE__,          \
       ZLOG_LEVEL_TRACE, format, ## __VA_ARGS__)

#define zlog_trace_enabled(zc) zlog_level_enabled(zc, ZLOG_LEVEL_TRACE)

#define RCSW_ER_PLUGIN_MODNAME_COMPONENT_SEPARATOR "_"

#define RCSW_ER_PLUGIN_PRINTF printf

#define RCSW_ER_ZLOG_FATAL fatal
#define RCSW_ER_ZLOG_ERROR error
#define RCSW_ER_ZLOG_WARN  warn
#define RCSW_ER_ZLOG_INFO  info
#define RCSW_ER_ZLOG_DEBUG debug
#define RCSW_ER_ZLOG_TRACE trace

#define RCSW_ER_ZLOG_LVL_TRANSLATE(LVL) RCSW_JOIN(RCSW_ER_ZLOG_,LVL)

#define RCSW_ER_PLUGIN_LVL_CHECK(HANDLE, LVL)                           \
  RCSW_JOIN3(zlog_,                                                     \
             RCSW_ER_ZLOG_LVL_TRANSLATE(LVL),                           \
             _enabled)(HANDLE)

#define RCSW_ER_PLUGIN_INIT(...) zlog_init(__VA_ARGS__)

#define RCSW_ER_PLUGIN_DEINIT(...) zlog_fini()

#define RCSW_ER_PLUGIN_REPORT(LVL, HANDLE,  ID, NAME, MSG, ...) \
  {                                                             \
    RCSW_JOIN(zlog_, RCSW_ER_ZLOG_LVL_TRANSLATE(LVL))(HANDLE, MSG, ## __VA_ARGS__); \
  }

/*
 * Not needed--when retrieving the handle, the module will be created if it
 * doesn't exist.
 */
#define RCSW_ER_PLUGIN_INSMOD(ID, NAME)

#define RCSW_ER_PLUGIN_HANDLE(ID, NAME) zlog_get_category(NAME)
