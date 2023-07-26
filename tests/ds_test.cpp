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

/*******************************************************************************
 * Namespaces/Decls
 ******************************************************************************/
namespace th {

/*******************************************************************************
 * Non-Member Functions
 ******************************************************************************/
status_t ds_init(darray_params *const params) {
  RCSW_ER_MODULE_INIT();

  /* *2 is to allow the splice tests succeed without segfault */
  params->elements = reinterpret_cast<uint8_t*>(malloc(darray_element_space(TH_NUM_ITEMS * 2,
                                                                            params->elt_size)));
  params->max_elts = TH_NUM_ITEMS * 2;
  return OK;
}

status_t ds_init(rbuffer_params *const params) {
  RCSW_ER_MODULE_INIT();
  params->elements =
      reinterpret_cast<uint8_t*>(malloc(rbuffer_element_space(params->elt_size,
                                                              TH_NUM_ITEMS)));
  return OK;
}

status_t ds_init(fifo_params *const params) {
  RCSW_ER_MODULE_INIT();
  params->elements =
      reinterpret_cast<uint8_t*>(malloc(fifo_element_space(params->elt_size, TH_NUM_ITEMS)));
  return OK;
}

status_t ds_init(llist_params *const params) {
  RCSW_ER_MODULE_INIT();
  /*  *2 is to allow the splice_tests() succeed without segfault */
  params->max_elts = TH_NUM_ITEMS * 2;
  params->meta = reinterpret_cast<uint8_t*>(malloc(llist_meta_space(TH_NUM_ITEMS * 2)));
  params->elements =
      reinterpret_cast<uint8_t*>(malloc(llist_element_space(TH_NUM_ITEMS * 2,
                                                            params->elt_size)));
  return OK;
}

status_t ds_init(binheap_params *const params) {
  RCSW_ER_MODULE_INIT();
  params->elements =
      reinterpret_cast<uint8_t*>(malloc(binheap_element_space(TH_NUM_ITEMS, params->elt_size)));
  memset(params->elements, 0,
         binheap_element_space(TH_NUM_ITEMS, params->elt_size));
  return OK;
}

status_t ds_init(adj_matrix_params *const params) {
  RCSW_ER_MODULE_INIT();
  /* Just do weighted all the time--need the space.... */
  params->elements = reinterpret_cast<uint8_t*>(malloc(adj_matrix_element_space(TH_NUM_ITEMS, true)));
  memset(params->elements, 0, adj_matrix_element_space(TH_NUM_ITEMS, true));
  return OK;
}
status_t ds_init(hashmap_params *const params) {
  RCSW_ER_MODULE_INIT();
  params->meta = reinterpret_cast<uint8_t*>(malloc(hashmap_meta_space(TH_NUM_BUCKETS)));
  params->elements = reinterpret_cast<uint8_t*>(malloc(hashmap_element_space(TH_NUM_BUCKETS,
                                                                             TH_NUM_ITEMS * TH_NUM_ITEMS,
                                                                             params->elt_size)));
  return OK;
}
status_t ds_init(bstree_params *const params) {

  if (params->flags & RCSW_DS_BSTREE_OS) {
    params->meta = reinterpret_cast<uint8_t*>(malloc(ostree_meta_space(TH_NUM_ITEMS)));
    params->elements = reinterpret_cast<uint8_t*>(malloc(ostree_element_space(TH_NUM_ITEMS, params->elt_size)));
  } else {
    params->meta = reinterpret_cast<uint8_t*>(malloc(bstree_meta_space(TH_NUM_ITEMS)));
    params->elements = reinterpret_cast<uint8_t*>(malloc(bstree_element_space(TH_NUM_ITEMS, params->elt_size)));
  }
  return OK;
}
status_t ds_init(matrix_params *const params) {
  RCSW_ER_MODULE_INIT();
  params->elements = reinterpret_cast<uint8_t*>(malloc(matrix_element_space(params->n_rows,
                                                                            params->n_cols,
                                                                            params->elt_size)));

  return OK;
}
status_t ds_init(dyn_matrix_params *const) {
  return OK;
}

} /* namespace th */
