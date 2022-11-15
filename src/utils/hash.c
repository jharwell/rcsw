/**
 * \file hash.c
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * This file is part of RCSW.
 *
 * RCSW is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * RCSW is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * RCSW.  If not, see <http://www.gnu.org/licenses/
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/utils/hash.h"
#include "rcsw/common/dbg.h"
#include "rcsw/common/fpc.h"

/*******************************************************************************
 * Constant Definitions
 ******************************************************************************/
/* taken from http://www.isthe.com/chongo/tech/comp/fnv/index.html#FNV-param */
#define FNV_PRIME 16777619U
#define FNV_OFFSET_BASIS 2166136261U

/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS

uint32_t hash_default(const void* const data, size_t len) {
  RCSW_FPC_NV(0, NULL != data, len > 0);

  const char* const key = data;
  uint32_t hash = 0;

  for (size_t i = 0; i < len; ++i) {
    hash += (uint32_t)key[i];
    hash += (hash << 10);
    hash ^= (hash >> 6);
  } /* for(i...) */

  hash += (hash << 3);
  hash ^= (hash >> 11);
  hash += (hash << 15);

  return hash;
} /* hash_default()  */

uint32_t hash_fnv1a(const void* const data, size_t len) {
  RCSW_FPC_NV(0, NULL != data, len > 0);
  const char* const key = data;
  uint32_t hash = FNV_OFFSET_BASIS;

  for (size_t i = 0; i < len; i++) {
    hash ^= (uint32_t)key[i];
    hash *= FNV_PRIME;
  }
  return hash;
} /* hash_fnv1a() */

uint32_t hash_djb(const void* const data, size_t len) {
  RCSW_FPC_NV(0, NULL != data, len > 0);

  const char* const key = data;
  uint32_t hash = 5381;
  size_t i;

  for (i = 0; i < len; i++) {
    hash = ((hash << 5) + hash) + (uint32_t)key[i]; /* hash * 33 + c */
  }

  return hash;
} /* hash_djb() */

END_C_DECLS
