/**
 * \file
 *
 * \copyright 2023 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/core/alloc.h"

#include <stdlib.h>
#include <string.h>

#include "rcsw/core/core.h"
#include "rcsw/core/flags.h"

/*******************************************************************************
 * Public API
 ******************************************************************************/
void* rcsw_alloc(void* ptr, size_t n_bytes, uint32_t flags) {
  void* ret = NULL;

#if defined(RCSW_CONFIG_NOALLOC)
  /*
   * Memory allocation is disabled entirely: always use the caller-supplied
   * pointer.
   */
  ret = ptr;

#if defined(RCSW_CONFIG_ZALLOC)
  memset(ret, 0, n_bytes);
#else
  /*
   * Allow per-call override to get zeroed memory, as some modules rely on
   * that unconditionally (e.g., mpool).
   */
  if (flags & RCSW_ZALLOC) {
    memset(ret, 0, n_bytes);
  }
#endif /* RCSW_CONFIG_ZALLOC */

#else
  /*
   * Memory allocation is allowed.
   */

  /*
   * If SOME kind of "don't do memory allocation for XXX" flag is passed, assume
   * that the passed pointer should be used instead.
   */
  if ((flags & RCSW_NOALLOC_HANDLE) || (flags & RCSW_NOALLOC_DATA) ||
      (flags & RCSW_NOALLOC_META)) {
    ret = ptr;
  } else {

#if defined(RCSW_CONFIG_ZALLOC)
    ret = calloc(1, n_bytes);

#else
    ret = malloc(n_bytes);

    /*
     * Allow per-call override to get zeroed memory, as some modules rely on
     * that unconditionally (e.g., mpool).
     */
    if (flags & RCSW_ZALLOC) {
      memset(ret, 0, n_bytes);
    }

#endif /* RCSW_CONFIG_ZALLOC */
  }

#endif /* RCSW_CONFIG_NOALLOC */

  return ret;
} /* rcsw_alloc() */

void rcsw_free(void* ptr, uint32_t flags) {
  RCSW_CHECK_PTR(ptr);

#if defined(RCSW_CONFIG_NOALLOC)
  /* memory allocation is disabled entirely--nothing to do */
  return;
#else
  /*
   * Memory allocation is allowed.
   */

  /*
   * If SOME kind of "don't do memory allocation for XXX" flag was passed to
   * rcsw_alloc(), nothing to do.
   */
  if ((flags & RCSW_NOALLOC_HANDLE) || (flags & RCSW_NOALLOC_DATA) ||
      (flags & RCSW_NOALLOC_META)) {
    return;
  } else {
    free(ptr);
  }
#endif

error:
  return;
} /* rcsw_free() */
