/**
 * @file llist-test.cpp
 *
 * Test of linked list module.
 *
 * @copyright 2017 John Harwell, All rights reserved.
 *
 * This file is part of RCSW.
 *
 * RCSW is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * RCSW is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * RCSW.  If not, see <http://www.gnu.org/licenses/
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <stdlib.h>

extern "C" {
#include "rcsw/ds/llist.h"
#include "rcsw/common/dbg.h"
#include "tests/ds_test.h"
}

#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_PREFIX_ALL
#include <catch.hpp>

/******************************************************************************
 * Constant Definitions
 *****************************************************************************/
#define NUM_TESTS       13

/*******************************************************************************
 * Test Function Forward Declarations
 ******************************************************************************/
/**
 * @brief Test appending/prepending items into a linked list
 */
static void insert_test(int len, struct ds_params * params);

/**
 * @brief Test clearing lists of different sizes
 */
static void clear_test(int len, struct ds_params * params);

/**
 * @brief Test deleting lists of different sizes
 */
static void delete_test(int len, struct ds_params * params);

/**
 * @brief Test of \ref llist_data_query()
 */
static void contains_test(int len, struct ds_params * params);

/**
 * @brief Test of \ref llist_copy()
 */
static void copy_test(int len, struct ds_params * params);

/**
 * @brief Test of \ref llist_copy2()
 *
 */
static void copy2_test(int len, struct ds_params * params);

/**
 * @brief Test of \ref llist_filter()
 */
static void filter_test(int len, struct ds_params * params);

/**
 * @brief Test of \ref llist_filter2()
 */
static void filter2_test(int len, struct ds_params * params);

/**
 * @brief Test of \ref llist_splice()
 */

static void sort_test(int len, struct ds_params * params);

/**
 * @brief Test of \ref llist_sort()
 */
static void splice_test(int len1, int len2, struct ds_params * params);

/**
 * @brief Test sharing llist_nodes between linked lists
 */
static void pool_test(int len, struct ds_params * params);

/**
 * @brief Test of \ref llist_inject()
 */
static void inject_test(int len, struct ds_params * params);

/**
 * @brief Test of linked list iteration
 */
static void iter_test(int len, struct ds_params * params);


/*******************************************************************************
 * Test Helper Functions
 ******************************************************************************/
static void test_runner(void (*test)(int len, struct ds_params *params)) {
  dbg_init();
  dbg_insmod(M_TESTING, "Testing");
  dbg_insmod(M_DS_LLIST, "LLIST");

  struct ds_params params;
  params.tag = DS_LLIST;
  params.flags = 0;
  params.cmpe = th_key_cmp;
  params.printe = th_printe;
  params.el_size = sizeof(struct element);
  CATCH_REQUIRE(th_ds_init(&params) == OK);

  for (int j = 3; j <= NUM_ITEMS; ++j) {
    for (int i = 0; i <= 0x100; ++i) {
      params.flags = i;
      params.max_elts = j;
      test(j, &params);
    } /* for(i..) */
  } /* for(j..) */
  th_ds_shutdown(&params);
} /* test_runner() */

static void test_runner2(void (*test)(int len1, int len2, struct ds_params *params)) {
  /* dbg_init(); */
  /* dbg_insmod(M_TESTING,"Testing"); */
  /* dbg_insmod(M_DS_LLIST,"LLIST"); */

  struct ds_params params;
  params.tag = DS_LLIST;
  params.flags = 0;
  params.cmpe = th_key_cmp;
  params.printe = th_printe;
  params.el_size = sizeof(struct element);
  CATCH_REQUIRE(th_ds_init(&params) == OK);

  for (int k = 2; k <= NUM_ITEMS; ++k) {
    for (int j = 1; j <= NUM_ITEMS; ++j) {
      for (int i = 0; i <= 0x100; ++i) {
        params.flags = i;
        params.max_elts = j+k;
        test(k, j, &params);
      } /* for(i..) */
    } /* for(j..) */
  } /* for(k..) */
  th_ds_shutdown(&params);
} /* test_runner() */

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("llist Insert Test", "[llist]") { test_runner(insert_test); }
CATCH_TEST_CASE("llist Clear Test", "[llist]") { test_runner(clear_test); }
CATCH_TEST_CASE("llist Delete Test", "[llist]") { test_runner(delete_test); }
CATCH_TEST_CASE("llist Contains Test", "[llist]") { test_runner(contains_test); }
CATCH_TEST_CASE("llist Copy Test", "[llist]") { test_runner(copy_test); }
CATCH_TEST_CASE("llist Copy2 Test", "[llist]") { test_runner(copy2_test); }
CATCH_TEST_CASE("llist Filter Test", "[llist]") { test_runner(filter_test); }
CATCH_TEST_CASE("llist Filter2 Test", "[llist]") { test_runner(filter2_test); }
CATCH_TEST_CASE("llist Splice Test", "[llist]") { test_runner2(splice_test); }
CATCH_TEST_CASE("llist Sort Test", "[llist]") { test_runner(sort_test); }
CATCH_TEST_CASE("llist Pool Test", "[llist]") { test_runner(pool_test); }
CATCH_TEST_CASE("llist Inject Test", "[llist]") { test_runner(inject_test); }
CATCH_TEST_CASE("llist Iter Test", "[llist]") { test_runner(iter_test); }

/*******************************************************************************
 * Test Functions
 ******************************************************************************/
static void insert_test(int len, struct ds_params * params) {
  struct llist* list;
  struct llist mylist;

  int i;
  int arr[len];

  params->flags &= ~(DS_LLIST_PTR_CMP | DS_LLIST_NO_DB);
  list = llist_init(&mylist, params);
  CATCH_REQUIRE(NULL != list);

  for (i = 0; i < len; i++) {
    struct element e;
    int value = (rand() % 2)? (rand()% len) % (len + 1): i;
    e.value1 = value;

    if (rand() % 2) {
      CATCH_REQUIRE(llist_append(list, &e) == OK);
    } else {
      CATCH_REQUIRE(llist_prepend(list, &e) == OK);
    }
    arr[i] = value;
  }

  CATCH_REQUIRE(list->current == len);

  for (i = 0; i < len; i++) {
    struct element e = {.value1 = arr[i], .value2 = 17};
    CATCH_REQUIRE(llist_node_query(list, &e));
  }

  llist_destroy(list);

  /* verify all DS_APP_DOMAIN data deallocated */
  CATCH_REQUIRE(th_leak_check_data(params) == 0);
  CATCH_REQUIRE(th_leak_check_nodes(params) == 0);
} /* insert_test () */

static void clear_test(int len, struct ds_params * params) {
  struct llist* list;
  struct llist mylist;

  int i;
  params->flags &= ~(DS_LLIST_PTR_CMP | DS_LLIST_NO_DB);
  list = llist_init(&mylist, params);
  CATCH_REQUIRE(NULL != list);

  for (i = 1; i <= len; i++) {

    struct element e;
    int value = (rand() % len) % (len + 1);
    e.value1 = value;

    CATCH_REQUIRE(llist_append(list, &e) == OK);
  } /* for() */

  llist_clear(list);
  CATCH_REQUIRE((llist_isempty(list) && (list->first == NULL)));


  llist_destroy(list);

  /* verify all DS_APP_DOMAIN data deallocated */
  CATCH_REQUIRE(th_leak_check_data(params) == 0);
  CATCH_REQUIRE(th_leak_check_nodes(params) == 0);
} /* clear_test() */

static void delete_test(int len, struct ds_params * params) {
  struct llist* list;
  struct llist mylist;
  int i;

  params->flags &= ~(DS_LLIST_PTR_CMP | DS_LLIST_NO_DB);
  list = llist_init(&mylist, params);
  CATCH_REQUIRE(NULL != list);

  for (i = 1; i <= len; i++) {
    struct element e;
    int value = (rand() % len) % (len + 1);
    e.value1 = value;

    CATCH_REQUIRE(llist_append(list, &e) == OK);
  } /* for() */

  llist_destroy(list);

  /* verify all DS_APP_DOMAIN data deallocated */
  CATCH_REQUIRE(th_leak_check_data(params) == 0);
  CATCH_REQUIRE(th_leak_check_nodes(params) == 0);
} /* delete_test() */

static void contains_test(int len, struct ds_params * params) {
  struct llist* list;
  struct llist mylist;
  int i, j;
  int arr[len];

  params->flags &= ~(DS_LLIST_PTR_CMP | DS_LLIST_NO_DB);
  list = llist_init(&mylist, params);
  CATCH_REQUIRE(NULL != list);

  for (i = 0; i < len; i++) {
    struct element e;
    int value = (rand() % len) % (len + 1);
    e.value1 = value;

    CATCH_REQUIRE(llist_append(list, &e) == OK);
    arr[i] = value;
  } /* for() */

  for (i = 0; i < len; i++) {
    int value = rand() % (i+ 1);
    for (j = 0; j < len; j++) {
      if (value == arr[j]) {
        struct element e = {.value1 = value, .value2 = 17};
        CATCH_REQUIRE(llist_data_query(list, &e) != NULL);
        break;
      }
    } /* end for() */
  } /* end for() */

  llist_destroy(list);

  /* verify all DS_APP_DOMAIN data deallocated */
  CATCH_REQUIRE(th_leak_check_data(params) == 0);
  CATCH_REQUIRE(th_leak_check_nodes(params) == 0);
} /* contains_test() */

static void copy_test(int len, struct ds_params * params) {
  struct llist* list1, *list2;
  struct llist mylist;
  int i;
  int arr[len];

  params->flags &= ~(DS_LLIST_PTR_CMP | DS_LLIST_NO_DB);
  list1 = llist_init(&mylist, params);
  CATCH_REQUIRE(NULL != list1);

  for (i = 0; i < len; i++) {
    struct element e;
    int value = rand() % (i+1) + 3;
    e.value1 = value;
    CATCH_REQUIRE(llist_append(list1, &e) == OK);
    arr[i] = value;
  } /* for() */

  if (!(params->flags & (DS_APP_DOMAIN_HANDLE | DS_APP_DOMAIN_DATA | DS_APP_DOMAIN_NODES))) {
    list2 = llist_copy(list1, params);
  } else {
    list2 = llist_copy(list1, NULL);
  }
  CATCH_REQUIRE(NULL != list2);


  for (i = 0; i < len; i++) {
    struct element e = {.value1 = arr[i], .value2 = 17};
    CATCH_REQUIRE(llist_node_query(list1, &e) != NULL);
    CATCH_REQUIRE(llist_node_query(list2, &e) != NULL);
  } /* for() */

  llist_destroy(list1);
  llist_destroy(list2);

  /* verify all DS_APP_DOMAIN data deallocated */
  CATCH_REQUIRE(th_leak_check_data(params) == 0);
  CATCH_REQUIRE(th_leak_check_nodes(params) == 0);
} /* copy_test() */

static void copy2_test(int len, struct ds_params * params) {
  struct llist* list1, *list2;
  struct llist mylist;
  int i;
  int arr[len];

  params->flags &= ~(DS_LLIST_PTR_CMP | DS_LLIST_NO_DB);
  if (params->flags & DS_APP_DOMAIN_HANDLE) {
    list1 = llist_init(&mylist, params);
  } else {
    list1 = llist_init(NULL, params);
  }

  CATCH_REQUIRE(NULL != list1);

  for (i = 0; i < len; i++) {
    struct element e;
    int value = rand() % (i+1);
    e.value1 = value;
    CATCH_REQUIRE(llist_append(list1, &e) == OK);
    arr[i] = value;
  } /* for() */

  if (!(params->flags & (DS_APP_DOMAIN_HANDLE |
                         DS_APP_DOMAIN_DATA | DS_APP_DOMAIN_NODES))) {
    list2 = llist_copy2(list1, th_filter_func, params);
  } else {
    list2 = llist_copy2(list1, th_filter_func, NULL);
  }

  CATCH_REQUIRE(NULL != list2);

  for (i = 0; i < len; i++) {
    struct element e = {.value1 = arr[i], .value2 = 17};
    if (th_filter_func(&e)) {
      CATCH_REQUIRE(NULL != llist_node_query(list2, &e));
    }
    CATCH_REQUIRE(NULL != llist_node_query(list1, &e));
  } /* for() */

  llist_destroy(list1);
  llist_destroy(list2);

  /* verify all DS_APP_DOMAIN data deallocated */
  CATCH_REQUIRE(th_leak_check_data(params) == 0);
  CATCH_REQUIRE(th_leak_check_nodes(params) == 0);
} /* copy2_test() */

static void filter_test(int len, struct ds_params * params) {
  struct llist* list1, *list2;
  struct llist mylist;

  int i;
  int arr[len];

  params->flags &= ~(DS_LLIST_PTR_CMP | DS_LLIST_NO_DB);
  if (params->flags & DS_APP_DOMAIN_HANDLE) {
    list1 = llist_init(&mylist, params);
  } else {
    list1 = llist_init(NULL, params);
  }

  CATCH_REQUIRE(NULL != list1);

  for (i = 0; i < len; i++) {
    struct element e;
    int value = rand() % (i+1) + 2;
    e.value1 = value;
    CATCH_REQUIRE(llist_append(list1, &e) == OK);
    arr[i] = value;
  } /* for() */

  if (!(params->flags & (DS_APP_DOMAIN_HANDLE | DS_APP_DOMAIN_DATA |
                         DS_APP_DOMAIN_NODES))) {
    list2 = llist_filter(list1, th_filter_func, params);
  } else {
    list2 = llist_filter(list1, th_filter_func, NULL);
  }

  CATCH_REQUIRE(NULL != list2);

  for (i = 0; i < len; i++) {
    struct element e = {.value1 = arr[i], .value2 = 17};
    if (th_filter_func(&e)) {
      CATCH_REQUIRE(llist_node_query(list2, &e) != NULL);
      CATCH_REQUIRE(llist_node_query(list1, &e) == NULL);
    }
  } /* for() */

  llist_destroy(list1);
  llist_destroy(list2);

  /* verify all DS_APP_DOMAIN data deallocated */
  CATCH_REQUIRE(th_leak_check_data(params) == 0);
  CATCH_REQUIRE(th_leak_check_nodes(params) == 0);
} /* filter_test() */

static void filter2_test(int len, struct ds_params * params) {
  struct llist* list1;
  struct llist mylist;
  int i;
  int arr[len];

  params->flags &= ~(DS_LLIST_PTR_CMP | DS_LLIST_NO_DB);
  if (params->flags & DS_APP_DOMAIN_HANDLE) {
    list1 = llist_init(&mylist, params);
  } else {
    list1 = llist_init(NULL, params);
  }

  CATCH_REQUIRE(NULL != list1);

  for (i = 0; i < len; i++) {
    struct element e;
    int value = rand() % (i+1);
    e.value1 = value;
    CATCH_REQUIRE(llist_append(list1, &e) == OK);
    arr[i] = value;
  } /* for() */

  CATCH_REQUIRE(llist_filter2(list1, th_filter_func) == OK);

  for (i = 0; i < len; i++) {
    struct element e = {.value1 = arr[i], .value2 = 17};
    if (th_filter_func(&e)) {
      CATCH_REQUIRE(llist_node_query(list1, &e) == NULL);
    }
  } /* for() */

  llist_destroy(list1);

  /* verify all DS_APP_DOMAIN data deallocated */
  CATCH_REQUIRE(th_leak_check_data(params) == 0);
  CATCH_REQUIRE(th_leak_check_nodes(params) == 0);} /* filter2_test() */

static void splice_test(int len1, int len2, struct ds_params * params) {
  struct llist *list1 = NULL;
  struct llist *list2 = NULL;
  struct llist mylist1;
  struct llist mylist2;

  int i;
  int arr1[len1];
  int arr2[len2];
  struct llist_node* splice_node = NULL;
  int splice = (rand() + len1/2) % len1;

  params->flags &= ~(DS_LLIST_PTR_CMP | DS_LLIST_NO_DB);
  if (params->flags & DS_APP_DOMAIN_HANDLE) {
    list1 = llist_init(&mylist1, params);
  } else {
    list1 = llist_init(NULL, params);
  }

  if (params->flags & DS_APP_DOMAIN_HANDLE) {
    list2 = llist_init(&mylist2, params);
  } else {
    list2 = llist_init(NULL, params);
  }

  CATCH_REQUIRE(NULL != list1);
  CATCH_REQUIRE(NULL != list2);

  /* fill list1 */
  for (i = 0; i < len1; i++) {
    struct element e;
    int value = i+1;
    e.value1 = value;
    CATCH_REQUIRE(llist_append(list1, &e) == OK);
    if (splice == i) {
      splice_node = llist_node_query(list1, &e);
    }
    arr1[i] = value;
  } /* for() */

  /* fill list2 */
  for(i=0;i<len2;i++){
    struct element e;
    int value = i+3;
    e.value1 = value;
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
          CATCH_REQUIRE(((struct element*)curr->data)->value1 == arr2[count]);
        } else {
          CATCH_REQUIRE(((struct element*)curr->data)->value1 ==
                        arr1[count - len2]);
        }
        count++;
      }
    } else { /* append */
      CATCH_REQUIRE(llist_splice(list1, list2, list1->last) == OK);
      int count = 0;
      LLIST_FOREACH(list1, next, curr) {
        if (count < len1) {
          CATCH_REQUIRE(((struct element*)curr->data)->value1 == arr1[count]);
        } else {
          CATCH_REQUIRE(((struct element*)curr->data)->value1 ==
                        arr2[count - len1]);
        }
        count++;
      }
    }
  } else { /* general case */
    CATCH_REQUIRE(llist_splice(list1, list2, splice_node) == OK);
    int count = 0;
    LLIST_FOREACH(list1, next, curr) {
      if (count < splice) {
        CATCH_REQUIRE(((struct element*)curr->data)->value1 == arr1[count]);
      } else if (count < splice + len2) {
        CATCH_REQUIRE(((struct element*)curr->data)->value1 ==
                      arr2[count -splice]);
      } else {
        CATCH_REQUIRE(((struct element*)curr->data)->value1 ==
                      arr1[count - len2]);
      }
      count++;
    }
  }

  llist_destroy(list1);

  /* verify all DS_APP_DOMAIN data deallocated */
  CATCH_REQUIRE(th_leak_check_data(params) == 0);
  CATCH_REQUIRE(th_leak_check_nodes(params) == 0);
} /* splice_test() */

static void sort_test(int len, struct ds_params * params) {
  struct llist* list1;
  struct llist  mylist;

  int i;
  params->flags &= ~(DS_LLIST_PTR_CMP | DS_LLIST_NO_DB);
  if (params->flags & DS_APP_DOMAIN_HANDLE) {
    list1 = llist_init(&mylist, params);
  } else {
    list1 = llist_init(NULL, params);
  }

  CATCH_REQUIRE(NULL != list1);

  for (i = 0; i < len; i++) {
    struct element e;
    int value = rand() % (i+1) + i;
    e.value1 = value;
    CATCH_REQUIRE(llist_append(list1, &e) == OK);
  } /* for() */

  llist_sort(list1, (enum alg_sort_type)(rand() % 2 + 2));

  struct element *e;
  int val = -1;

  /* verify list is sorted */
  LLIST_FOREACH(list1, next, curr) {
    e = (struct element*)curr->data;
    CATCH_REQUIRE(val <= e->value1);
    val = e->value1;
  }

  llist_destroy(list1);

  /* verify all DS_APP_DOMAIN data deallocated */
  CATCH_REQUIRE(th_leak_check_data(params) == 0);
  CATCH_REQUIRE(th_leak_check_nodes(params) == 0);
} /* sort_test() */

static void pool_test(int len, struct ds_params * params) {
  struct llist* list1, *list2;
  int i;

  if (!(params->flags & DS_APP_DOMAIN_DATA)) {
    return;
  }

  params->flags |= DS_LLIST_NO_DB | DS_LLIST_PTR_CMP;
  params->flags &= ~DS_APP_DOMAIN_HANDLE;

  list1 = llist_init(NULL, params);
  list2 = llist_init(NULL, params);

  CATCH_REQUIRE(NULL != list1);
  CATCH_REQUIRE(NULL != list2);

  /* fill list1 by carving up the space provided for data */
  for (i = 0; i < len; i++) {
    CATCH_REQUIRE(llist_append(list1,
                               params->elements +
                               i * sizeof(struct element)) == OK);
  } /* for() */

  /* Remove each element from list1, add it to list2 */
  for (i = 0; i < len; ++i) {
    CATCH_REQUIRE(llist_remove(list1, params->elements +
                               i* sizeof(struct element)) == OK);
    struct llist_node* node = llist_node_query(list1,
                                               params->elements +
                                               i * sizeof(struct element));
    CATCH_REQUIRE(node == NULL);
    CATCH_REQUIRE(llist_append(list2,
                               params->elements +
                               i * sizeof(struct element)) == OK);
    node = llist_node_query(list2,
                            params->elements + i * sizeof(struct element));
    CATCH_REQUIRE(NULL != node);
  } /* for() */

  CATCH_REQUIRE(llist_isempty(list1));

  /* Remove each element from list2, add it back to list1 */
  for (i = 0; i < len; ++i) {
    llist_remove(list2, params->elements + i* sizeof(struct element));
    struct llist_node *node = llist_node_query(list2,
                                               params->elements +
                                               i * sizeof(struct element));
    CATCH_REQUIRE(node == NULL);
    CATCH_REQUIRE(llist_append(list1,
                               params->elements +
                               i * sizeof(struct element)) == OK);
    node = llist_node_query(list1, params->elements +
                            i * sizeof(struct element));
    CATCH_REQUIRE(NULL != node);
  } /* for() */

  CATCH_REQUIRE(llist_isempty(list2));

  llist_destroy(list1);
  llist_destroy(list2);

  params->flags &= ~DS_LLIST_NO_DB;

  /* verify all DS_APP_DOMAIN data deallocated */
  CATCH_REQUIRE(th_leak_check_data(params) == 0);
  CATCH_REQUIRE(th_leak_check_nodes(params) == 0);
} /* pool_test() */

static void inject_test(int len, struct ds_params * params) {
  struct llist *list;
  struct llist mylist;
  int i;
  int sum = 0;

  params->flags &= ~(DS_LLIST_PTR_CMP | DS_LLIST_NO_DB);
  if (params->flags & DS_APP_DOMAIN_HANDLE) {
    list = llist_init(&mylist, params);
  } else {
    list = llist_init(NULL, params);
  }
  CATCH_REQUIRE(NULL != list);

  for (i = 0; i < len; i++) {
    struct element e = {.value1 = i, .value2 = 17};
    sum +=i;
    CATCH_REQUIRE(llist_append(list, &e) == OK);
  } /* for() */

  int total = 0;
  CATCH_REQUIRE(llist_inject(list, th_inject_func, &total) == OK);
  CATCH_REQUIRE(total == sum);

  llist_destroy(list);
} /* inject_test() */

static void iter_test(int len, struct ds_params * params) {
  struct llist *list;
  struct llist mylist;
  int i;

  params->flags &= ~(DS_LLIST_PTR_CMP | DS_LLIST_NO_DB);
  if (params->flags & DS_APP_DOMAIN_HANDLE) {
    list = llist_init(&mylist, params);
  } else {
    list = llist_init(NULL, params);
  }
  CATCH_REQUIRE(NULL != list);

  for (i = 0; i < len; i++) {
    struct element e = {.value1 = i, .value2 = 17};
    CATCH_REQUIRE(llist_append(list, &e) == OK);
  } /* for() */

  struct element * e;

  struct ds_iterator * iter = ds_iter_init(DS_LLIST, list, th_iter_func);
  CATCH_REQUIRE(NULL != iter);
  while ((e = (element*)ds_iter_next(iter)) != NULL) {
    CATCH_REQUIRE(e->value1 % 2 == 0);
  }

  iter = ds_iter_init(DS_LLIST, list, NULL);
  CATCH_REQUIRE(NULL != iter);
  int count = 0;
  while ((e = (element*)ds_iter_next(iter)) != NULL) {
    CATCH_REQUIRE((int)e->value1 == count);
    count++;
  }
  CATCH_REQUIRE(count == list->current);

  llist_destroy(list);
} /* iter_test() */
