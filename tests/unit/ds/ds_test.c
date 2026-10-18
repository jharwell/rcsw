/**
 * \file
 *
 * \copyright 2017 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * Common testing routines for the DS library.
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "tests/unit/ds/ds_test.h"

#include <stdio.h>

#include "rcsw/ds/hashmap.h"

/*******************************************************************************
 * Global Variables
 ******************************************************************************/
BEGIN_C_DECLS

int th_key_cmp(const void* a, const void* b) {
  if (a == NULL && b == NULL) {
    return 0;
  }
  if (a == NULL) {
    return -1;
  }
  if (b == NULL) {
    return 1;
  }
  return *(const int*)a - *(const int*)b;  // strcmp(a, b);
}

void th_printn(const void* node) {
  const struct hashnode* hashnode = (node);
  DPRINTF("node key: %s\nnode hash: 0x%08x\nnode data: 0x%08x\n",
          (const char*)hashnode->key,
          hashnode->hash,
          *(int*)hashnode->data);
}

/*
 * Default for test builds that route stdio through th_putchar
 * (RCSW_CONFIG_STDIO_PUTCHAR=th_putchar). It must really print: tests such as
 * minimon-utest capture stdout and check it. stdio-utest overrides it.
 */
int RCSW_WEAK th_putchar(int c) { return putchar(c); }

END_C_DECLS
