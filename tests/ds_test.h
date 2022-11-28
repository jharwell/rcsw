/**
 * \file ds_test.h
 *
 * Header file for common testing routines for the DS library.
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef TESTS_DS_TEST_H_
#define TESTS_DS_TEST_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/ds.h"
#include "rcsw/common/test_frmwk.h"
#include "rcsw/common/dbg.h"

/*******************************************************************************
 * Constant Definitions
 ******************************************************************************/
#define NUM_ITEMS       10
#define NUM_MERGE_ITEMS 10

/*******************************************************************************
 * Structure Definitions
 ******************************************************************************/
/* Test data element for all data structures */
struct element {
  int value1;
  int value2;
};

/*******************************************************************************
 * Global Variables
 ******************************************************************************/
BEGIN_C_DECLS
extern struct test_frmwk *tests;

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/


/**
 * \brief Classify an element for iteration
 *
 * RETURN:
 *     int - 1 if should be returned, 0 if not
 */
bool_t th_iter_func(void * e) RCSW_PURE;

/**
 * \brief Map an element (i.e. do something to it)
 */
void th_map_func(void * e);

/**
 * \brief Iterate with a cumulative SOMETHING
 */
void th_inject_func(void* e, void * res);

/**
 * \brief Compare the data of two hashnodes
 */
int th_data_cmp(const void *a, const void *b) RCSW_PURE;

/**
 * \brief Print a hashnode
 */
void th_printn(const void * node);

/**
 * \brief Compare two elements (any data structure)
 *
 * \return  < 0 if e1 < e2, 0 if e1 == e2, > 0 if e2 > e1
 */
int th_cmpe(const void* const e1, const void* const e2) RCSW_PURE;

/**
 * \brief Print an element (any data structure)
 */
void th_printe(const void* const e1);

/**
 * \brief Compare the keys of two nodes (linked list or BSTREE)
 */
int th_key_cmp(const void* const e1, const void* const e2) RCSW_PURE;

/**
 * \brief Simple filtering function for testing filter() and filter2()
 *
 * \return 0 if element does not fulfill the filter requirements, non-zero
 *         otherwise
 */
bool_t th_filter_func(const void* const e) RCSW_PURE;

/**
 * \brief Check for leaked memory within application allocated node memory
 *
 * This function checks for memory leaks within the application-allocated chunks
 * for nodes.
 *
 * \return if no leaks, non-zero otherwise
 */
int th_leak_check_nodes(const struct ds_params * const params) RCSW_PURE;

/**
 * \brief Check for leaked memory within application allocated data
 *
 * This function checks for memory leaks within the application-allocated chunks
 * for data. It is safe to call this function for BSTREE with all data in the DS
 * domain.
 *
 * \return 0 if no leaks, non-zero otherwise
 *
 */
int th_leak_check_data(const struct ds_params * const params) RCSW_PURE;

/**
 * \brief Initialize test harness
 *
 * \return \ref status_t
 */
status_t th_ds_init(struct ds_params *const params);

/**
 * \brief Shutdown the test harness
 */
void th_ds_shutdown(const struct ds_params *const params);

END_C_DECLS

#endif /* TESTS_DS_TEST_H_ */
