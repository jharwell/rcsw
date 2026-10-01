/**
 * \file
 *
 * \copyright 2023 John Harwell
 *
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>

#include "rcsw/core/compilers.h"
#include "rcsw/stdio/printf.h"

BEGIN_C_DECLS
/* NOLINTNEXTLINE(readability-identifier-naming) */
void putchar_(char c) { (void)putchar(c); }
END_C_DECLS
