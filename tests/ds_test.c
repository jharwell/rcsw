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
#include "rcsw/ds/bin_heap.h"
#include "rcsw/ds/bstree.h"
#include "rcsw/ds/darray.h"
#include "rcsw/ds/dynamic_matrix.h"
#include "rcsw/ds/fifo.h"
#include "rcsw/ds/hashmap.h"
#include "rcsw/ds/inttree.h"
#include "rcsw/ds/llist.h"
#include "rcsw/ds/ostree.h"
#include "rcsw/ds/rbuffer.h"
#include "rcsw/ds/static_adj_matrix.h"
#include "rcsw/ds/static_matrix.h"
#include "rcsw/utils/utils.h"

#define RCSW_ER_MODNAME "rcsw.ds.test"
#define RCSW_ER_MODID M_TESTING
#include "rcsw/er/client.h"

/*******************************************************************************
 * Global Variables
 ******************************************************************************/
BEGIN_C_DECLS

status_t th_ds_init(struct ds_params *const params) {
  RCSW_ER_MODULE_INIT();

  /* finish initializing parameter struct */
  params->elements = NULL;
  params->nodes = NULL;
  switch (params->tag) {
  case ekRCSW_DS_DARRAY:
    /* *2 is to allow the splice tests succeed without segfault */
    params->elements = (malloc(darray_element_space(TH_NUM_ITEMS * 2, params->elt_size)));
    params->max_elts = TH_NUM_ITEMS * 2;
    break;
  case ekRCSW_DS_RBUFFER:
    params->elements =
        (malloc(rbuffer_element_space(params->elt_size, TH_NUM_ITEMS)));
    break;
  case ekRCSW_DS_FIFO:
    params->elements =
        (malloc(fifo_element_space(params->elt_size, TH_NUM_ITEMS)));
    break;
  case ekRCSW_DS_LLIST:
    /*  *2 is to allow the splice_tests() succeed without segfault */
    params->max_elts = TH_NUM_ITEMS * 2;
    params->nodes = (malloc(llist_node_space(TH_NUM_ITEMS * 2)));
    params->elements =
        (malloc(llist_element_space(TH_NUM_ITEMS * 2, params->elt_size)));
    RCSW_CHECK_PTR(params->nodes);
    break;
  case ekRCSW_DS_BIN_HEAP:
    params->elements =
        (malloc(bin_heap_element_space(TH_NUM_ITEMS, params->elt_size)));
    memset(params->elements, 0,
           bin_heap_element_space(TH_NUM_ITEMS, params->elt_size));
    break;
  case ekRCSW_DS_ADJ_MATRIX:
    /* Just do weighted all the time--need the space.... */
    params->elements = (malloc(static_adj_matrix_space(TH_NUM_ITEMS, TRUE)));
    memset(params->elements, 0, static_adj_matrix_space(TH_NUM_ITEMS, TRUE));
    break;
  case ekRCSW_DS_HASHMAP:
    params->nodes = (malloc(hashmap_node_space(TH_NUM_BUCKETS)));
    params->elements = (malloc(hashmap_element_space(TH_NUM_BUCKETS,
                                                     TH_NUM_ITEMS * TH_NUM_ITEMS,
                                                     params->elt_size)));
    RCSW_CHECK_PTR(params->nodes);
    break;
  case ekRCSW_DS_BSTREE:
    if (params->flags & RCSW_DS_BSTREE_OS) {
      params->nodes = (malloc(ostree_node_space(TH_NUM_ITEMS)));
      params->elements = (malloc(ostree_element_space(TH_NUM_ITEMS, params->elt_size)));
      RCSW_CHECK_PTR(params->nodes);
      RCSW_CHECK_PTR(params->elements);
    } else {
      params->nodes = (malloc(bstree_node_space(TH_NUM_ITEMS)));
      params->elements = (malloc(bstree_element_space(TH_NUM_ITEMS, params->elt_size)));
      RCSW_CHECK_PTR(params->nodes);
      RCSW_CHECK_PTR(params->elements);
    }
    break;
  case ekRCSW_DS_STATIC_MATRIX:
    params->elements = (malloc(static_matrix_space(params->type.smat.n_rows,
                                                                       params->type.smat.n_cols,
                                                                       params->elt_size)));
    break;
  case ekRCSW_DS_DYNAMIC_MATRIX:
    params->elements = (malloc(dynamic_matrix_space(params->type.dmat.n_rows,
                                                                         params->type.dmat.n_cols,
                                                                         params->elt_size)));
    break;
  default:
    ER_ERR("No tag defined.");
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
  if (params->tag == ekRCSW_DS_BSTREE) {
    len = params->max_elts;
  } else if (params->tag == ekRCSW_DS_HASHMAP) {
    len = params->type.hm.bsize * params->type.hm.n_buckets;
  } else {
    len = params->max_elts;
  }
  if (params->flags & RCSW_DS_NOALLOC_DATA) {
    for (i = 0; i < len; ++i) {
      ER_CHECK(((struct allocm_entry*)(params->elements))[i].value == -1,
               "Memory leak at index %d in data block area", i);
    } /* for() */
  }
  return 0;

error:
  return 1;
} /* th_leak_check_data() */

int th_leak_check_nodes(const struct ds_params *params) {
  int i;
  int len;
  if (params->tag == ekRCSW_DS_BSTREE) {
    len = params->max_elts;
  } else if (params->tag == ekRCSW_DS_HASHMAP) {
    len = params->type.hm.bsize * params->type.hm.n_buckets;
  } else {
    len = params->max_elts;
  }
  /* It's not valid to check for leaks in this case, because you are sharing
   * things between two or more lists
   */
  if (params->flags & RCSW_DS_LLIST_NO_DB) {
    return 0;
  }
  if (params->flags & RCSW_DS_NOALLOC_NODES) {
    for (i = 0; i < len; ++i) {
      ER_CHECK(((struct allocm_entry *)(params->nodes))[i].value == -1,
                  "Memory leak at index %d in node area", i);
    }
  }
  return 0;

error:
  return 1;
} /* th_leak_check_nodes() */

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


END_C_DECLS
