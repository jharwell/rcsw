/**
 * \file ds_test.h
 *
 * Header file for common testing routines for the DS library.
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/ds.h"
#include "rcsw/ds/hashmap.h"
#include "rcsw/ds/iter.h"
#include "rcsw/er/client.h"
#include "tests/element.h"
#include "tests/test.h"

/*******************************************************************************
 * Constants
 ******************************************************************************/
#define TH_NUM_BUCKETS 10
#define NUM_MERGE_ITEMS 10
#define TH_MULTIFIFO_CHILDREN 1

struct hashmap_params;

/*******************************************************************************
 * Types
 ******************************************************************************/
typedef void (*hashmap_test_t)(struct hashmap_config* config);

/*******************************************************************************
 * Public API
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Print a hashnode
 */
void th_printn(const void* node);

/**
 * \brief Compare the keys of two nodes (BSTREE, hashmap)
 */
int th_key_cmp(const void* a, const void* b) RCSW_PURE;

/**
 * \brief Stub implementation so that unit tests OTHER than the stdio test
 * (which defines its own version of this) link.
 */
int th_putchar(int c);

END_C_DECLS
