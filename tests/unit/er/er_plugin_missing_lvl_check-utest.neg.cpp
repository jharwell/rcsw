/**
 * \file
 *
 * \copyright 2023 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * Negative compile test: RCSW_ER_PLUGIN_LVL_CHECK is mandatory.
 *
 * Expected: compilation FAILS with:
 *   error: "RCSW_ER_PLUGIN_LVL_CHECK() not defined"
 */

#define RCSW_CONFIG_ER_PLUGIN 99

#include "rcsw/er/er.h"
#define RCSW_ER_PLUGIN_PRINTF printf
#define RCSW_ER_PLUGIN_INIT(...)
#define RCSW_ER_PLUGIN_DEINIT(...)
#define RCSW_ER_PLUGIN_REPORT(...)
#define RCSW_ER_PLUGIN_INSMOD(...)
#define RCSW_ER_PLUGIN_HANDLE(...)
#define RCSW_ER_PLUGIN_MODNAME_COMPONENT_SEPARATOR "."
/* RCSW_ER_PLUGIN_LVL_CHECK intentionally absent */

#include "rcsw/er/plugin/plugin.h"

int main() { return 0; }
