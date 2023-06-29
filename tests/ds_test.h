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
#include "rcsw/ds/iter.h"

#include "tests/element.h"

/*******************************************************************************
 * Constant Definitions
 ******************************************************************************/
#define TH_NUM_ITEMS    16
#define TH_NUM_BUCKETS  10
#define NUM_MERGE_ITEMS 10

/*******************************************************************************
 * Structure Definitions
 ******************************************************************************/
enum gen_elt_type {
  ekINC_VALS,
  ekDEC_VALS,
  ekRAND_VALS
};

/*******************************************************************************
 * Type Definitions
 ******************************************************************************/
typedef void (*ds_test_t)(int len, struct ds_params *params);
typedef void (*ds_test2_t)(int len1, int len2, struct ds_params *params);

typedef void (*hashmap_test_t)(struct ds_params *params);

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Print a hashnode
 */
void th_printn(const void * node);

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

/**
 * \brief Compare the keys of two nodes (BSTREE, hashmap)
 */
int th_key_cmp(const void *a, const void *b) RCSW_PURE;

/**
 * \brief Stub implementation so that unit tests OTHER than the stdio test
 * (which defines its own version of this) link.
 */
int th_putchar(int c) RCSW_ATTR(weak,const);

END_C_DECLS

#endif /* TESTS_DS_TEST_H_ */
