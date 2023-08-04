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

#include "rcsw/er/client.h"
#include "rcsw/ds/binheap.h"
#include "rcsw/ds/bstree.h"
#include "rcsw/ds/darray.h"
#include "rcsw/ds/dyn_matrix.h"
#include "rcsw/ds/fifo.h"
#include "rcsw/ds/hashmap.h"
#include "rcsw/ds/inttree.h"
#include "rcsw/ds/llist.h"
#include "rcsw/ds/ostree.h"
#include "rcsw/ds/rbuffer.h"
#include "rcsw/ds/adj_matrix.h"
#include "rcsw/ds/matrix.h"

#include "tests/ds_test.h"

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
    U e;
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
    T e;
    e.value2 = 17;
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

 private:
  enum gen_elt_type m_type;
  int               m_max_elts;
  int               m_i;
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
bool_t iter_func(void *e) {
  T* e1 = reinterpret_cast<T*>(e);
  return (e1->value1 % 2 == 0);
}

status_t ds_init(darray_params *const params);
status_t ds_init(rbuffer_params *const params);
status_t ds_init(fifo_params *const params);
status_t ds_init(llist_params *const params);
status_t ds_init(binheap_params *const params);
status_t ds_init(adj_matrix_params *const params);
status_t ds_init(hashmap_params *const params);
status_t ds_init(bstree_params *const params);
status_t ds_init(matrix_params *const params);
status_t ds_init(dyn_matrix_params *const params);


template<typename T>
void ds_shutdown(const T *const params) {
  if constexpr(has_elements<T>::value) {
     if (params->elements) {
       free(params->elements);
      }
  }
  if constexpr (has_meta<T>::value) {
    if (params->meta) {
      free(params->meta);
    }
  }
}

template<typename T>
int leak_check_data(const T *params) {
  int i;
  int len;
  if constexpr(std::is_same<T,struct bstree_params>::value) {
    len = params->max_elts;
  } else if constexpr(std::is_same<T,struct hashmap_params>::value) {
    len = params->bsize * params->n_buckets;
  } else {
    len = params->max_elts;
  }
  if (params->flags & RCSW_NOALLOC_DATA) {
    for (i = 0; i < len; ++i) {
      ER_CHECK((reinterpret_cast<allocm_entry*>(params->elements))[i].value == -1,
               "Memory leak at index %d in data block area", i);
    } /* for() */
  }
  return 0;

error:
  return 1;
}

template<typename T>
int leak_check_nodes(const T *params) {
  int i;
  int len;
  if constexpr(std::is_same<T,struct bstree_params>::value) {
      len = params->max_elts;
    } else if constexpr(std::is_same<T,struct hashmap_params>::value) {
      len = params->bsize * params->n_buckets;
    } else {
    len = params->max_elts;
  }
  /* It's not valid to check for leaks in this case, because you are sharing
   * things between two or more lists
   */
  if (params->flags & RCSW_DS_LLIST_DB_DISOWN) {
    return 0;
  }
  if (params->flags & RCSW_NOALLOC_META) {
    for (i = 0; i < len; ++i) {
      ER_CHECK((reinterpret_cast< allocm_entry *>(params->meta))[i].value == -1,
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
