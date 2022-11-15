/**
 * \file ds_test.c
 *
 * Common testing routines for the DS library.
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * This file is part of RCSW.
 *
 * RCSW is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * RCSW is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * RCSW.  If not, see <http://www.gnu.org/licenses/
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "ds_test.h"
#include "rcsw/common/dbg.h"
#include "rcsw/ds/bin_heap.h"
#include "rcsw/ds/bstree.h"
#include "rcsw/ds/darray.h"
#include "rcsw/ds/dynamic_matrix.h"
#include "rcsw/ds/hashmap.h"
#include "rcsw/ds/int_tree.h"
#include "rcsw/ds/llist.h"
#include "rcsw/ds/ostree.h"
#include "rcsw/ds/rbuffer.h"
#include "rcsw/ds/static_adj_matrix.h"
#include "rcsw/ds/static_matrix.h"
#include "rcsw/utils/utils.h"

/*******************************************************************************
 * Constant Definitions
 ******************************************************************************/
#define MODULE_ID M_DS_CORE

/*******************************************************************************
 * Global Variables
 ******************************************************************************/
BEGIN_C_DECLS

status_t th_ds_init(struct ds_params *const params) {
  dbg_init();
  dbg_insmod(M_TESTING, "Testing");

  /* finish initializing parameter struct */
  params->elements = NULL;
  params->nodes = NULL;
  switch (params->tag) {
  case DS_DARRAY:
    /* *2 is to allow the splice tests succeed without segfault */
    params->elements =
        malloc(darray_element_space(NUM_ITEMS * 2, sizeof(struct element)));
    params->max_elts = NUM_ITEMS * 2;
    break;
  case DS_RBUFFER:
    params->elements =
        malloc(rbuffer_element_space(sizeof(struct element), NUM_ITEMS));
    break;
  case DS_LLIST:
    /*  *2 is to allow the splice_tests() succeed without segfault */
    params->max_elts = NUM_ITEMS * 2;
    params->nodes = malloc(llist_node_space(NUM_ITEMS * 2));
    params->elements =
        malloc(llist_element_space(NUM_ITEMS * 2, sizeof(struct element)));
    RCSW_CHECK_PTR(params->nodes);
    break;
  case DS_BIN_HEAP:
    params->elements =
        malloc(bin_heap_element_space(NUM_ITEMS, sizeof(struct element)));
    memset(params->elements, 0,
           bin_heap_element_space(NUM_ITEMS, sizeof(struct element)));
    break;
  case DS_ADJ_MATRIX:
    /* Just do weighted all the time--need the space.... */
    params->elements = malloc(static_adj_matrix_space(NUM_ITEMS, TRUE));
    memset(params->elements, 0, static_adj_matrix_space(NUM_ITEMS, TRUE));
    break;
  case DS_HASHMAP:
    params->elements = malloc(
        hashmap_element_space(NUM_ITEMS * NUM_ITEMS, sizeof(struct element)));
    params->nodes = malloc(hashmap_node_space(NUM_ITEMS));
    RCSW_CHECK_PTR(params->nodes);
    break;
  case DS_BSTREE:
    if (params->flags & DS_BSTREE_OS) {
      params->nodes = malloc(OSTREE_CALC_NODE_SPACE(NUM_ITEMS));
      params->elements =
          malloc(OSTREE_CALC_ELEMENT_SPACE(NUM_ITEMS, sizeof(struct element)));
      RCSW_CHECK_PTR(params->nodes);
    } else {
      params->nodes = malloc(bstree_node_space(NUM_ITEMS));
      params->elements =
          malloc(bstree_element_space(NUM_ITEMS, sizeof(struct element)));
      RCSW_CHECK_PTR(params->nodes);
    }
    break;
  case DS_STATIC_MATRIX:
    params->elements = malloc(static_matrix_space(params->type.smat.n_rows,
                                                  params->type.smat.n_cols,
                                                  sizeof(struct element)));
    break;
  case DS_DYNAMIC_MATRIX:
    params->elements = malloc(dynamic_matrix_space(params->type.dmat.n_rows,
                                                   params->type.dmat.n_cols,
                                                   sizeof(struct element)));
    break;
  default:
    DBGE("ERROR: No tag defined.\n");
    break;
  } /* switch() */

  RCSW_CHECK_PTR(params->elements);
  return OK;

error:
  return ERROR;
} /* th_ds_init() */

void th_ds_shutdown(const struct ds_params *const params) {
  if (params->elements) {
    free(params->elements);
  }
  if (params->nodes) {
    free(params->nodes);
  }
} /* th_ds_shutdown() */

int th_leak_check_data(const struct ds_params *params) {
  int i;
  int len;
  if (params->tag == DS_BSTREE) {
    len = params->max_elts;
  } else if (params->tag == DS_HASHMAP) {
    len = params->type.hm.bsize * params->type.hm.n_buckets;
  } else {
    len = params->max_elts;
  }
  if (params->flags & DS_APP_DOMAIN_DATA) {
    for (i = 0; i < len; ++i) {
      SOFT_ASSERT(((int *)(params->elements))[i] == -1,
                  "ERROR: Memory leak at index %d in data block area\n", i);
    } /* for() */
  }
  return 0;

error:
  return 1;
} /* th_leak_check_data() */

int th_leak_check_nodes(const struct ds_params *params) {
  int i;
  int len;
  if (params->tag == DS_BSTREE) {
    len = params->max_elts;
  } else if (params->tag == DS_HASHMAP) {
    len = params->type.hm.bsize * params->type.hm.n_buckets;
  } else {
    len = params->max_elts;
  }
  /* It's not valid to check for leaks in this case, because you are sharing
   * things between two or more lists
   */
  if (params->flags & DS_LLIST_NO_DB) {
    return 0;
  }
  if (params->flags & DS_APP_DOMAIN_NODES) {
    for (i = 0; i < len; ++i) {
      SOFT_ASSERT(((int *)(params->nodes))[i] == -1,
                  "ERROR: Memory leak at index %d in node area\n", i);
    }
  }
  return 0;

error:
  return 1;
} /* th_leak_check_nodes() */

bool_t th_filter_func(const void *const e) {
  const struct element *q = e;
  return (q->value1 % 2 == 0);
} /* th_filter_func() */

int th_key_cmp(const void *a, const void *b) {
  if (a == NULL && b == NULL)
    return 0;
  if (a == NULL)
    return -1;
  if (b == NULL)
    return 1;
  return *(const int *)a - *(const int *)b; // strcmp(a, b);
} /* th_key_cmp() */

void th_printe(const void *e) {
  const struct element *el = e;
  PRINTF("%d\n", el->value1);
  PRINTF("%d\n", el->value2);
} /* th_printe() */

int th_cmpe(const void *const e1, const void *const e2) {
  const struct element *q1 = (const struct element *)e1;
  const struct element *q2 = (const struct element *)e2;

  if (q1->value1 < q2->value1) {
    return -1;
  } else if (q1->value1 == q2->value1) {
    return 0;
  }
  return 1;
} /* th_cmpe() */

void th_printn(const void *node) {
  const struct hashnode *hashnode = node;
  PRINTF("node key: %s\nnode hash: 0x%08x\nnode data: 0x%08x\n",
         (const char *)hashnode->key, hashnode->hash, *(int *)hashnode->data);
} /* th_printn() */

int th_data_cmp(const void *a, const void *b) {
  const struct hashnode *q1 = a;
  const struct hashnode *q2 = b;
  return *(int *)q1->data - *(int *)q2->data;
} /* th_data_cmp() */

void th_inject_func(void *e, void *res) {
  struct element *e1 = e;
  int *tmp = res;
  *tmp += e1->value1;
} /* th_inject_func() */

void th_map_func(void *e) {
  struct element *e1 = e;
  e1->value1--;
} /* th_map_func() */

bool_t th_iter_func(void *e) {
  return (((struct element *)e)->value1 % 2 == 0);
} /* th_iter_func() */

END_C_DECLS
