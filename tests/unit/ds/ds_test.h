/**
 * \file
 *
 * \copyright 2017 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * Header file for common testing routines for the DS library.
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/ds/ds.h"
#include "rcsw/ds/hashmap.h"
#include "rcsw/ds/iter.h"
#include "rcsw/er/client.h"
#include "tests/unit/element.h"
#include "tests/unit/test.h"

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
 * \brief Default character output for test builds that set
 * RCSW_CONFIG_STDIO_PUTCHAR=th_putchar: forwards to putchar(). Weak, so the
 * stdio test can define its own.
 */
int th_putchar(int c);

END_C_DECLS
