/**
 * \file ds-darray-utest.cpp
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_PREFIX_ALL
#include <catch.hpp>

#include "rcsw/ds/darray.h"
#include "rcsw/algorithm/sort.h"
#include "tests/ds_test.h"
#include "tests/ds_test.hpp"

/*******************************************************************************
 * Test Helper Functions
 ******************************************************************************/
template<typename T>
static void run_test(void (*test)(int len, struct darray_params *params)) {
  /* dbg_init(); */
  /* dbg_insmod(M_TESTING,"Testing"); */
  /* dbg_insmod(M_DS_DARRAY,"DARRAY"); */

  struct darray_params params;
  params.flags = 0;
  params.cmpe = th_cmpe<T>;
  params.printe = th_printe<T>;
  params.elt_size = sizeof(T);
  params.init_size = 0;

  CATCH_REQUIRE(th_ds_init(&params) == OK);

  uint32_t flags[] = {
    RCSW_DS_SORTED,
    RCSW_DS_ORDERED,
    RCSW_NOALLOC_HANDLE,
    RCSW_NOALLOC_DATA,
  };
  /* test with defined sizes */
  for (int j = 1; j <= TH_NUM_ITEMS; ++j) {
    for (size_t i = 0; i < RCSW_ARRAY_ELTS(flags); ++i) {
      params.flags = flags[i];
      test(j, &params);
    } /* for(i...) */
  } /* for(j...) */
  th_ds_shutdown(&params);
} /* run_test() */

/*******************************************************************************
 * Test Functions
 ******************************************************************************/
template <typename T>
static void addremove_test(int len, struct darray_params *params) {
  struct darray *_arr;
  struct darray my_arr;

  if (params->flags & RCSW_NOALLOC_HANDLE) {
    CATCH_REQUIRE(nullptr == darray_init(nullptr, params));
    _arr = darray_init(&my_arr, params);
  } else {
    _arr = darray_init(nullptr, params);
  }
  CATCH_REQUIRE(nullptr != _arr);

  T arr[TH_NUM_ITEMS];

  element_generator<T> g(gen_elt_type::ekINC_VALS, params->max_elts);

  for (int i = 0; i < len; i++) {
    arr[i] = g.next();
    /* arr[i].value1 += 1; */

    if (rand() % 2) {  /* prepend */
      CATCH_REQUIRE(darray_insert(_arr, &arr[i], 0) == OK);
    } else { /* append */
      CATCH_REQUIRE(darray_insert(_arr, &arr[i], _arr->current) == OK);
    }

    for (int j = 0; j <= i; ++j) {
      CATCH_REQUIRE(darray_idx_query(_arr, &arr[j]) != -1);
    }
  } /* for() */

  CATCH_REQUIRE(darray_n_elts(_arr) == (size_t)len);

  if (!(_arr->flags & RCSW_NOALLOC_DATA)) {
    CATCH_REQUIRE(OK == darray_resize(_arr, darray_n_elts(_arr) * 2));
  }

  darray_print(_arr);
  for (int i = 0; i < len; i++) {
    T e;
    CATCH_REQUIRE(OK == darray_remove(_arr, &e, 0));
    CATCH_REQUIRE(darray_idx_query(_arr, &e) == -1);
  }
  CATCH_REQUIRE(darray_isempty(_arr));
  darray_destroy(_arr);
} /* addremove_test () */

template <typename T>
static void delete_test(int len, struct darray_params *params) {
  struct darray* _arr;
  struct darray my_arr;

  if (params->flags & RCSW_NOALLOC_HANDLE) {
    _arr = darray_init(&my_arr, params);
  } else {
    _arr = darray_init(nullptr, params);
  }

  CATCH_REQUIRE(nullptr != _arr);

  element_generator<T> g(gen_elt_type::ekRAND_VALS, params->max_elts);

  for (int i = 1; i <= len; i++) {
    T e = g.next();
    CATCH_REQUIRE(darray_insert(_arr, &e, _arr->current) == OK);
  }

  darray_clear(_arr);
  int sum = 0;
  for (int i = 0; i < len; ++i) {
    sum += _arr->elements[i];
  } /* for(i..) */

  CATCH_REQUIRE(sum == 0);

  g.reset();
  for (int i = 0; i < len; i++) {
    T e = g.next();
    CATCH_REQUIRE(darray_insert(_arr, &e, _arr->current) == OK);
  }

  if (len / 2 > 0) {
    if (_arr->flags & RCSW_NOALLOC_DATA) {
      CATCH_REQUIRE(ERROR == darray_resize(_arr, darray_n_elts(_arr)/ 2));
    } else {
      CATCH_REQUIRE(OK == darray_resize(_arr, darray_n_elts(_arr)/ 2));
      CATCH_REQUIRE(darray_n_elts(_arr) == (size_t)(len / 2 - 1));
    }
  } else {
    CATCH_REQUIRE(OK == darray_resize(_arr, darray_n_elts(_arr)/ 2));
    CATCH_REQUIRE(darray_n_elts(_arr) == 0);
  }
  darray_destroy(_arr);
} /* delete_test() */

template <typename T>
static void contains_test(int len, struct darray_params *params) {
  struct darray* _arr;
  struct darray my_arr;

  T arr[TH_NUM_ITEMS];

  if (params->flags & RCSW_NOALLOC_HANDLE) {
    _arr = darray_init(&my_arr, params);
  } else {
    _arr = darray_init(nullptr, params);
  }

  CATCH_REQUIRE(nullptr != _arr);

  element_generator<T> g(gen_elt_type::ekRAND_VALS, params->max_elts);

  for (int i = 0; i < len; i++) {
    arr[i] = g.next();
    CATCH_REQUIRE(darray_insert(_arr, &arr[i], _arr->current) == OK);

    for (int j = 0; j <= i; ++j) {
      CATCH_REQUIRE(darray_idx_query(_arr, &arr[j]) != -1);
    } /* end for() */
  }

  darray_destroy(_arr);
} /* contains_test() */

template <typename T>
static void filter_test(int len, struct darray_params *params) {
  struct darray* _arr1, *_arr2;
  struct darray my_arr;

  T arr[TH_NUM_ITEMS];

  if (params->flags & RCSW_NOALLOC_HANDLE) {
    _arr1 = darray_init(&my_arr, params);
  } else {
    _arr1 = darray_init(nullptr, params);
  }

  CATCH_REQUIRE(nullptr != _arr1);

  element_generator<T> g(gen_elt_type::ekINC_VALS, params->max_elts);

  for (int i = 0; i < len; i++) {
    arr[i] = g.next();
    CATCH_REQUIRE(darray_insert(_arr1, &arr[i], _arr1->current) == OK);
  }

  if ((params->flags & (RCSW_NOALLOC_DATA | RCSW_NOALLOC_HANDLE))) {
    _arr2 = darray_filter(_arr1, th_filter_func<T>, 0, nullptr);
    CATCH_REQUIRE(nullptr != _arr2);
    for (int i = 0; i < len; i++) {
      if (th_filter_func<T>(&arr[i])) {
        CATCH_REQUIRE(darray_idx_query(_arr2, &arr[i]) != -1);
        CATCH_REQUIRE(darray_idx_query(_arr1, &arr[i]) == -1);
      } else {
        CATCH_REQUIRE(darray_idx_query(_arr2, &arr[i]) == -1);
        CATCH_REQUIRE(darray_idx_query(_arr1, &arr[i]) != -1);
      }
    }
    darray_destroy(_arr2);
  }
  darray_destroy(_arr1);
} /* filter_test() */

template <typename T>
static void copy_test(int len, struct darray_params *params) {
  struct darray* _arr1, *_arr2;
  struct darray my_arr;

  T arr[TH_NUM_ITEMS];

  if (params->flags & RCSW_NOALLOC_HANDLE) {
    _arr1 = darray_init(&my_arr, params);
  } else {
    _arr1 = darray_init(nullptr, params);
  }
  CATCH_REQUIRE(nullptr != _arr1);

  element_generator<T> g(gen_elt_type::ekRAND_VALS, params->max_elts);

  for (int i = 0; i < len; i++) {
    arr[i] = g.next();
    CATCH_REQUIRE(darray_insert(_arr1, &arr[i], _arr1->current) == OK);
  }


  if (!(params->flags & (RCSW_NOALLOC_DATA | RCSW_NOALLOC_HANDLE))) {
    _arr2 = darray_copy(_arr1, params->flags, nullptr);
  } else {
    _arr2 = darray_copy(_arr1, 0x0, nullptr);
  }

  CATCH_REQUIRE(nullptr != _arr2);

  for (int i = 0; i < len; i++) {
    CATCH_REQUIRE(darray_idx_query(_arr1, &arr[i]) != -1);
    CATCH_REQUIRE(darray_idx_query(_arr2, &arr[i]) != -1);
  }

  darray_destroy(_arr1);
  darray_destroy(_arr2);
} /* copy_test() */

template <typename T>
static void sort_test(int len, struct darray_params *params) {
  struct darray* _arr1;
  struct darray my_arr;

  if (params->flags & RCSW_NOALLOC_HANDLE) {
    _arr1 = darray_init(&my_arr, params);
  } else {
    _arr1 = darray_init(nullptr, params);
  }

  CATCH_REQUIRE((nullptr != _arr1));

  element_generator<T> g(gen_elt_type::ekRAND_VALS, params->max_elts);

  for (int i = 0; i < len; i++) {
    T e = g.next();
    CATCH_REQUIRE(darray_insert(_arr1, &e, _arr1->current) == OK);
  }

  if (rand() %2) {
    darray_sort(_arr1, ekQSORT_ITER);
  } else {
    darray_sort(_arr1, ekQSORT_REC);
  }

  /* validate sorting */
  for (int i = 0; i < len-1; i++) {
    CATCH_REQUIRE(((T*)darray_data_get(_arr1, i))->value1 <=
            ((T*)darray_data_get(_arr1, i+1))->value1);
  } /* for() */

  darray_destroy(_arr1);
} /* sort_test() */

template <typename T>
static void binarysearch_test(int len, struct darray_params *params) {
  struct darray* _arr1;
  struct darray my_arr;

  T arr[TH_NUM_ITEMS];

  if (params->flags & RCSW_NOALLOC_HANDLE) {
    _arr1 = darray_init(&my_arr, params);
  } else {
    _arr1 = darray_init(nullptr, params);
  }

  CATCH_REQUIRE(nullptr != _arr1);

  element_generator<T> g(gen_elt_type::ekRAND_VALS, params->max_elts);

  for (int i = 0; i < len; i++) {
    arr[i] = g.next();
    CATCH_REQUIRE(darray_insert(_arr1, &arr[i], _arr1->current) == OK);
  } /* for() */

  darray_sort(_arr1, ekQSORT_ITER);

  for (int i = 0; i < len; i++) {
    CATCH_REQUIRE(darray_idx_query(_arr1, &arr[i]) != -1);
  } /* for() */

  darray_destroy(_arr1);
} /* binarysearch_test() */

template <typename T>
static void inject_test(int len, struct darray_params * params) {
  struct darray *arr;
  struct darray myarr;

  if (params->flags & RCSW_NOALLOC_HANDLE) {
    arr = darray_init(&myarr, params);
  } else {
    arr = darray_init(nullptr, params);
  }
  CATCH_REQUIRE(nullptr != arr);

  element_generator<T> g(gen_elt_type::ekINC_VALS, params->max_elts);

  int sum = 0;
  for (int i = 0; i < len; i++) {
    T e = g.next();
    sum += i;
    CATCH_REQUIRE(darray_insert(arr, &e, arr->current) == OK);
  }

  int total = 0;
  CATCH_REQUIRE(darray_inject(arr, th_inject_func<T>, &total) == OK);
  CATCH_REQUIRE(total == sum);

  darray_destroy(arr);
} /* inject_test() */

template <typename T>
static void iter_test(int len, struct darray_params * params) {
  struct darray *arr;
  struct darray myarr;

  if (params->flags & RCSW_NOALLOC_HANDLE) {
    arr = darray_init(&myarr, params);
  } else {
    arr = darray_init(nullptr, params);
  }
  CATCH_REQUIRE(nullptr != arr);

  element_generator<T> g(gen_elt_type::ekINC_VALS, params->max_elts);

  for (int i = 0; i < len; i++) {
    T e = g.next();
    CATCH_REQUIRE(darray_insert(arr, &e, arr->current) == OK);
  }

  T * e;
  struct ds_iterator * iter = ds_filter_init(arr,
                                             ekRCSW_DS_DARRAY,
                                             th_iter_func<T>);
  CATCH_REQUIRE(nullptr != iter);

  while ((e = (T*)ds_iter_next(iter)) != nullptr) {
    CATCH_REQUIRE(e->value1 % 2 == 0);
  }

  iter = ds_iter_init(arr, ekRCSW_DS_DARRAY, ekRCSW_DS_ITER_FORWARD);
  CATCH_REQUIRE(nullptr != iter);

  size_t count = 0;
  while ((e = (T*)ds_iter_next(iter)) != nullptr) {
    CATCH_REQUIRE(e->value1 == (decltype(T::value1))count);
    count++;
  }
  CATCH_REQUIRE(count == darray_n_elts(arr));

  iter = ds_iter_init(arr, ekRCSW_DS_DARRAY, ekRCSW_DS_ITER_BACKWARD);
  CATCH_REQUIRE(nullptr != iter);

  count = 0;
  while ((e = (T*)ds_iter_next(iter)) != nullptr) {
    CATCH_REQUIRE(e->value1 == len - (decltype(T::value1))count - 1);
    count++;
  }
  CATCH_REQUIRE(count == darray_n_elts(arr));

  darray_destroy(arr);
} /* iter_test() */

template <typename T>
static void map_test(int len, struct darray_params * params) {
  struct darray *arr;
  struct darray myarr;

  if (params->flags & RCSW_NOALLOC_HANDLE) {
    arr = darray_init(&myarr, params);
  } else {
    arr = darray_init(nullptr, params);
  }
  CATCH_REQUIRE(nullptr != arr);

  element_generator<T> g(gen_elt_type::ekINC_VALS, params->max_elts);

  for (int i = 0; i < len; i++) {
    T e = g.next();
    CATCH_REQUIRE(darray_insert(arr, &e, arr->current) == OK);
  }

  CATCH_REQUIRE(darray_map(arr, th_map_func<T>) == OK);
  for (int i = 0; i < len; ++i) {
    T e;
    CATCH_REQUIRE(darray_idx_serve(arr, &e, i) == OK);
    CATCH_REQUIRE(e.value1 == i - 1);
  } /* for(i..) */

  darray_destroy(arr);
} /* map_test() */

template <typename T>
static void print_test(int len, struct darray_params *params) {
  struct darray* _arr;
  struct darray my_arr;

  T arr[TH_NUM_ITEMS];

  if (params->flags & RCSW_NOALLOC_HANDLE) {
    _arr = darray_init(&my_arr, params);
  } else {
    _arr = darray_init(nullptr, params);
  }

  CATCH_REQUIRE(nullptr != _arr);

  element_generator<T> g(gen_elt_type::ekRAND_VALS, params->max_elts);

  for (int i = 0; i < len; i++) {
    arr[i] = g.next();
    CATCH_REQUIRE(darray_insert(_arr, &arr[i], _arr->current) == OK);
  }

  darray_print(nullptr);
} /* print_test() */

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("Add/Remove Test", "[ds][darray]") {
  run_test<element8>(addremove_test<element8>);
  run_test<element4>(addremove_test<element4>);
  run_test<element2>(addremove_test<element2>);
  run_test<element1>(addremove_test<element1>);
}
CATCH_TEST_CASE("Delete Test", "[ds][darray]") {
  run_test<element8>(delete_test<element8>);
  run_test<element4>(delete_test<element4>);
  run_test<element2>(delete_test<element2>);
  run_test<element1>(delete_test<element1>);
}
CATCH_TEST_CASE("Contains Test", "[ds][darray]") {
  run_test<element8>(contains_test<element8>);
  run_test<element4>(contains_test<element4>);
  run_test<element2>(contains_test<element2>);
  run_test<element1>(contains_test<element1>);
}
CATCH_TEST_CASE("Filter Test", "[ds][darray]") {
  run_test<element8>(filter_test<element8>);
  run_test<element4>(filter_test<element4>);
  run_test<element2>(filter_test<element2>);
  run_test<element1>(filter_test<element1>);
}
CATCH_TEST_CASE("Copy Test", "[ds][darray]") {
  run_test<element8>(copy_test<element8>);
  run_test<element4>(copy_test<element4>);
  run_test<element2>(copy_test<element2>);
  run_test<element1>(copy_test<element1>);
}
CATCH_TEST_CASE("Sort Test", "[ds][darray]") {
  run_test<element8>(sort_test<element8>);
  run_test<element4>(sort_test<element4>);
  run_test<element2>(sort_test<element2>);
  run_test<element1>(sort_test<element1>);
}
CATCH_TEST_CASE("Binary Search Test", "[ds][darray]") {
  run_test<element8>(binarysearch_test<element8>);
  run_test<element4>(binarysearch_test<element4>);
  run_test<element2>(binarysearch_test<element2>);
  run_test<element1>(binarysearch_test<element1>);
}
CATCH_TEST_CASE("Inject Test", "[ds][darray]") {
  run_test<element8>(inject_test<element8>);
  run_test<element4>(inject_test<element4>);
  run_test<element2>(inject_test<element2>);
  run_test<element1>(inject_test<element1>);
}
CATCH_TEST_CASE("Iter Test", "[ds][darray]") {
  run_test<element8>(iter_test<element8>);
  run_test<element4>(iter_test<element4>);
  run_test<element2>(iter_test<element2>);
  run_test<element1>(iter_test<element1>);
}
CATCH_TEST_CASE("Map Test", "[ds][darray]") {
  run_test<element8>(map_test<element8>);
  run_test<element4>(map_test<element4>);
  run_test<element2>(map_test<element2>);
  run_test<element1>(map_test<element1>);
}
CATCH_TEST_CASE("Print Test", "[ds][darray]") {
  run_test<element8>(print_test<element8>);
}
