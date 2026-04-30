/**
 * \file
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/utils/hash.h"

#include "rcsw/core/fpc.h"
#include "rcsw/er/client.h"

/*******************************************************************************
 * Constant Definitions
 ******************************************************************************/
/* taken from http://www.isthe.com/chongo/tech/comp/fnv/index.html#FNV-param */
#define FNV_PRIME 16777619U
#define FNV_OFFSET_BASIS 2166136261U

/*******************************************************************************
 * Public API
 ******************************************************************************/
BEGIN_C_DECLS

status_t utils_hash_default(const void* const data, size_t len, uint32_t* hash) {
  RCSW_FPC_NV(ERROR, NULL != data, len > 0, NULL != hash);

  const unsigned char* const key = data;
  *hash                          = 0;

  for (size_t i = 0; i < len; ++i) {
    *hash += key[i];
    *hash += (*hash << 10);
    *hash ^= (*hash >> 6);
  } /* for(i...) */

  *hash += (*hash << 3);
  *hash ^= (*hash >> 11);
  *hash += (*hash << 15);

  return OK;
}

status_t utils_hash_fnv1a(const void* const data, size_t len, uint32_t* hash) {
  RCSW_FPC_NV(ERROR, NULL != data, len > 0, NULL != hash);
  const unsigned char* const key = data;
  *hash                          = FNV_OFFSET_BASIS;

  for (size_t i = 0; i < len; i++) {
    *hash ^= key[i];
    *hash *= FNV_PRIME;
  }
  return OK;
}

status_t utils_hash_djb(const void* const data, size_t len, uint32_t* hash) {
  RCSW_FPC_NV(ERROR, NULL != data, len > 0);

  const unsigned char* const key = data;
  *hash                          = 5381;

  for (size_t i = 0; i < len; i++) {
    *hash = ((*hash << 5) + *hash) + key[i]; /* hash * 33 + c */
  }

  return OK;
}

END_C_DECLS
