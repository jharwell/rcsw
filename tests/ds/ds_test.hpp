/**
 * \file ds_test.hpp
 *
 * \copyright 2023 John Harwell, All rights reserved.
 *
 * SPDX-License Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <vector>
#include <algorithm>

#include "rcsw/er/client.h"
#include "rcsw/ds/binheap.h"
#include "rcsw/ds/bstree.h"
#include "rcsw/ds/darray.h"
#include "rcsw/ds/dynmatrix.h"
#include "rcsw/ds/fifo.h"
#include "rcsw/ds/hashmap.h"
#include "rcsw/ds/inttree.h"
#include "rcsw/ds/llist.h"
#include "rcsw/ds/ostree.h"
#include "rcsw/ds/rbuffer.h"
#include "rcsw/ds/adjmatrix.h"
#include "rcsw/ds/matrix.h"
#include "rcsw/ds/multififo.h"

#include "tests/ds/ds_test.h"
#include "tests/element.hpp"

/*******************************************************************************
 * Namespaces/Decls
 ******************************************************************************/
namespace th {

/*******************************************************************************
 * Templates
 ******************************************************************************/
template <typename T, typename = int>
struct has_meta : std::false_type { };

template <typename T>
struct has_meta <T, decltype((void) T::meta, 0)> : std::true_type { };

template <typename T, typename = int>
struct has_elements : std::false_type { };

template <typename T>
struct has_elements <T, decltype((void) T::elements, 0)> : std::true_type { };

/*******************************************************************************
 * Public API
 ******************************************************************************/
/**
 * \brief Compare the data of two hashnodes
 */
template<typename T>
int data_cmp(const void *a, const void *b) {
  const struct hashnode *q1 = reinterpret_cast<const T*>(a);
  const struct hashnode *q2 = reinterpret_cast<const T*>(b);
  const T* d1 = reinterpret_cast<const T*>(q1->data);
  const T* d2 = reinterpret_cast<const T*>(q2->data);
  return d1->value1 - d2->value2;
}

  status_t ds_init(darray_config *const config);
status_t ds_init(rbuffer_config *const config);
status_t ds_init(fifo_config *const config);
status_t ds_init(multififo_config *const config);
status_t ds_init(llist_config *const config);
status_t ds_init(binheap_config *const config);
status_t ds_init(adjmatrix_config *const config);
status_t ds_init(hashmap_config *const config);
status_t ds_init(bstree_config *const config);
status_t ds_init(matrix_config *const config);
status_t ds_init(dynmatrix_config *const config);


template<typename T>
void ds_shutdown(const T *const config) {
  if constexpr(has_elements<T>::value) {
     if (config->elements) {
       free(config->elements);
      }
  }
  if constexpr (has_meta<T>::value) {
    if (config->meta) {
      free(config->meta);
    }
  }
}

template<typename T>
int leak_check_data(const T *config) {
  int i;
  int len;
  if constexpr(std::is_same<T,struct bstree_config>::value) {
    len = config->max_elts;
  } else if constexpr(std::is_same<T,struct hashmap_config>::value) {
    len = config->bsize * config->n_buckets;
  } else {
    len = config->max_elts;
  }
  if (config->flags & RCSW_NOALLOC_DATA) {
    for (i = 0; i < len; ++i) {
      ER_CHECK((reinterpret_cast<allocm_entry*>(config->elements))[i].value == -1,
               "Memory leak at index %d in data block area", i);
    } /* for() */
  }
  return 0;

error:
  return 1;
}

template<typename T>
int leak_check_nodes(const T *config) {
  int i;
  int len;
  if constexpr(std::is_same<T,struct bstree_config>::value) {
      len = config->max_elts;
    } else if constexpr(std::is_same<T,struct hashmap_config>::value) {
      len = config->bsize * config->n_buckets;
    } else {
    len = config->max_elts;
  }
  /* It's not valid to check for leaks in this case, because you are sharing
   * things between two or more lists
   */
  if (config->flags & RCSW_DS_LLIST_DB_DISOWN) {
    return 0;
  }
  if (config->flags & RCSW_NOALLOC_META) {
    for (i = 0; i < len; ++i) {
      ER_CHECK((reinterpret_cast< allocm_entry *>(config->meta))[i].value == -1,
               "Memory leak at index %d in meta area", i);
    }
  }
  return 0;

error:
  return 1;
}
} /* namespace th */

/*******************************************************************************
 * Operators
 ******************************************************************************/
/**
 * run_test_flags
 *
 * Runs test(len, config) for:
 *   1. RCSW_NONE alone
 *   2. Each flag alone
 *   3. All pairwise combinations of flags
 * across len = 1..max_len.
 */
template <typename Config, typename TestFn>
static void run_test_flags(Config&              config,
                           const uint32_t*      flags,
                           size_t               n_flags,
                           int                  max_len,
                           TestFn               test) {
  /* 1. RCSW_NONE */
  for (int k = 1; k <= max_len; ++k) {
    config.flags = RCSW_NONE;
    test(k, &config);
  }

  /* 2. Each flag alone */
  for (size_t i = 0; i < n_flags; ++i) {
    for (int k = 1; k <= max_len; ++k) {
      config.flags = flags[i];
      test(k, &config);
    }
  }

  /* 3. Pairwise combinations */
  for (size_t i = 0; i < n_flags; ++i) {
    for (size_t j = i + 1; j < n_flags; ++j) {
      uint32_t combo = flags[i] | flags[j];
      for (int k = 1; k <= max_len; ++k) {
        config.flags = combo;
        test(k, &config);
      }
    }
  }
}

/**
 * Checks that [sorted, sorted+n) is:
 *   1. In non-decreasing order per cmpe
 *   2. A permutation of original
 */
template <typename T>
static void verify_sort_permutation(const std::vector<T>& original,
                                    const T*              sorted,
                                    size_t                n) {
  for (size_t i = 0; i + 1 < n; ++i) {
    CATCH_REQUIRE(th::cmpe<T>(sorted + i, sorted + i + 1) <= 0);
  }
  std::vector<T> a = original;
  std::vector<T> b(sorted, sorted + n);
  std::sort(a.begin(), a.end());
  std::sort(b.begin(), b.end());
  CATCH_REQUIRE(a == b);
}
