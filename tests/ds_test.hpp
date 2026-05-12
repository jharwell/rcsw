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

#include "tests/ds_test.h"
#include "tests/ds_test.hpp"
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
 * Class Definitions
 ******************************************************************************/
template<typename T>
class element_generator {
 public:
  element_generator(enum gen_elt_type type, int max_elts)
    : m_type(type),
      m_max_elts(max_elts),
      m_i(0) {}

  void reset(void) {
    m_i = 0;
  }

  static T sentinel(void) {
    T e;
    e.value1 = -1;
    return e;
  }

  template <typename U = T,
            typename std::enable_if<std::is_same<U, element1>::value,
                                    int>::type = 0>
  U next(void) {
    U e{};
    if (ekINC_VALS == m_type) {
      e.value1 = m_i;
    } else if (ekDEC_VALS == m_type) {
      e.value1 = m_max_elts - m_i;
    } else {
      e.value1 = rand() % m_max_elts;
    }
    ++m_i;
    return e;
  }

  template <typename U = T,
            typename std::enable_if<!std::is_same<U, element1>::value,
                                    int>::type = 0>
  T next(void) {
    T e{};
    e.value2 = 17;
    if (ekINC_VALS == m_type) {
      e.value1 = m_i;
    } else if (ekDEC_VALS == m_type) {
      e.value1 = m_max_elts - m_i;
    } else if (ekPACKED_VALS == m_type) {
      auto upper = (m_i & (-1UL << std::numeric_limits<decltype(std::declval<T>().value1)>::digits / 2));
      auto lower = (m_i & (-1UL >> (std::numeric_limits<decltype(std::declval<T>().value1)>::digits / 2)));
      e.value1 = upper | lower;
    } else {
      e.value1 = rand() % m_max_elts;
    }
    ++m_i;
    return e;
  }

 private:
  enum gen_elt_type m_type;
  int               m_max_elts;
  int64_t           m_i;
};

/* Test data element for all data structures */
template<typename T>
struct element_set {
  explicit element_set(size_t size) : elts(size) {}

  void data_gen(void) {
    element_generator<T> g(gen_elt_type::ekDEC_VALS, elts.size());
    for (size_t i = 0; i < elts.size(); ++i) {
      elts[i] = g.next();
    } /* for(i..) */
  }
  std::vector<T> elts;
};


/*******************************************************************************
 * API Functions
 ******************************************************************************/
/**
 * \brief Generate an element.
 */
template<typename T>
T gen_elt(enum gen_elt_type type, int i, int max_elts) {
  T e;
  e.value2 = -1;
  if (ekINC_VALS == type) {
    e.value1 = i;
  } else if (ekDEC_VALS == type) {
    e.value1 = max_elts - i;
  } else {
    e.value1 = rand() % max_elts;
  }
  return e;
} /* th_gen_elt() */

/**
 * \brief Print an element (any data structure)
 */
template<typename T>
void printe(const void *e) {
  const T *el = reinterpret_cast<const T*>(e);
  if constexpr (std::is_same<T, element1>::value) {
      DPRINTF("value1: %d value2: N/A\n", el->value1);
  } else {
    DPRINTF("value1: %d value2: %d\n", el->value1, el->value2);
  }
} /* th_printe() */

/**
 * \brief Simple filtering function for testing filter() and filter2()
 *
 * \return 0 if element does not fulfill the filter requirements, non-zero
 *         otherwise
 */
template<typename T>
bool_t filter_func(const void *const e) {
  const T* q = reinterpret_cast<const T*>(e);
  return (q->value1 % 2 == 0);
} /* th_filter_func() */

/**
 * \brief Compare two elements (any data structure)
 *
 * \return  < 0 if e1 < e2, 0 if e1 == e2, > 0 if e2 > e1
 */
template<typename T>
int cmpe(const void *const e1, const void *const e2) {
  const T* q1 = reinterpret_cast<const T*>(e1);
  const T* q2 = reinterpret_cast<const T*>(e2);

  if (q1->value1 < q2->value1) {
    return -1;
  } else if (q1->value1 == q2->value1) {
    return 0;
  }
  return 1;
}

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

/**
 * \brief Iterate with a cumulative SOMETHING
 */
template<typename T>
void inject_func(void *e, void *res) {
  T* e1 = reinterpret_cast<T*>(e);
  int *tmp = reinterpret_cast<int*>(res);
  *tmp += e1->value1;
}

/**
 * \brief Map an element (i.e. do something to it)
 */
template<typename T>
void map_func(void *e) {
  T *e1 = reinterpret_cast<T*>(e);
  e1->value1--;
}

/**
 * \brief Classify an element for iteration
 *
 * RETURN:
 *     int - 1 if should be returned, 0 if not
 */
template<typename T>
bool_t iter_func_even(void *e) {
  T* e1 = reinterpret_cast<T*>(e);
  return (e1->value1 % 2 == 0);
}

  template<typename T>
bool_t iter_func_all(void *e) {
  return true;
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
template<typename T>
bool operator<(const T& lhs, const T& rhs) {
  return lhs.value1 < rhs.value1;
}

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
