/**
 * \file test_er_plugin_missing_report.neg.cpp
 *
 * Negative compile test: RCSW_ER_PLUGIN_REPORT is mandatory.  Omitting it
 * while satisfying all other plugin requirements must trigger the #error guard
 * in plugin/plugin.h.
 *
 * Expected: compilation FAILS with:
 *   error: "RCSW_ER_PLUGIN_REPORT() not defined"
 */

#define RCSW_CONFIG_ER_PLUGIN 99

#include "rcsw/er/er.h"
/* Provide everything except RCSW_ER_PLUGIN_REPORT */
#define RCSW_ER_PLUGIN_PRINTF               printf
#define RCSW_ER_PLUGIN_INIT(...)
#define RCSW_ER_PLUGIN_DEINIT(...)
#define RCSW_ER_PLUGIN_INSMOD(...)
#define RCSW_ER_PLUGIN_HANDLE(...)
#define RCSW_ER_PLUGIN_LVL_CHECK(...)       true
#define RCSW_ER_PLUGIN_MODNAME_COMPONENT_SEPARATOR "."
/* RCSW_ER_PLUGIN_REPORT intentionally absent */

#include "rcsw/er/plugin/plugin.h"

int main() { return 0; }
