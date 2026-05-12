/**
 * \file element.h
 *
 * \copyright 2023 John Harwell, All rights reserved.
 *
 * SPDX-License Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "tests/ds/element.h"

/******************************************************************************
 * Namespaces/Decls
 ******************************************************************************/
namespace th {
  
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
} /* namespace th */

/*******************************************************************************
 * Operators
 ******************************************************************************/
static inline auto operator==(const element8& lhs, const element8 &rhs) -> bool {
  return lhs.value1 == rhs.value1 && lhs.value2 == rhs.value2;
}

static inline auto operator==(const element4 &lhs, const element4 &rhs) -> bool {
  return lhs.value1 == rhs.value1 && lhs.value2 == rhs.value2;
}

static inline auto operator==(const element2 &lhs, const element2 &rhs) -> bool {
  return lhs.value1 == rhs.value1 && lhs.value2 == rhs.value2;
}

static inline auto operator==(const element1 &lhs, const element1 &rhs) -> bool {
  return lhs.value1 == rhs.value1;
}
