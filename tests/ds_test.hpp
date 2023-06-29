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
T th_gen_elt(enum gen_elt_type type, int i, int max_elts) {
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
void th_printe(const void *e) {
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
bool_t th_filter_func(const void *const e) {
  const T* q = reinterpret_cast<const T*>(e);
  return (bool_t)(q->value1 % 2 == 0);
} /* th_filter_func() */

/**
 * \brief Compare two elements (any data structure)
 *
 * \return  < 0 if e1 < e2, 0 if e1 == e2, > 0 if e2 > e1
 */
template<typename T>
int th_cmpe(const void *const e1, const void *const e2) {
  const T* q1 = reinterpret_cast<const T*>(e1);
  const T* q2 = reinterpret_cast<const T*>(e2);

  if (q1->value1 < q2->value1) {
    return -1;
  } else if (q1->value1 == q2->value1) {
    return 0;
  }
  return 1;
} /* th_cmpe() */

/**
 * \brief Compare the data of two hashnodes
 */
template<typename T>
int th_data_cmp(const void *a, const void *b) {
  const struct hashnode *q1 = reinterpret_cast<const T*>(a);
  const struct hashnode *q2 = reinterpret_cast<const T*>(b);
  const T* d1 = reinterpret_cast<const T*>(q1->data);
  const T* d2 = reinterpret_cast<const T*>(q2->data);
  return d1->value1 - d2->value2;
} /* th_data_cmp() */

/**
 * \brief Iterate with a cumulative SOMETHING
 */
template<typename T>
void th_inject_func(void *e, void *res) {
  T* e1 = reinterpret_cast<T*>(e);
  int *tmp = reinterpret_cast<int*>(res);
  *tmp += e1->value1;
} /* th_inject_func() */

/**
 * \brief Map an element (i.e. do something to it)
 */
template<typename T>
void th_map_func(void *e) {
  T *e1 = reinterpret_cast<T*>(e);
  e1->value1--;
} /* th_map_func() */

/**
 * \brief Classify an element for iteration
 *
 * RETURN:
 *     int - 1 if should be returned, 0 if not
 */
template<typename T>
bool_t th_iter_func(void *e) {
  T* e1 = reinterpret_cast<T*>(e);
  return (bool_t)(e1->value1 % 2 == 0);
} /* th_iter_func() */
