/**
 * \file er_inherit_without_erl.neg.cpp
 *
 * Negative compile test: LIBRA_ERL_INHERIT without LIBRA_ERL must be a
 * hard error.
 *
 * Expected compiler error (from er.h):
 *   error: LIBRA_ERL_INHERIT defined but LIBRA_ERL not defined!
 *
 * \copyright 2023 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Trigger Condition
 ******************************************************************************/

/* Define INHERIT but deliberately omit LIBRA_ERL. */
#define LIBRA_ERL_INHERIT

#include "rcsw/er/er.h"

int main() { return 0; }
