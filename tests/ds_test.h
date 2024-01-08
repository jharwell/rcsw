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
#include "tests/test.h"

#include "rcsw/er/client.h"

/*******************************************************************************
 * Constant Definitions
 ******************************************************************************/
#define TH_NUM_ITEMS    16
#define TH_NUM_BUCKETS  10
#define NUM_MERGE_ITEMS 10
#define TH_MULTIFIFO_CHILDREN 1

/*******************************************************************************
 * Structure Definitions
 ******************************************************************************/
enum gen_elt_type {
  ekINC_VALS,
  ekDEC_VALS,
  ekRAND_VALS,
  ekPACKED_VALS
};

struct hashmap_params;
/*******************************************************************************
 * Type Definitions
 ******************************************************************************/
typedef void (*hashmap_test_t)(struct hashmap_params *params);

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Print a hashnode
 */
void th_printn(const void * node);

/**
 * \brief Compare the keys of two nodes (BSTREE, hashmap)
 */
int th_key_cmp(const void *a, const void *b) RCSW_PURE;

/**
 * \brief Stub implementation so that unit tests OTHER than the stdio test
 * (which defines its own version of this) link.
 */
int th_putchar(int c);

END_C_DECLS

#endif /* TESTS_DS_TEST_H_ */
