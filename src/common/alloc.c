/**
 * \file alloc.c
 *
 * \copyright 2023 John Harwell, All rights reserved.
 *
 * SPDX-License Identifier:
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/common/alloc.h"
#include "rcsw/common/flags.h"

/*******************************************************************************
 * API Functions
 ******************************************************************************/
void* rcsw_alloc(void* ptr, size_t n_bytes, uint32_t flags) {
  void* ret = NULL;

#if defined(RCSW_NOALLOC)
  /*
   * Memory allocation is disabled entirely.
   */
  ret = ptr;

#if defined(RCSW_ZALLOC)
  memset(ret, 0, n_bytes);
#else
  /*
   * Allow per-call override to get zeroed memory, as some modules rely on
   * that unconditionally (e.g., mpool).
   */
  if (flags & RCSW_ZALLOC) {
    memset(ret, 0, n_bytes);
  }
#endif /* RCSW_ZALLOC */

#else
  /*
   * Memory allocation is allowed.
   */

  /*
   * If SOME kind of "don't do memory allocation for XXX" flag is passed, assume
   * that the passed pointer should be used instead.
   */
  if ((flags & RCSW_NOALLOC_HANDLE) ||
      (flags & RCSW_NOALLOC_DATA) ||
      (flags & RCSW_NOALLOC_META)) {
    ret = ptr;
  } else {
#if defined(RCSW_ZALLOC)
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

#endif /* RCSW_ZALLOC */
  }

#endif /* RCSW_NOALLOC */

  return ret;
} /* rcsw_alloc() */

void rcsw_free(void* ptr, uint32_t flags) {
  RCSW_CHECK_PTR(ptr);

#if defined(RCSW_NOALLOC)
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
  if ((flags & RCSW_NOALLOC_HANDLE) ||
      (flags & RCSW_NOALLOC_DATA) ||
      (flags & RCSW_NOALLOC_META)) {
    return;
  } else {
    free(ptr);
  }
#endif

error:
  return;
} /* rcsw_free() */
