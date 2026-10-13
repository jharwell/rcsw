/**
 * \file
 *
 * \copyright 2023 John Harwell
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/er/er.h"
#include "rcsw/core/flags.h"

/*******************************************************************************
 * Constants
 ******************************************************************************/
#define TH_NUM_ITEMS 16UL

/*
 * Flags every configuration under test must carry. With RCSW_CONFIG_NOALLOC
 * the library never allocates, whatever the flags say, so tests must always
 * take their caller-provided-storage branches (pass a handle and pools, skip
 * resizing, ...).
 */
#if defined(RCSW_CONFIG_NOALLOC)
#define TH_FORCED_FLAGS RCSW_NOALLOC_ALL
#else
#define TH_FORCED_FLAGS RCSW_NONE
#endif

#if RCSW_CONFIG_ER_PLUGIN == RCSW_ER_PLUGIN_ZLOG
#define TH_ZLOG_CONF "./test.conf"
#else
#define TH_ZLOG_CONF
#endif
