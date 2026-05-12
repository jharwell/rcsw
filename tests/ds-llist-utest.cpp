/**
 * \file llist-test.cpp
 *
 * Test of linked list module.
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#define CATCH_CONFIG_PREFIX_ALL
#include <catch2/catch_test_macros.hpp>

#include "rcsw/ds/llist.h"
#include "tests/ds_test.h"
#include "tests/ds_test.hpp"

/*******************************************************************************
 * Namespaces/Decls
 ******************************************************************************/
using llist_test1_t = void (*)(int len, struct llist_config* config);
using llist_test2_t = void (*)(int len1, int len2, struct llist_config* config);

/*******************************************************************************
 * Test Helper Functions
 ******************************************************************************/
template <typename T>
static void run_test(llist_test1_t test) {
  RCSW_ER_INIT(TH_ZLOG_CONF);

  struct llist_config config;
  memset(&config, 0, sizeof(llist_config));
  config.flags    = 0;
  config.cmpe     = th::cmpe<T>;
  config.printe   = th::printe<T>;
  config.elt_size = sizeof(T);
  CATCH_REQUIRE(th::ds_init(&config) == OK);

  uint32_t flags[] = {
    RCSW_NONE,
    RCSW_ZALLOC,
    RCSW_NOALLOC_HANDLE,
    RCSW_NOALLOC_DATA,
    RCSW_NOALLOC_META,
  };

  uint32_t applied = 0;
  for (size_t i = 0; i < RCSW_ARRAY_ELTS(flags); ++i) {
    applied |= flags[i];
    for (size_t j = i + 1; j < RCSW_ARRAY_ELTS(flags); ++j) {
      applied |= flags[j];

      for (size_t m = 3; m <= TH_NUM_ITEMS; ++m) {
        config.flags    = applied;
        config.max_elts = m;
        test(m, &config);
      } /* for(m..) */

      applied &= ~flags[j];
    } /* for(j..) */
  } /* for(i..) */

  th::ds_shutdown(&config);
  RCSW_ER_DEINIT();
} /* run_test() */

template <typename T>
static void run_test2(llist_test2_t test) {
  struct llist_config config;
  memset(&config, 0, sizeof(llist_config));
  config.flags    = 0;
  config.cmpe     = th::cmpe<T>;
  config.printe   = th::printe<T>;
  config.elt_size = sizeof(T);
  CATCH_REQUIRE(th::ds_init(&config) == OK);

  uint32_t flags[] = {
    RCSW_NONE,
    RCSW_ZALLOC,
    RCSW_NOALLOC_HANDLE,
    RCSW_NOALLOC_DATA,
    RCSW_NOALLOC_META,
  };

  uint32_t applied = 0;
  for (size_t i = 0; i < RCSW_ARRAY_ELTS(flags); ++i) {
    applied |= flags[i];
    for (size_t j = i + 1; j < RCSW_ARRAY_ELTS(flags); ++j) {
      applied |= flags[j];

      for (size_t k = 2; k <= TH_NUM_ITEMS; ++k) {
        for (size_t m = 1; m <= TH_NUM_ITEMS; ++m) {
          config.flags    = applied;
          config.max_elts = m + k;
          test(k, m, &config);
        } /* for(m..) */
      } /* for(k..) */
      applied &= ~flags[j];
    } /* for(j..) */
  } /* for(i..) */

  th::ds_shutdown(&config);
} /* run_test() */

/*******************************************************************************
 * Test Functions
 ******************************************************************************/
/**
 * \brief Test appending/prepending items into a linked list
 */
template <typename T>
static void insert_test(int len, struct llist_config* config) {
  struct llist* list;
  struct llist  mylist;

  T arr[TH_NUM_ITEMS];

  if (config->flags & RCSW_NOALLOC_HANDLE) {
    llist_print(nullptr);
    CATCH_REQUIRE(nullptr == llist_init(nullptr, config));
    list = llist_init(&mylist, config);
    llist_print(&mylist);
  } else {
    list = llist_init(nullptr, config);
  }
  CATCH_REQUIRE(nullptr != list);

  th::element_generator<T> g(gen_elt_type::ekRAND_VALS, config->max_elts);
  for (int i = 0; i < len; i++) {
    T e = g.next();

    if (rand() % 2) {
      CATCH_REQUIRE(llist_append(list, &e) == OK);
    } else {
      CATCH_REQUIRE(llist_prepend(list, &e) == OK);
    }
    arr[i] = e;

    /* verify as we go */
    for (int j = 0; j <= i; ++j) {
      CATCH_REQUIRE(llist_data_query(list, &arr[j]));
    }
  }

  CATCH_REQUIRE(llist_isfull(list));
  CATCH_REQUIRE(llist_append(list, nullptr) == ERROR);
  CATCH_REQUIRE(llist_prepend(list, nullptr) == ERROR);

  if (!(list->flags & RCSW_NOALLOC_ALL)) {
    CATCH_REQUIRE(llist_heap_footprint(list) == 0);
  } else {
    CATCH_REQUIRE(llist_heap_footprint(list) > 0);
  }
  llist_destroy(list);

  /* verify all DS_APP_DOMAIN data deallocated */
  CATCH_REQUIRE(th::leak_check_data(config) == 0);
  CATCH_REQUIRE(th::leak_check_nodes(config) == 0);
} /* insert_test () */

/**
 * \brief Test clearing lists of different sizes
 */
template <typename T>
static void clear_test(int len, struct llist_config* config) {
  struct llist* list;
  struct llist  mylist;

  list = llist_init(&mylist, config);
  CATCH_REQUIRE(nullptr != list);

  th::element_generator<T> g(gen_elt_type::ekRAND_VALS, config->max_elts);
  for (int i = 1; i <= len; i++) {
    T e = g.next();
    CATCH_REQUIRE(llist_append(list, &e) == OK);
  } /* for() */

  llist_clear(list);
  CATCH_REQUIRE((llist_isempty(list) && (list->first == nullptr)));

  llist_destroy(list);

  /* verify all DS_APP_DOMAIN data deallocated */
  CATCH_REQUIRE(th::leak_check_data(config) == 0);
  CATCH_REQUIRE(th::leak_check_nodes(config) == 0);
} /* clear_test() */

/**
 * \brief Test deleting lists of different sizes
 */
template <typename T>
static void delete_test(int len, struct llist_config* config) {
  struct llist* list;
  struct llist  mylist;

  list = llist_init(&mylist, config);
  CATCH_REQUIRE(nullptr != list);

  th::element_generator<T> g(gen_elt_type::ekRAND_VALS, config->max_elts);
  for (int i = 1; i <= len; i++) {
    T e = g.next();

    CATCH_REQUIRE(llist_append(list, &e) == OK);
  } /* for() */

  llist_destroy(list);

  /* verify all DS_APP_DOMAIN data deallocated */
  CATCH_REQUIRE(th::leak_check_data(config) == 0);
  CATCH_REQUIRE(th::leak_check_nodes(config) == 0);
} /* delete_test() */

/**
 * \brief Test of \ref llist_copy()
 */
template <typename T>
static void copy_test(int len, struct llist_config* config) {
  struct llist *list1, *list2;
  struct llist  mylist;
  T             arr[TH_NUM_ITEMS];

  list1 = llist_init(&mylist, config);
  CATCH_REQUIRE(nullptr != list1);

  th::element_generator<T> g(gen_elt_type::ekRAND_VALS, config->max_elts);
  for (int i = 0; i < len; i++) {
    T e = g.next();

    CATCH_REQUIRE(llist_append(list1, &e) == OK);
    CATCH_REQUIRE(llist_node_query(list1, &e) != nullptr);
    arr[i] = e;
  } /* for() */

  if (config->flags &
      RCSW_NOALLOC_ALL) {
    list2 = llist_copy(list1, 0, nullptr, nullptr);
    CATCH_REQUIRE(nullptr != list2);

    for (int i = 0; i < len; i++) {
      CATCH_REQUIRE(llist_node_query(list1, &arr[i]) != nullptr);
      CATCH_REQUIRE(llist_node_query(list2, &arr[i]) != nullptr);
    } /* for() */
    llist_destroy(list2);
  }

  llist_destroy(list1);

  /* verify all DS_APP_DOMAIN data deallocated */
  CATCH_REQUIRE(th::leak_check_data(config) == 0);
  CATCH_REQUIRE(th::leak_check_nodes(config) == 0);
} /* copy_test() */

/**
 * \brief Test of \ref llist_copy2()
 *
 */
template <typename T>
static void copy2_test(int len, struct llist_config* config) {
  struct llist *list1, *list2;
  struct llist  mylist;
  T             arr[TH_NUM_ITEMS];

  if (config->flags & RCSW_NOALLOC_HANDLE) {
    list1 = llist_init(&mylist, config);
  } else {
    list1 = llist_init(nullptr, config);
  }

  CATCH_REQUIRE(nullptr != list1);

  th::element_generator<T> g(gen_elt_type::ekRAND_VALS, config->max_elts);
  for (int i = 0; i < len; i++) {
    T e = g.next();
    CATCH_REQUIRE(llist_append(list1, &e) == OK);
    arr[i] = e;
  } /* for() */

  if ((config->flags &
       RCSW_NOALLOC_ALL)) {
    list2 = llist_copy_if(list1, th::filter_func<T>, 0, nullptr, nullptr);
    CATCH_REQUIRE(nullptr != list2);

    for (int i = 0; i < len; i++) {
      if (th::filter_func<T>(&arr[i])) {
        CATCH_REQUIRE(nullptr != llist_node_query(list2, &arr[i]));
      }
      CATCH_REQUIRE(nullptr != llist_node_query(list1, &arr[i]));
    } /* for() */
    llist_destroy(list2);
  }

  llist_destroy(list1);

  /* verify all DS_APP_DOMAIN data deallocated */
  CATCH_REQUIRE(th::leak_check_data(config) == 0);
  CATCH_REQUIRE(th::leak_check_nodes(config) == 0);
} /* copy2_test() */

/**
 * \brief Test of \ref llist_filter()
 */
template <typename T>
static void filter_test(int len, struct llist_config* config) {
  struct llist *list1, *list2;
  struct llist  mylist;

  T arr[TH_NUM_ITEMS];

  if (config->flags & RCSW_NOALLOC_HANDLE) {
    list1 = llist_init(&mylist, config);
  } else {
    list1 = llist_init(nullptr, config);
  }

  CATCH_REQUIRE(nullptr != list1);

  th::element_generator<T> g(gen_elt_type::ekRAND_VALS, config->max_elts);
  for (int i = 0; i < len; i++) {
    T e = g.next();
    CATCH_REQUIRE(llist_append(list1, &e) == OK);
    arr[i] = e;
  } /* for() */

  if ((config->flags &
       RCSW_NOALLOC_ALL)) {
    list2 = llist_filter(list1, th::filter_func<T>, 0, nullptr, nullptr);
    CATCH_REQUIRE(nullptr != list2);

    for (int i = 0; i < len; i++) {
      if (th::filter_func<T>(&arr[i])) {
        CATCH_REQUIRE(llist_node_query(list2, &arr[i]) != nullptr);
        CATCH_REQUIRE(llist_node_query(list1, &arr[i]) == nullptr);
      }
    } /* for() */
    llist_destroy(list2);
  }

  llist_destroy(list1);

  /* verify all DS_APP_DOMAIN data deallocated */
  CATCH_REQUIRE(th::leak_check_data(config) == 0);
  CATCH_REQUIRE(th::leak_check_nodes(config) == 0);
} /* filter_test() */

/**
 * \brief Test of \ref llist_filter2()
 */
template <typename T>
static void filter2_test(int len, struct llist_config* config) {
  struct llist* list1;
  struct llist  mylist;
  T             arr[TH_NUM_ITEMS];

  if (config->flags & RCSW_NOALLOC_HANDLE) {
    list1 = llist_init(&mylist, config);
  } else {
    list1 = llist_init(nullptr, config);
  }

  CATCH_REQUIRE(nullptr != list1);

  th::element_generator<T> g(gen_elt_type::ekRAND_VALS, config->max_elts);
  for (int i = 0; i < len; i++) {
    T e = g.next();
    CATCH_REQUIRE(llist_append(list1, &e) == OK);
    arr[i] = e;
  } /* for() */

  CATCH_REQUIRE(llist_remove_if(list1, th::filter_func<T>) == OK);

  for (int i = 0; i < len; i++) {
    if (th::filter_func<T>(&arr[i])) {
      CATCH_REQUIRE(llist_node_query(list1, &arr[i]) == nullptr);
    }
  } /* for() */

  llist_destroy(list1);

  /* verify all DS_APP_DOMAIN data deallocated */
  CATCH_REQUIRE(th::leak_check_data(config) == 0);
  CATCH_REQUIRE(th::leak_check_nodes(config) == 0);
} /* filter2_test() */

/**
 * \brief Test of \ref llist_sort()
 */
template <typename T>
static void sort_test(int len, struct llist_config* config) {
  struct llist* list1;
  struct llist  mylist;

  int i;
  if (config->flags & RCSW_NOALLOC_HANDLE) {
    list1 = llist_init(&mylist, config);
  } else {
    list1 = llist_init(nullptr, config);
  }

  CATCH_REQUIRE(nullptr != list1);

  for (i = 0; i < len; i++) {
    T   e;
    int value = rand() % (i + 1) + i;
    e.value1  = value;
    CATCH_REQUIRE(llist_append(list1, &e) == OK);
  } /* for() */

  CATCH_REQUIRE(OK == llist_sort(list1, (exec_type)(rand() % 2)));

  T*  e;
  int val = -1;

  /* verify list is sorted */
  LLIST_FOREACH(list1, next, curr) {
    e = (T*)curr->data;
    CATCH_REQUIRE(val <= e->value1);
    val = e->value1;
  }

  llist_destroy(list1);

  /* verify all DS_APP_DOMAIN data deallocated */
  CATCH_REQUIRE(th::leak_check_data(config) == 0);
  CATCH_REQUIRE(th::leak_check_nodes(config) == 0);
} /* sort_test() */

/**
 * \brief Test of \ref llist_splice()
 */
template <typename T>
static void splice_test(int len1, int len2, struct llist_config* config) {
  struct llist* list1 = nullptr;
  struct llist* list2 = nullptr;
  struct llist  mylist1;
  struct llist  mylist2;

  int                i;
  int                arr1[TH_NUM_ITEMS];
  int                arr2[TH_NUM_ITEMS];
  struct llist_node* splice_node = nullptr;
  int                splice      = (rand() + len1 / 2) % len1;

  if (config->flags & RCSW_NOALLOC_HANDLE) {
    list1 = llist_init(&mylist1, config);
  } else {
    list1 = llist_init(nullptr, config);
  }

  if (config->flags & RCSW_NOALLOC_HANDLE) {
    list2 = llist_init(&mylist2, config);
  } else {
    list2 = llist_init(nullptr, config);
  }

  CATCH_REQUIRE(nullptr != list1);
  CATCH_REQUIRE(nullptr != list2);

  /* fill list1 */
  for (i = 0; i < len1; i++) {
    T   e;
    int value = i + 1;
    e.value1  = value;
    CATCH_REQUIRE(llist_append(list1, &e) == OK);
    if (splice == i) {
      splice_node = llist_node_query(list1, &e);
    }
    arr1[i] = value;
  } /* for() */

  /* fill list2 */
  for (i = 0; i < len2; i++) {
    T   e;
    int value = i + 3;
    e.value1  = value;
    CATCH_REQUIRE(llist_append(list2, &e) == OK);
    arr2[i] = value;
  } /* for() */

  /*
   * If list1/list2 have 1 or 2 items, you are appending/prepending to the list,
   * which requires different validation than the general case
   */

  if (len1 < 3 || len2 < 3 || splice_node == list1->first ||
      splice_node == list1->last) {
    if (rand() % 2) { /* prepend */
      CATCH_REQUIRE(llist_splice(list1, list2, list1->first) == OK);
      int count = 0;
      LLIST_FOREACH(list1, next, curr) {
        if (count < len2) {
          CATCH_REQUIRE(((T*)curr->data)->value1 == arr2[count]);
        } else {
          CATCH_REQUIRE(((T*)curr->data)->value1 == arr1[count - len2]);
        }
        count++;
      }
    } else { /* append */
      CATCH_REQUIRE(llist_splice(list1, list2, list1->last) == OK);
      int count = 0;
      LLIST_FOREACH(list1, next, curr) {
        if (count < len1) {
          CATCH_REQUIRE(((T*)curr->data)->value1 == arr1[count]);
        } else {
          CATCH_REQUIRE(((T*)curr->data)->value1 == arr2[count - len1]);
        }
        count++;
      }
    }
  } else { /* general case */
    CATCH_REQUIRE(llist_splice(list1, list2, splice_node) == OK);
    int count = 0;
    LLIST_FOREACH(list1, next, curr) {
      if (count < splice) {
        CATCH_REQUIRE(((T*)curr->data)->value1 == arr1[count]);
      } else if (count < splice + len2) {
        CATCH_REQUIRE(((T*)curr->data)->value1 == arr2[count - splice]);
      } else {
        CATCH_REQUIRE(((T*)curr->data)->value1 == arr1[count - len2]);
      }
      count++;
    }
  }

  llist_destroy(list1);

  /* verify all DS_APP_DOMAIN data deallocated */
  CATCH_REQUIRE(th::leak_check_data(config) == 0);
  CATCH_REQUIRE(th::leak_check_nodes(config) == 0);
} /* splice_test() */

/**
 * \brief Test sharing llist_nodes between linked lists
 */
template <typename T>
static void pool_test(int len, struct llist_config* config) {
  struct llist *list1, *list2;
  int           i;

  if (!(config->flags & RCSW_NOALLOC_DATA)) {
    return;
  }

  config->flags |= RCSW_DS_LLIST_DB_DISOWN | RCSW_DS_LLIST_DB_PTR;
  config->flags &= ~RCSW_NOALLOC_HANDLE;

  list1 = llist_init(nullptr, config);
  list2 = llist_init(nullptr, config);

  CATCH_REQUIRE(nullptr != list1);
  CATCH_REQUIRE(nullptr != list2);

  /* fill list1 by carving up the space provided for data */
  for (i = 0; i < len; i++) {
    CATCH_REQUIRE(llist_append(list1, config->elements + i * sizeof(T)) == OK);
  } /* for() */

  /* Remove each element from list1, add it to list2 */
  for (i = 0; i < len; ++i) {
    CATCH_REQUIRE(llist_remove(list1, config->elements + i * sizeof(T)) == OK);
    struct llist_node* node =
      llist_node_query(list1, config->elements + i * sizeof(T));
    CATCH_REQUIRE(node == nullptr);
    CATCH_REQUIRE(llist_append(list2, config->elements + i * sizeof(T)) == OK);
    node = llist_node_query(list2, config->elements + i * sizeof(T));
    CATCH_REQUIRE(nullptr != node);
  } /* for() */

  CATCH_REQUIRE(llist_isempty(list1));
  CATCH_REQUIRE(llist_remove(list1, config->elements) == ERROR);

  /* Remove each element from list2, add it back to list1 */
  for (i = 0; i < len; ++i) {
    llist_remove(list2, config->elements + i * sizeof(T));
    struct llist_node* node =
      llist_node_query(list2, config->elements + i * sizeof(T));
    CATCH_REQUIRE(node == nullptr);
    CATCH_REQUIRE(llist_append(list1, config->elements + i * sizeof(T)) == OK);
    node = llist_node_query(list1, config->elements + i * sizeof(T));
    CATCH_REQUIRE(nullptr != node);
  } /* for() */

  CATCH_REQUIRE(llist_isempty(list2));

  llist_destroy(list1);
  llist_destroy(list2);

  config->flags &= ~RCSW_DS_LLIST_DB_DISOWN;

  /* verify all DS_APP_DOMAIN data deallocated */
  CATCH_REQUIRE(th::leak_check_data(config) == 0);
  CATCH_REQUIRE(th::leak_check_nodes(config) == 0);
} /* pool_test() */

/**
 * \brief Test of \ref llist_inject()
 */
template <typename T>
static void inject_test(int len, struct llist_config* config) {
  struct llist* list;
  struct llist  mylist;
  int           sum = 0;

  if (config->flags & RCSW_NOALLOC_HANDLE) {
    list = llist_init(&mylist, config);
  } else {
    list = llist_init(nullptr, config);
  }
  CATCH_REQUIRE(nullptr != list);

  th::element_generator<T> g(gen_elt_type::ekINC_VALS, config->max_elts);
  for (int i = 0; i < len; i++) {
    T e = g.next();
    sum += i;
    CATCH_REQUIRE(llist_append(list, &e) == OK);
  } /* for() */

  int total = 0;
  CATCH_REQUIRE(llist_inject(list, th::inject_func<T>, &total) == OK);
  CATCH_REQUIRE(total == sum);

  llist_destroy(list);
} /* inject_test() */

/**
 * \brief Test of linked list iteration
 */
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

  th::element_generator<T> g(gen_elt_type::ekINC_VALS, config->max_elts);

  /* for easy comparison when reverse iterating */

  for (int i = 0; i < len; i++) {
    T e = g.next();
    CATCH_REQUIRE(llist_append(list, &e) == OK);
  } /* for() */

  T* e;

  struct ds_iterator iter;
  CATCH_REQUIRE(nullptr != llist_iter_init(&iter, list, ekITER_FORWARD, th::iter_func_even<T>));

  while ((e = (T*)ds_iter_next(&iter)) != nullptr) {
    CATCH_REQUIRE(e->value1 % 2 == 0);
  }

  CATCH_REQUIRE(nullptr != llist_iter_init(&iter, list, ekITER_FORWARD, th::iter_func_all<T>));
  size_t count = 0;

  while ((e = (T*)ds_iter_next(&iter)) != nullptr) {
    CATCH_REQUIRE((size_t)e->value1 == count);
    count++;
  }
  CATCH_REQUIRE(count == list->current);

  CATCH_REQUIRE(nullptr != llist_iter_init(&iter, list, ekITER_BACKWARD, th::iter_func_all<T>));
  count = 0;
  while ((e = (T*)ds_iter_next(&iter)) != nullptr) {
    CATCH_REQUIRE((size_t)e->value1 == config->max_elts - count - 1);
    count++;
  }
  CATCH_REQUIRE(count == list->current);

  llist_destroy(list);
} /* iter_test() */

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("llist Insert Test", "[ds][llist]") {
  run_test<element8>(insert_test<element8>);
  run_test<element4>(insert_test<element4>);
  run_test<element2>(insert_test<element2>);
  run_test<element1>(insert_test<element1>);
}
CATCH_TEST_CASE("llist Clear Test", "[ds][llist]") {
  run_test<element8>(clear_test<element8>);
  run_test<element4>(clear_test<element4>);
  run_test<element2>(clear_test<element2>);
  run_test<element1>(clear_test<element1>);
}
CATCH_TEST_CASE("llist Delete Test", "[ds][llist]") {
  run_test<element8>(delete_test<element8>);
  run_test<element4>(delete_test<element4>);
  run_test<element2>(delete_test<element2>);
  run_test<element1>(delete_test<element1>);
}
CATCH_TEST_CASE("llist Copy Test", "[ds][llist]") {
  run_test<element8>(copy_test<element8>);
  run_test<element4>(copy_test<element4>);
  run_test<element2>(copy_test<element2>);
  run_test<element1>(copy_test<element1>);
}
CATCH_TEST_CASE("llist Copy2 Test", "[ds][llist]") {
  run_test<element8>(copy2_test<element8>);
  run_test<element4>(copy2_test<element4>);
  run_test<element2>(copy2_test<element2>);
  run_test<element1>(copy2_test<element1>);
}
CATCH_TEST_CASE("llist Filter Test", "[ds][llist]") {
  run_test<element8>(filter_test<element8>);
  run_test<element4>(filter_test<element4>);
  run_test<element2>(filter_test<element2>);
  run_test<element1>(filter_test<element1>);
}
CATCH_TEST_CASE("llist Filter2 Test", "[ds][llist]") {
  run_test<element8>(filter2_test<element8>);
  run_test<element4>(filter2_test<element4>);
  run_test<element2>(filter2_test<element2>);
  run_test<element1>(filter2_test<element1>);
}
CATCH_TEST_CASE("llist Splice Test", "[ds][llist]") {
  run_test2<element8>(splice_test<element8>);
  run_test2<element4>(splice_test<element4>);
  run_test2<element2>(splice_test<element2>);
  run_test2<element1>(splice_test<element1>);
}
CATCH_TEST_CASE("llist Sort Test", "[ds][llist]") {
  run_test<element8>(sort_test<element8>);
  run_test<element4>(sort_test<element4>);
  run_test<element2>(sort_test<element2>);
  run_test<element1>(sort_test<element1>);
}
CATCH_TEST_CASE("llist Pool Test", "[ds][llist]") {
  run_test<element8>(pool_test<element8>);
  run_test<element4>(pool_test<element4>);
  run_test<element2>(pool_test<element2>);
  run_test<element1>(pool_test<element1>);
}
CATCH_TEST_CASE("llist Inject Test", "[ds][llist]") {
  run_test<element8>(inject_test<element8>);
  run_test<element4>(inject_test<element4>);
  run_test<element2>(inject_test<element2>);
  run_test<element1>(inject_test<element1>);
}
CATCH_TEST_CASE("llist Iter Test", "[ds][llist]") {
  run_test<element8>(iter_test<element8>);
  run_test<element4>(iter_test<element4>);
  run_test<element2>(iter_test<element2>);
  run_test<element1>(iter_test<element1>);
}
