/**
 * \file test_er_plugin_missing_printf.neg.cpp
 *
 * Negative compile test: plugin/plugin.h enforces that every required plugin
 * symbol is defined.  If RCSW_ER_PLUGIN_PRINTF is absent the header must
 * emit a hard #error.
 *
 * Expected: compilation FAILS with:
 *   error: "RCSW_ER_PLUGIN_PRINTF() not defined"
 *
 * We trick the system by selecting an unknown plugin value so none of the
 * built-in plugin headers are included, leaving the symbol undefined.
 */

/* Select a nonexistent plugin so no built-in header provides the symbols */
#define RCSW_CONFIG_ER_PLUGIN 99

#include "rcsw/er/er.h"
/* Provide every OTHER required symbol manually… */
#define RCSW_ER_PLUGIN_INIT(...)
#define RCSW_ER_PLUGIN_DEINIT(...)
#define RCSW_ER_PLUGIN_REPORT(...)
#define RCSW_ER_PLUGIN_INSMOD(...)
#define RCSW_ER_PLUGIN_HANDLE(...)
#define RCSW_ER_PLUGIN_LVL_CHECK(...)  true
#define RCSW_ER_PLUGIN_MODNAME_COMPONENT_SEPARATOR "."
/* … but deliberately omit RCSW_ER_PLUGIN_PRINTF */

#include "rcsw/er/plugin/plugin.h"

int main() { return 0; }
