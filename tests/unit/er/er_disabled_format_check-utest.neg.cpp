/**
 * \file
 *
 * \copyright 2026 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * Negative compile test: disabled reporting macros still check their format
 * strings. At ERL = NONE nothing is reported, but a format/argument mismatch
 * must still be diagnosed, so it cannot hide until reporting is turned on.
 *
 * Expected: compilation FAILS with a -Wformat diagnostic (promoted to an error
 * below) for the ER_ERR() call.
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#define LIBRA_ERL 0 /* RCSW_ERL_NONE */

#define RCSW_CONFIG_ER_PLUGIN 1 /* RCSW_ER_PLUGIN_SIMPLE */
#include "rcsw/er/er.h"
#include "rcsw/er/macros.h"
#include "rcsw/er/plugin/simple.h"

/*******************************************************************************
 * Trigger Condition
 ******************************************************************************/
#pragma GCC diagnostic error "-Wformat"

int main() {
  ER_ERR("expects a string: %s", 42);
  return 0;
}
