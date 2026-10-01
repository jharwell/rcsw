/**
 * \file
 *
 * \copyright 2026 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * Common bits algorithm tests.
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <string.h>

#include "rcsw/al/types.h"

/*******************************************************************************
 * Test Harness
 ******************************************************************************/
namespace th {

struct big_elt {
  int key;
  char pad[76]; /* 80 bytes total: above the internal 64-byte swap buffer */
};

int cmp_first_int(const void *const a, const void *const b);

bool_t eq_int(const void *a, const void *b);

size_t len_int(const void *s);

} /* namespace th */
