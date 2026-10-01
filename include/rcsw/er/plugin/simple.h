/**
 * \file
 *
 * \copyright 2023 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * \ingroup er
 *
 * \brief A simple C debugging/logging framework which only uses RCSW internals.
 *
 * Prints every compiled-in report, prefixed with its module name; there is one
 * compile-time level for the whole build and no per-module control. Mainly
 * intended for environments with no stdlib, such as bootstraps. Requires the
 * stdio component.
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/core/core.h"
#include "rcsw/er/er.h"
#include "rcsw/stdio/printf.h"  // IWYU pragma: export

/*******************************************************************************
 * RCSW ER Plugin Definitions
 ******************************************************************************/
/* \cond INTERNAL */
#define RCSW_ER_PLUGIN_PRINTF stdio_printf

/* not used for anything other than printing */
#define RCSW_ER_PLUGIN_MODNAME_COMPONENT_SEPARATOR "."

#define RCSW_ER_PLUGIN_INIT(...)
#define RCSW_ER_PLUGIN_DEINIT(...)

#define RCSW_ER_SIMPLE_FATAL_PRINT(NAME, MSG, ...)  \
  {                                                 \
    RCSW_ER_PLUGIN_PRINTF(NAME " %s[FATAL]%s " MSG, \
                          RCSW_ER_FAILC,            \
                          RCSW_ER_ENDC,             \
                          ##__VA_ARGS__);           \
  }

#define RCSW_ER_SIMPLE_ERROR_PRINT(NAME, MSG, ...)  \
  {                                                 \
    RCSW_ER_PLUGIN_PRINTF(NAME " %s[ERROR]%s " MSG, \
                          RCSW_ER_FAILC,            \
                          RCSW_ER_ENDC,             \
                          ##__VA_ARGS__);           \
  }

#define RCSW_ER_SIMPLE_WARN_PRINT(NAME, MSG, ...)               \
  {                                                             \
    RCSW_ER_PLUGIN_PRINTF(NAME " [WARN ] " MSG, ##__VA_ARGS__); \
  }

#define RCSW_ER_SIMPLE_INFO_PRINT(NAME, MSG, ...)               \
  {                                                             \
    RCSW_ER_PLUGIN_PRINTF(NAME " [INFO ] " MSG, ##__VA_ARGS__); \
  }

#define RCSW_ER_SIMPLE_DEBUG_PRINT(NAME, MSG, ...)              \
  {                                                             \
    RCSW_ER_PLUGIN_PRINTF(NAME " [DEBUG] " MSG, ##__VA_ARGS__); \
  }

#define RCSW_ER_SIMPLE_TRACE_PRINT(NAME, MSG, ...)              \
  {                                                             \
    RCSW_ER_PLUGIN_PRINTF(NAME " [TRACE] " MSG, ##__VA_ARGS__); \
  }

#define RCSW_ER_SIMPLE_PRINT_TRANSLATE(NAME, LVL, MSG, ...) \
  RCSW_JOIN3(RCSW_ER_SIMPLE_, LVL, _PRINT)(NAME, MSG, ##__VA_ARGS__)

#define RCSW_ER_PLUGIN_REPORT(LVL, LOGGER, ID, NAME, MSG, ...) \
  RCSW_ER_SIMPLE_PRINT_TRANSLATE(NAME, LVL, MSG, ##__VA_ARGS__)

#define RCSW_ER_PLUGIN_INSMOD(ID, NAME)

#define RCSW_ER_PLUGIN_HANDLE(ID, NAME)

#define RCSW_ER_PLUGIN_LVL_CHECK(HANDLE, LVL) \
  (RCSW_JOIN(RCSW_ERL_, LVL) <= RCSW_ERL)

/* \endcond */
