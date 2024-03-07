/**
 * \file simple.h
 * \ingroup er
 * \brief A simple C debugging/logging frameworkwhich only uses RCSW internals.
 *
 * Comprises debug printing on a module basis, with the capability to set the
 * level for each module independently (if you really want to). Mainly intended
 * for environments with no stdlib, such as bootstraps.
 *
 * \copyright 2023 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/rcsw.h"
#include "rcsw/stdio/printf.h"
#include "rcsw/er/er.h"

/*******************************************************************************
 * RCSW ER Plugin Definitions
 ******************************************************************************/
/* \cond INTERNAL */
#define RCSW_ER_PLUGIN_PRINTF stdio_printf

/* not used for anything other than printing */
#define RCSW_ER_PLUGIN_MODNAME_COMPONENT_SEPARATOR "."

#define RCSW_ER_PLUGIN_INIT(...)
#define RCSW_ER_PLUGIN_DEINIT(...)

#define RCSW_ER_SIMPLE_FATAL_PRINT(NAME,  MSG, ...)          \
  {                                                             \
  RCSW_ER_PLUGIN_PRINTF(NAME " %s[FATAL]%s "  MSG, \
                        RCSW_ER_FAILC,                          \
                        RCSW_ER_ENDC,                           \
                        ## __VA_ARGS__);                        \
  }

#define RCSW_ER_SIMPLE_ERROR_PRINT(NAME, MSG, ...)          \
  {                                                             \
  RCSW_ER_PLUGIN_PRINTF(NAME " %s[ERROR]%s "  MSG, \
                        RCSW_ER_FAILC,                          \
                        RCSW_ER_ENDC,                           \
                        ## __VA_ARGS__);                        \
  }

#define RCSW_ER_SIMPLE_WARN_PRINT(NAME, MSG, ...)           \
  {                                                             \
  RCSW_ER_PLUGIN_PRINTF(NAME " [WARN ] "  MSG, \
                        ## __VA_ARGS__);                        \
  }

#define RCSW_ER_SIMPLE_INFO_PRINT(NAME, MSG, ...)           \
  {                                                             \
  RCSW_ER_PLUGIN_PRINTF(NAME " [INFO ] "  MSG, \
                        ## __VA_ARGS__);                        \
  }

#define RCSW_ER_SIMPLE_DEBUG_PRINT(NAME, MSG, ...)          \
  {                                                             \
  RCSW_ER_PLUGIN_PRINTF(NAME " [DEBUG] "  MSG, \
                        ## __VA_ARGS__);                        \
  }

#define RCSW_ER_SIMPLE_TRACE_PRINT(NAME, MSG, ...)          \
  {                                                             \
  RCSW_ER_PLUGIN_PRINTF(NAME " [TRACE] "  MSG, \
                        ## __VA_ARGS__);                        \
  }


#define RCSW_ER_SIMPLE_PRINT_TRANSLATE(NAME, LVL, MSG, ...)      \
  RCSW_JOIN3(RCSW_ER_SIMPLE_, LVL, _PRINT)(NAME, MSG, ## __VA_ARGS__)

#define RCSW_ER_PLUGIN_REPORT(LVL, LOGGER,  ID, NAME, MSG, ...) \
  RCSW_ER_SIMPLE_PRINT_TRANSLATE(NAME, LVL, MSG, ## __VA_ARGS__)

#define RCSW_ER_PLUGIN_INSMOD(ID, NAME)

#define RCSW_ER_PLUGIN_HANDLE(ID, NAME)

#define RCSW_ER_PLUGIN_LVL_CHECK(HANDLE, LVL) true

/* \endcond */
