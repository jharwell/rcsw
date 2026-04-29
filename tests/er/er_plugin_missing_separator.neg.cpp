/**
 * \file er_plugin_missing_separator.neg.cpp
 *
 * Negative compile test: plugin/plugin.h must emit a hard #error when
 * RCSW_ER_PLUGIN_MODNAME_COMPONENT_SEPARATOR is absent.
 *
 * Expected compiler error (from plugin/plugin.h):
 *   error: "RCSW_ER_PLUGIN_MODNAME_COMPONENT_SEPARATOR not defined"
 *
 * \copyright 2023 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Trigger Condition
 ******************************************************************************/
#define RCSW_CONFIG_ER_PLUGIN 99

#include "rcsw/er/er.h"

#define RCSW_ER_PLUGIN_PRINTF               printf
#define RCSW_ER_PLUGIN_INIT(...)
#define RCSW_ER_PLUGIN_DEINIT(...)
#define RCSW_ER_PLUGIN_REPORT(...)
#define RCSW_ER_PLUGIN_INSMOD(...)
#define RCSW_ER_PLUGIN_HANDLE(...)
#define RCSW_ER_PLUGIN_LVL_CHECK(...) true
/* RCSW_ER_PLUGIN_MODNAME_COMPONENT_SEPARATOR intentionally absent */

#include "rcsw/er/plugin/plugin.h"

int main() { return 0; }
