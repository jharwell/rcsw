/**
 * \file ds_test.c
 *
 * Common testing routines for the DS library.
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "tests/ds_test.h"
#include "rcsw/ds/binheap.h"
#include "rcsw/ds/bstree.h"
#include "rcsw/ds/darray.h"
#include "rcsw/ds/dyn_matrix.h"
#include "rcsw/ds/fifo.h"
#include "rcsw/ds/hashmap.h"
#include "rcsw/ds/inttree.h"
#include "rcsw/ds/llist.h"
#include "rcsw/ds/ostree.h"
#include "rcsw/ds/rbuffer.h"
#include "rcsw/ds/adj_matrix.h"
#include "rcsw/ds/matrix.h"
#include "rcsw/utils/utils.h"

#define RCSW_ER_MODNAME "rcsw.ds.test"
#define RCSW_ER_MODID M_TESTING
#include "rcsw/er/client.h"

/*******************************************************************************
 * Global Variables
 ******************************************************************************/
BEGIN_C_DECLS

int th_key_cmp(const void *a, const void *b) {
  if (a == NULL && b == NULL) {
    return 0;
  }
  if (a == NULL) {
    return -1;
  }
  if (b == NULL) {
    return 1;
  }
  return *(const int *)a - *(const int *)b; // strcmp(a, b);
} /* th_key_cmp() */

void th_printn(const void *node) {
  const struct hashnode *hashnode = (node);
  DPRINTF("node key: %s\nnode hash: 0x%08x\nnode data: 0x%08x\n",
         (const char *)hashnode->key, hashnode->hash, *(int *)hashnode->data);
} /* th_printn() */

int th_putchar(int c) { return c; }

END_C_DECLS
