/**
 * \file er_plugin_missing_printf.neg.cpp
 *
 * Negative compile test: plugin/plugin.h must emit a hard #error when
 * RCSW_ER_PLUGIN_PRINTF is absent.
 *
 * Expected compiler error (from plugin/plugin.h):
 *   error: "RCSW_ER_PLUGIN_PRINTF() not defined"
 *
 * \copyright 2023 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Trigger Condition
 *
 * Select a nonexistent plugin id so none of the built-in plugin headers are
 * pulled in.  Provide every required symbol manually except the one under
 * test, then include plugin.h to trip its guard.
 ******************************************************************************/
#define RCSW_CONFIG_ER_PLUGIN 99

#include "rcsw/er/er.h"

#define RCSW_ER_PLUGIN_INIT(...)
#define RCSW_ER_PLUGIN_DEINIT(...)
#define RCSW_ER_PLUGIN_REPORT(...)
#define RCSW_ER_PLUGIN_INSMOD(...)
#define RCSW_ER_PLUGIN_HANDLE(...)
#define RCSW_ER_PLUGIN_LVL_CHECK(...) true
#define RCSW_ER_PLUGIN_MODNAME_COMPONENT_SEPARATOR "."
/* RCSW_ER_PLUGIN_PRINTF intentionally absent */

#include "rcsw/er/plugin/plugin.h"

int main() { return 0; }
