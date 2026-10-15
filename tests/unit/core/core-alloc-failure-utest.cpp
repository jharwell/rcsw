/**
 * \file
 *
 * \copyright 2026 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * Recovery from heap allocation failures: init and resize functions that make
 * several allocations must, when any one of them fails, return an error,
 * release everything they allocated, and leave existing objects unchanged.
 *
 * This executable replaces malloc()/calloc()/realloc()/free() with wrappers
 * around glibc's allocator that can fail the Nth allocation and count live
 * blocks. Replacing the allocator does not mix with a sanitizer's own
 * allocator, so in sanitized builds and on non-glibc platforms the tests only
 * warn that they did not run.
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#define CATCH_CONFIG_PREFIX_ALL
#include <cstdlib>
#include <cstring>

#include <catch2/catch_test_macros.hpp>

#include "rcsw/ds/dynmatrix.h"
#include "rcsw/multithread/mpool.h"

/*******************************************************************************
 * Allocation Fault Injection
 ******************************************************************************/
#if defined(__SANITIZE_ADDRESS__) || defined(__SANITIZE_THREAD__)
#define TH_ALLOC_SANITIZED 1
#elif defined(__has_feature)
#if __has_feature(address_sanitizer) || __has_feature(thread_sanitizer) || \
  __has_feature(memory_sanitizer)
#define TH_ALLOC_SANITIZED 1
#endif
#endif

#if defined(__GLIBC__) && !defined(TH_ALLOC_SANITIZED)
#define TH_ALLOC_INJECT 1

namespace {
long g_fail_at = 0; /* fail the Nth allocation after arming; 0 = never */
long g_count   = 0; /* allocations since arming */
long g_live    = 0; /* blocks currently allocated */

bool fail_now() { return 0 != g_fail_at && ++g_count == g_fail_at; }
} /* namespace */

extern "C" {
void* __libc_malloc(size_t n);
void* __libc_calloc(size_t n, size_t size);
void* __libc_realloc(void* p, size_t n);
void  __libc_free(void* p);

void* malloc(size_t n) {
  void* p = fail_now() ? nullptr : __libc_malloc(n);
  g_live += (nullptr != p);
  return p;
}
void* calloc(size_t n, size_t size) {
  void* p = fail_now() ? nullptr : __libc_calloc(n, size);
  g_live += (nullptr != p);
  return p;
}
void* realloc(void* p, size_t n) {
  void* r = fail_now() ? nullptr : __libc_realloc(p, n);
  g_live += (nullptr == p && nullptr != r);
  return r;
}
void free(void* p) {
  g_live -= (nullptr != p);
  __libc_free(p);
}
} /* extern "C" */
#endif

namespace {
/* Fail the Nth allocation from now on (1-based); 0 disarms. */
void arm(long n) {
#if defined(TH_ALLOC_INJECT)
  g_fail_at = n;
  g_count   = 0;
#else
  (void)n;
#endif
}

long live_blocks() {
#if defined(TH_ALLOC_INJECT)
  return g_live;
#else
  return 0;
#endif
}

/*
 * Whether failures can be injected. If not, the test passes vacuously with a
 * warning (not CATCH_SKIP(), which fails the run when every test is skipped).
 */
bool injection_available() {
#if defined(TH_ALLOC_INJECT)
  return true;
#else
  CATCH_WARN("allocation failures can only be injected in unsanitized glibc "
             "builds; not tested");
  return false;
#endif
}

int val(size_t i, size_t j) { return static_cast<int>(i * 100 + j + 1); }

struct dynmatrix* make_filled(size_t rows, size_t cols) {
  struct dynmatrix_config c;
  memset(&c, 0, sizeof(c));
  c.elt_size          = sizeof(int);
  c.n_rows            = rows;
  c.n_cols            = cols;
  struct dynmatrix* m = dynmatrix_init(nullptr, &c);
  CATCH_REQUIRE(nullptr != m);
  for (size_t i = 0; i < rows; ++i) {
    for (size_t j = 0; j < cols; ++j) {
      int v = val(i, j);
      CATCH_REQUIRE(OK == dynmatrix_set(m, i, j, &v));
    }
  }
  return m;
}

/*
 * \p m is \p rows x \p cols, and each cell holds its original value if it was
 * inside the original \p rows0 x \p cols0 matrix, and 0 otherwise.
 */
bool holds(const struct dynmatrix* m,
           size_t                  rows,
           size_t                  cols,
           size_t                  rows0,
           size_t                  cols0) {
  if (m->n_rows != rows || m->n_cols != cols) {
    return false;
  }
  for (size_t i = 0; i < rows; ++i) {
    for (size_t j = 0; j < cols; ++j) {
      int want = (i < rows0 && j < cols0) ? val(i, j) : 0;
      if (want != *static_cast<int*>(dynmatrix_access(m, i, j))) {
        return false;
      }
    }
  }
  return true;
}
} /* namespace */

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("dynmatrix_init releases everything when an allocation fails",
                "[core][alloc][dynmatrix]") {
  if (!injection_available()) {
    return;
  }
  struct dynmatrix_config c;
  memset(&c, 0, sizeof(c));
  c.elt_size = sizeof(int);
  c.n_rows   = 4;
  c.n_cols   = 3;
  dynmatrix_destroy(dynmatrix_init(nullptr, &c)); /* one-time setup */

  long n = 1;
  for (; n < 100; ++n) {
    long before = live_blocks();
    arm(n);
    struct dynmatrix* m = dynmatrix_init(nullptr, &c);
    arm(0);
    if (nullptr != m) {
      dynmatrix_destroy(m);
      CATCH_REQUIRE(before == live_blocks());
      break;
    }
    CATCH_REQUIRE(before == live_blocks()); /* nothing leaked */
  }
  CATCH_REQUIRE(n > 1); /* at least one failure was exercised */
}

CATCH_TEST_CASE("dynmatrix_resize is all-or-nothing when an allocation fails",
                "[core][alloc][dynmatrix]") {
  if (!injection_available()) {
    return;
  }
  dynmatrix_destroy(make_filled(1, 1)); /* one-time setup */

  /* every combination of growing/shrinking rows and columns */
  const size_t shapes[][2] = {
    {6, 7}, {2, 7}, {6, 2}, {2, 2}, {4, 3}, {1, 1}, {9, 1}, {1, 9}};
  for (const auto& shape : shapes) {
    size_t rows = shape[0];
    size_t cols = shape[1];
    long   n    = 1;
    for (; n < 200; ++n) {
      long              before = live_blocks();
      struct dynmatrix* m      = make_filled(4, 3);
      arm(n);
      status_t rc = dynmatrix_resize(m, rows, cols);
      arm(0);
      if (OK == rc) {
        CATCH_REQUIRE(holds(m, rows, cols, 4, 3));
        /* and the matrix is still fully usable */
        int x = 5;
        CATCH_REQUIRE(OK == dynmatrix_set(m, rows + 1, cols + 1, &x));
        CATCH_REQUIRE(5 ==
                      *static_cast<int*>(dynmatrix_access(m, rows + 1, cols + 1)));
        dynmatrix_destroy(m);
        CATCH_REQUIRE(before == live_blocks());
        break;
      }
      /* a failed resize leaves the matrix exactly as it was */
      CATCH_REQUIRE(holds(m, 4, 3, 4, 3));
      dynmatrix_destroy(m);
      CATCH_REQUIRE(before == live_blocks());
    }
    CATCH_REQUIRE(n < 200);
  }
}

CATCH_TEST_CASE("mpool_init releases everything when an allocation fails",
                "[core][alloc][mpool]") {
  if (!injection_available()) {
    return;
  }
  struct mpool_config c;
  memset(&c, 0, sizeof(c));
  c.elt_size = 16;
  c.max_elts = 8;
  mpool_destroy(mpool_init(nullptr, &c)); /* one-time setup */

  long n = 1;
  for (; n < 100; ++n) {
    long before = live_blocks();
    arm(n);
    struct mpool* p = mpool_init(nullptr, &c);
    arm(0);
    if (nullptr != p) {
      void* chunk = mpool_req(p);
      CATCH_REQUIRE(nullptr != chunk);
      CATCH_REQUIRE(OK == mpool_release(p, chunk));
      mpool_destroy(p);
      CATCH_REQUIRE(before == live_blocks());
      break;
    }
    CATCH_REQUIRE(before == live_blocks());
  }
  CATCH_REQUIRE(n > 1);
}
