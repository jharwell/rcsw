/**
 * \file
 *
 * \copyright 2023 John Harwell
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "tests/unit/element.h"
#include <vector>
#include <stdlib.h>
#include <cassert>
#include <random>

#include <catch2/catch_get_random_seed.hpp>

/******************************************************************************
 * Namespaces/Decls
 ******************************************************************************/
namespace th {

/*******************************************************************************
 * Structure Definitions
 ******************************************************************************/
enum gen_elt_type { INC_VALS, DEC_VALS, RAND_VALS, PACKED_VALS };


static inline std::mt19937& make_rng() {
  static auto rng =  std::mt19937{Catch::getSeed()};
  return rng;
}


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
    if (INC_VALS == m_type) {
      e.value1 = m_i;
    } else if (DEC_VALS == m_type) {
      e.value1 = m_max_elts - m_i;
    } else {
      auto dist = std::uniform_int_distribution<size_t>(0, m_max_elts - 1);
      e.value1 = dist(make_rng());
    }
    ++m_i;
    return e;
  }

  template <typename U = T,
            typename std::enable_if<!std::is_same<U, element1>::value,
                                    int>::type = 0>
  T next(void) {
    T e{};
    assert(m_max_elts > 0);
    e.value2 = 17;
    if (INC_VALS == m_type) {
      e.value1 = m_i;
    } else if (DEC_VALS == m_type) {
      e.value1 = m_max_elts - m_i;
    } else if (PACKED_VALS == m_type) {
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
    element_generator<T> g(gen_elt_type::DEC_VALS, elts.size());
    for (size_t i = 0; i < elts.size(); ++i) {
      elts[i] = g.next();
    } /* for(i..) */
  }
  std::vector<T> elts;
};

/******************************************************************************
 * Public API
 ******************************************************************************/
/**
 * \brief Generate an element.
 */
template<typename T>
T gen_elt(enum gen_elt_type type, int i, int max_elts) {
  T e;
  e.value2 = -1;
  if (INC_VALS == type) {
    e.value1 = i;
  } else if (DEC_VALS == type) {
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
 * \brief Accumulate each element's value1 into \p res (an int).
 *
 * The sum wraps on overflow (computed unsigned), since element values may be
 * arbitrary.
 */
template<typename T>
void inject_func(void *e, void *res) {
  T* e1 = reinterpret_cast<T*>(e);
  int *tmp = reinterpret_cast<int*>(res);
  *tmp = static_cast<int>(static_cast<unsigned>(*tmp) +
                          static_cast<unsigned>(e1->value1));
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
template<typename T>
auto operator<(const T& lhs, const T& rhs) -> bool {
  return lhs.value1 < rhs.value1;
}

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
