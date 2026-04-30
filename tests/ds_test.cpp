/**
 * \file ds_test.cpp
 *
 * \copyright 2023 John Harwell, All rights reserved.
 *
 * SPDX-License Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "tests/ds_test.hpp"

#include <string.h>

/*******************************************************************************
 * Namespaces/Decls
 ******************************************************************************/
namespace th {

/*******************************************************************************
 * Non-Member Functions
 ******************************************************************************/
status_t ds_init(darray_config *const config) {
  RCSW_ER_MODULE_INIT();

  /* *2 is to allow the splice tests succeed without segfault */
  config->elements = reinterpret_cast<dptr_t *>(
    malloc(darray_element_space(TH_NUM_ITEMS * 2, config->elt_size)));
  config->max_elts = TH_NUM_ITEMS * 2;
  return OK;
}

status_t ds_init(rbuffer_config *const config) {
  RCSW_ER_MODULE_INIT();
  config->elements = reinterpret_cast<dptr_t *>(
    malloc(rbuffer_element_space(config->elt_size, TH_NUM_ITEMS)));
  return OK;
}

status_t ds_init(fifo_config *const config) {
  RCSW_ER_MODULE_INIT();
  config->elements = reinterpret_cast<dptr_t *>(
    malloc(fifo_element_space(config->elt_size, TH_NUM_ITEMS)));
  return OK;
}

status_t ds_init(multififo_config *const config) {
  RCSW_ER_MODULE_INIT();
  config->elements = reinterpret_cast<dptr_t *>(
    malloc(multififo_element_space(config->elt_size, TH_NUM_ITEMS)));
  config->meta = reinterpret_cast<dptr_t *>(
    malloc(multififo_meta_space(config->elt_size, TH_MULTIFIFO_CHILDREN)));
  return OK;
}

status_t ds_init(llist_config *const config) {
  RCSW_ER_MODULE_INIT();
  /*  *2 is to allow the splice_tests() succeed without segfault */
  config->max_elts = TH_NUM_ITEMS * 2;
  config->meta =
    reinterpret_cast<dptr_t *>(malloc(llist_meta_space(TH_NUM_ITEMS * 2)));
  config->elements = reinterpret_cast<dptr_t *>(
    malloc(llist_element_space(TH_NUM_ITEMS * 2, config->elt_size)));
  return OK;
}

status_t ds_init(binheap_config *const config) {
  RCSW_ER_MODULE_INIT();
  config->elements = reinterpret_cast<dptr_t *>(
    malloc(binheap_element_space(TH_NUM_ITEMS, config->elt_size)));
  memset(config->elements,
         0,
         binheap_element_space(TH_NUM_ITEMS, config->elt_size));
  return OK;
}

status_t ds_init(adjmatrix_config *const config) {
  RCSW_ER_MODULE_INIT();
  /* Just do weighted all the time--need the space.... */
  config->elements = reinterpret_cast<dptr_t *>(
    malloc(adjmatrix_element_space(TH_NUM_ITEMS, true)));
  memset(config->elements, 0, adjmatrix_element_space(TH_NUM_ITEMS, true));
  return OK;
}
status_t ds_init(hashmap_config *const config) {
  RCSW_ER_MODULE_INIT();
  config->meta =
    reinterpret_cast<dptr_t *>(malloc(hashmap_meta_space(TH_NUM_BUCKETS)));
  config->elements = reinterpret_cast<dptr_t *>(
    malloc(hashmap_element_space(TH_NUM_BUCKETS,
                                 TH_NUM_ITEMS * TH_NUM_ITEMS,
                                 config->elt_size)));
  return OK;
}
status_t ds_init(bstree_config *const config) {
  if (config->flags & RCSW_DS_BSTREE_OS) {
    config->meta =
      reinterpret_cast<dptr_t *>(malloc(ostree_meta_space(TH_NUM_ITEMS)));
    config->elements = reinterpret_cast<dptr_t *>(
      malloc(ostree_element_space(TH_NUM_ITEMS, config->elt_size)));
  } else {
    config->meta =
      reinterpret_cast<dptr_t *>(malloc(bstree_meta_space(TH_NUM_ITEMS)));
    config->elements = reinterpret_cast<dptr_t *>(
      malloc(bstree_element_space(TH_NUM_ITEMS, config->elt_size)));
  }
  return OK;
}
status_t ds_init(matrix_config *const config) {
  RCSW_ER_MODULE_INIT();
  config->elements = reinterpret_cast<dptr_t *>(malloc(
    matrix_element_space(config->n_rows, config->n_cols, config->elt_size)));

  return OK;
}
RCSW_CONST status_t ds_init(dynmatrix_config *const) { return OK; }

} /* namespace th */
