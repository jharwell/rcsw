/**
 * \file
 *
 * \copyright 2023 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * Negative compile test: defining LIBRA_ERL_INHERIT without first defining
 * LIBRA_ERL must produce a hard #error (see er.h).
 *
 * Expected: compilation FAILS with:
 *   error: LIBRA_ERL_INHERIT defined but LIBRA_ERL not defined!
 */

/* Define INHERIT but intentionally omit LIBRA_ERL */
#undef LIBRA_ERL
#define LIBRA_ERL_INHERIT

#include "rcsw/er/er.h"

int main() { return 0; }
