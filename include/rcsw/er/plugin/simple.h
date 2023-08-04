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
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "rcsw/rcsw.h"
#include "rcsw/stdio/printf.h"

/*******************************************************************************
 * RCSW ER Plugin Definitions
 ******************************************************************************/
/* \cond INTERNAL */
#define RCSW_ER_PLUGIN_PRINTF stdio_printf

/* not used for anything other than printing */
#define RCSW_ER_PLUGIN_MODNAME_COMPONENT_SEPARATOR "."

#define RCSW_ER_PLUGIN_INIT(...)
#define RCSW_ER_PLUGIN_DEINIT(...)

#define RCSW_ER_PLUGIN_REPORT(LVL, LOGGER,  ID, NAME, MSG, ...) \
  {                                                             \
    RCSW_ER_PLUGIN_PRINTF(NAME " [" RCSW_XSTR(LVL) "] "  MSG,   \
                          ## __VA_ARGS__);                      \
  }

#define RCSW_ER_PLUGIN_INSMOD(ID, NAME)

#define RCSW_ER_PLUGIN_HANDLE(ID, NAME)

#define RCSW_ER_PLUGIN_LVL_CHECK(HANDLE, LVL) true

/* \endcond */
