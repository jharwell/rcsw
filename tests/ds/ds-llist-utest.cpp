/**
 * \file ds-llist-utest.cpp
 *
 * \copyright 2017 John Harwell, All rights reserved.
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#define CATCH_CONFIG_PREFIX_ALL
#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <vector>

#include "rcsw/ds/llist.h"
#include "tests/ds/ds_test.hpp"

/*******************************************************************************
 * Test Runner
 ******************************************************************************/
template <typename T>
static void run_test(void (*test)(int len, struct llist_config* config)) {
  struct llist_config config;
  memset(&config, 0, sizeof(llist_config));
  config.cmpe     = th::cmpe<T>;
  config.printe   = th::printe<T>;
  config.elt_size = sizeof(T);
  CATCH_REQUIRE(th::ds_init(&config) == OK);

  uint32_t flags[] = {
    RCSW_ZALLOC,
    RCSW_NOALLOC_HANDLE,
    RCSW_NOALLOC_DATA,
    RCSW_NOALLOC_META,
  };
  run_test_flags(config, flags, RCSW_ARRAY_ELTS(flags), TH_NUM_ITEMS, test);
  th::ds_shutdown(&config);
}

/*******************************************************************************
 * Test Functions
 ******************************************************************************/
template <typename T>
static void insert_test(int len, struct llist_config* config) {
  struct llist* list;
  struct llist  mylist;

  if (config->flags & RCSW_NOALLOC_HANDLE) {
    CATCH_REQUIRE(nullptr == llist_init(nullptr, config));
    list = llist_init(&mylist, config);
  } else {
    list = llist_init(nullptr, config);
  }
  CATCH_REQUIRE(nullptr != list);

  /* empty state */
  CATCH_REQUIRE(llist_isempty(list));
  CATCH_REQUIRE(llist_size(list) == 0);

  th::element_generator<T> g(th::gen_elt_type::ekRAND_VALS, config->max_elts);
  std::vector<T>           inserted;

  for (int i = 0; i < len; i++) {
    T e = g.next();
    if (rand() % 2) {
      CATCH_REQUIRE(llist_append(list, &e) == OK);
    } else {
      CATCH_REQUIRE(llist_prepend(list, &e) == OK);
    }
    inserted.push_back(e);
    for (auto& el : inserted) {
      CATCH_REQUIRE(llist_data_query(list, &el));
    }
  }

  /* full: append/prepend nullptr rejected */
  if (len == config->max_elts) {
  CATCH_REQUIRE(llist_isfull(list));
  }
  CATCH_REQUIRE(llist_append(list, nullptr) == ERROR);
  CATCH_REQUIRE(llist_prepend(list, nullptr) == ERROR);

  llist_destroy(list);
  CATCH_REQUIRE(th::leak_check_data(config) == 0);
  CATCH_REQUIRE(th::leak_check_nodes(config) == 0);
}

template <typename T>
static void remove_if_test(int len, struct llist_config* config) {
  struct llist* list;
  struct llist  mylist;

  if (config->flags & RCSW_NOALLOC_HANDLE) {
    list = llist_init(&mylist, config);
  } else {
    list = llist_init(nullptr, config);
  }
  CATCH_REQUIRE(nullptr != list);

  th::element_generator<T> g(th::gen_elt_type::ekRAND_VALS, config->max_elts);
  std::vector<T>           inserted;
  for (int i = 0; i < len; i++) {
    T e = g.next();
    CATCH_REQUIRE(llist_append(list, &e) == OK);
    inserted.push_back(e);
  }

  CATCH_REQUIRE(llist_remove_if(list, th::filter_func<T>) == OK);

  /* matching elements removed, non-matching remain */
  for (auto& e : inserted) {
    if (th::filter_func<T>(&e)) {
      CATCH_REQUIRE(llist_node_query(list, &e) == nullptr);
    } else {
      CATCH_REQUIRE(llist_node_query(list, &e) != nullptr);
    }
  }

  llist_destroy(list);
  CATCH_REQUIRE(th::leak_check_data(config) == 0);
  CATCH_REQUIRE(th::leak_check_nodes(config) == 0);
}

template <typename T>
static void copy_test(int len, struct llist_config* config) {
  struct llist* list1;
  struct llist* list2;
  struct llist  mylist;

  if (config->flags & RCSW_NOALLOC_HANDLE) {
    list1 = llist_init(&mylist, config);
  } else {
    list1 = llist_init(nullptr, config);
  }
  CATCH_REQUIRE(nullptr != list1);

  th::element_generator<T> g(th::gen_elt_type::ekRAND_VALS, config->max_elts);
  std::vector<T>           inserted;
  for (int i = 0; i < len; i++) {
    T e = g.next();
    CATCH_REQUIRE(llist_append(list1, &e) == OK);
    inserted.push_back(e);
  }

  /* copy_if: only elements satisfying filter_func */
  list2 = llist_copy_if(list1, th::filter_func<T>, 0, nullptr, nullptr);
  CATCH_REQUIRE(nullptr != list2);

  for (auto& e : inserted) {
    if (th::filter_func<T>(&e)) {
      CATCH_REQUIRE(llist_node_query(list2, &e) != nullptr);
    }
    /* original list unchanged */
    CATCH_REQUIRE(llist_node_query(list1, &e) != nullptr);
  }
  llist_destroy(list2);

  /* full copy: all elements present in both */
  list2 = llist_copy(list1, 0, nullptr, nullptr);
  CATCH_REQUIRE(nullptr != list2);
  for (auto& e : inserted) {
    CATCH_REQUIRE(llist_node_query(list1, &e) != nullptr);
    CATCH_REQUIRE(llist_node_query(list2, &e) != nullptr);
  }
  llist_destroy(list2);

  llist_destroy(list1);
  CATCH_REQUIRE(th::leak_check_data(config) == 0);
  CATCH_REQUIRE(th::leak_check_nodes(config) == 0);
}

template <typename T>
static void sort_test(int len, struct llist_config* config) {
  struct llist* list;
  struct llist  mylist;

  if (config->flags & RCSW_NOALLOC_HANDLE) {
    list = llist_init(&mylist, config);
  } else {
    list = llist_init(nullptr, config);
  }
  CATCH_REQUIRE(nullptr != list);

  for (int i = 0; i < len; i++) {
    T   e;
    e.value1 = rand() % (i + 1) + i;
    CATCH_REQUIRE(llist_append(list, &e) == OK);
  }

  CATCH_REQUIRE(OK == llist_sort(list, (exec_type)(rand() % 2)));

  /* verify sorted */
  int val = -1;
  LLIST_FOREACH(list, next, curr) {
    T* e = (T*)curr->data;
    CATCH_REQUIRE(val <= e->value1);
    val = e->value1;
  }

  llist_destroy(list);
  CATCH_REQUIRE(th::leak_check_data(config) == 0);
  CATCH_REQUIRE(th::leak_check_nodes(config) == 0);
}

template <typename T>
static void inject_test(int len, struct llist_config* config) {
  struct llist* list;
  struct llist  mylist;

  if (config->flags & RCSW_NOALLOC_HANDLE) {
    list = llist_init(&mylist, config);
  } else {
    list = llist_init(nullptr, config);
  }
  CATCH_REQUIRE(nullptr != list);

  /* NULL callback rejected */
  int dummy = 0;
  CATCH_REQUIRE(ERROR == llist_inject(list, nullptr, &dummy));

  th::element_generator<T> g(th::gen_elt_type::ekINC_VALS, config->max_elts);
  int expected_sum = 0;
  for (int i = 0; i < len; i++) {
    T e = g.next();
    expected_sum += i;
    CATCH_REQUIRE(llist_append(list, &e) == OK);
  }

  int total = 0;
  CATCH_REQUIRE(llist_inject(list, th::inject_func<T>, &total) == OK);
  CATCH_REQUIRE(total == expected_sum);

  llist_destroy(list);
}

template <typename T>
static void iter_test(int len, struct llist_config* config) {
  struct llist* list;
  struct llist  mylist;

  if (config->flags & RCSW_NOALLOC_HANDLE) {
    list = llist_init(&mylist, config);
  } else {
    list = llist_init(nullptr, config);
  }
  CATCH_REQUIRE(nullptr != list);

  th::element_generator<T> g(th::gen_elt_type::ekINC_VALS, config->max_elts);
  for (int i = 0; i < len; i++) {
    T e = g.next();
    CATCH_REQUIRE(llist_append(list, &e) == OK);
  }

  T* e;
  struct ds_iterator iter;

  /* filtered forward */
  CATCH_REQUIRE(nullptr != llist_iter_init(&iter, list, ekITER_FORWARD,
                                           th::iter_func_even<T>));
  while ((e = (T*)ds_iter_next(&iter)) != nullptr) {
    CATCH_REQUIRE(e->value1 % 2 == 0);
  }

  /* unfiltered forward */
  CATCH_REQUIRE(nullptr != llist_iter_init(&iter, list, ekITER_FORWARD,
                                           th::iter_func_all<T>));
  size_t count = 0;
  while ((e = (T*)ds_iter_next(&iter)) != nullptr) {
    CATCH_REQUIRE((size_t)e->value1 == count);
    count++;
  }
  CATCH_REQUIRE(count == list->current);

  /* unfiltered backward: values in reverse order */
  CATCH_REQUIRE(nullptr != llist_iter_init(&iter, list, ekITER_BACKWARD,
                                           th::iter_func_all<T>));
  count = 0;
  while ((e = (T*)ds_iter_next(&iter)) != nullptr) {
    CATCH_REQUIRE((size_t)e->value1 == (size_t)len - count - 1);
    count++;
  }
  CATCH_REQUIRE(count == list->current);

  /* two independent iterators */
  struct ds_iterator iter2;
  CATCH_REQUIRE(nullptr != llist_iter_init(&iter,  list, ekITER_FORWARD,
                                           th::iter_func_all<T>));
  CATCH_REQUIRE(nullptr != llist_iter_init(&iter2, list, ekITER_FORWARD,
                                           th::iter_func_all<T>));
  T* a = (T*)ds_iter_next(&iter);
  T* b = (T*)ds_iter_next(&iter2);
  if (len > 0) {
    CATCH_REQUIRE(a != nullptr);
    CATCH_REQUIRE(b != nullptr);
    CATCH_REQUIRE(th::cmpe<T>(a, b) == 0);
  }

  llist_destroy(list);
}

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("llist Insert Test", "[ds][llist]") {
  run_test<element8>(insert_test<element8>);
  run_test<element4>(insert_test<element4>);
  run_test<element2>(insert_test<element2>);
  run_test<element1>(insert_test<element1>);
}
CATCH_TEST_CASE("llist Remove_if Test", "[ds][llist]") {
  run_test<element8>(remove_if_test<element8>);
  run_test<element4>(remove_if_test<element4>);
  run_test<element2>(remove_if_test<element2>);
  run_test<element1>(remove_if_test<element1>);
}
CATCH_TEST_CASE("llist Copy Test", "[ds][llist]") {
  run_test<element8>(copy_test<element8>);
  run_test<element4>(copy_test<element4>);
  run_test<element2>(copy_test<element2>);
  run_test<element1>(copy_test<element1>);
}
CATCH_TEST_CASE("llist Sort Test", "[ds][llist]") {
  run_test<element8>(sort_test<element8>);
  run_test<element4>(sort_test<element4>);
  run_test<element2>(sort_test<element2>);
  run_test<element1>(sort_test<element1>);
}
CATCH_TEST_CASE("llist Inject Test", "[ds][llist]") {
  run_test<element8>(inject_test<element8>);
  run_test<element4>(inject_test<element4>);
  run_test<element2>(inject_test<element2>);
  run_test<element1>(inject_test<element1>);
}
CATCH_TEST_CASE("llist Iterator Test", "[ds][llist]") {
  run_test<element8>(iter_test<element8>);
  run_test<element4>(iter_test<element4>);
  run_test<element2>(iter_test<element2>);
  run_test<element1>(iter_test<element1>);
}
