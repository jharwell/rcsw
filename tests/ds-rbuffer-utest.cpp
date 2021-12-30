/**
 * @file rbuffer-test.cpp
 *
 * Test of rbuffer module.
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
#include "rcsw/ds/rbuffer.h"
#include "rcsw/utils/utils.h"
#include "tests/ds_test.h"
}

#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_PREFIX_ALL
#include <catch.hpp>

/******************************************************************************
 * Constant declarations
 *****************************************************************************/
#define NUM_TESTS       6
#define MODULE_ID M_TESTING

/*******************************************************************************
 * Test Functions
 ******************************************************************************/
/**
 * @brief Test adding things to the rbuffer and then reading them out
 */
static void rdwr_test(int len, struct ds_params *  params);

/**
 * @brief Test adding/removing things from the rbuffer in tandem, including
 * overwrites.
 */
static void overwrite_test(int len, struct ds_params *  params);

/**
 * @brief Test of \ref rbuffer_map()
 */
static void map_test(int len, struct ds_params *  params);

/**
 * @brief Test using the rbuffer as a FIFO
 */
static void fifo_test(int len, struct ds_params * params);

/**
 * @brief Test of \ref rbuffer_inject()
 *
 */
static void inject_test(int len, struct ds_params * params);

/**
 * @brief Test of rbuffer iteration
 */
static void iter_test(int len, struct ds_params * params);

/*******************************************************************************
 * Test Helper Functions
 ******************************************************************************/
static void test_runner(void (*test)(int len, struct ds_params *params)) {
  /* dbg_init(); */
  /* dbg_insmod(M_TESTING,"Testing"); */
  /* dbg_insmod(M_DS_RBUFFER,"RBuffer"); */

  struct ds_params params;
  params.tag = DS_RBUFFER;
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

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("rbuffer RDWR Test", "[rbuffer]") { test_runner(rdwr_test); }
CATCH_TEST_CASE("rbuffer Overwrite Test", "[rbuffer]") { test_runner(overwrite_test); }
CATCH_TEST_CASE("rbuffer Map Test", "[rbuffer]") { test_runner(map_test); }
CATCH_TEST_CASE("rbuffer FIFO Test", "[rbuffer]") { test_runner(fifo_test); }
CATCH_TEST_CASE("rbuffer Inject Test", "[rbuffer]") { test_runner(inject_test); }
CATCH_TEST_CASE("rbuffer Iterator Test", "[rbuffer]") { test_runner(iter_test); }

static void rdwr_test(int len, struct ds_params *  params) {
    struct rbuffer *rb;
    struct rbuffer myrb;
    int i;

    params->flags &= ~DS_RBUFFER_AS_FIFO;
    rb = rbuffer_init(&myrb, params);
    CATCH_REQUIRE(NULL != rb);

    for (i = 0; i < len; i++) {
      struct element e = {.value1 = i, .value2 = 17};
        CATCH_REQUIRE(rbuffer_add(rb, &e) == OK);
    } /* for() */

    /* verify rbuffer contents */
    for (i = 0; i < len; ++i) {
      struct element * e = (element*)rbuffer_data_get(rb, i);
        CATCH_REQUIRE((int)i == e->value1);
    } /* for() */

    /* test reading out rbuffer contents */
    for (i = 0; i < len; ++i) {
        struct element e;
        CATCH_REQUIRE(rbuffer_remove(rb, &e) == OK);
        CATCH_REQUIRE((int)i == e.value1);
    } /* for() */

    rbuffer_destroy(rb);
} /* rdwr_test() */

static void overwrite_test(int len, struct ds_params *  params) {
    struct rbuffer *rb;
    struct rbuffer myrb;
    int i;
    struct element arr[len*len];

    params->flags &= ~DS_RBUFFER_AS_FIFO;
    rb = rbuffer_init(&myrb, params);

    unsigned tail = 0;
    unsigned count = 0;

    /* test overwriting */
    for (i = 0; i < len * len; i++) {
      struct element e = {.value1 = i, .value2 = 17};
        CATCH_REQUIRE(rbuffer_add(rb, &e) == OK);
        arr[i] = e;
        if (i >= len) {
            tail++;
        }
        count = 0;
        struct element * ep = NULL;
        struct ds_iterator *iter = ds_iter_init(DS_RBUFFER, rb, NULL);
        while ((ep = (element*)ds_iter_next(iter)) != NULL) {
          CATCH_REQUIRE(memcmp(ep, &arr[tail+count+rb->start],
                         sizeof(struct element)) == 0);
          count++;
        } /* while() */
    } /* for() */

    rbuffer_destroy(rb);
} /* overwrite_test() */

static void map_test(int len, struct ds_params * params){
    struct rbuffer *rb;
    struct rbuffer myrb;
    int i;
    int arr[len];

    rb = rbuffer_init(&myrb, params);
    CATCH_REQUIRE(rb != NULL);

    for (i = 0; i < len; i++) {
      struct element e = {.value1 = i, .value2 = 17};
        arr[i] = i;
        CATCH_REQUIRE(rbuffer_add(rb, &e) == OK);
    } /* for() */

    CATCH_REQUIRE(rbuffer_map(rb, th_map_func) == OK);

    for (i = 0; i < len; ++i) {
        struct element e;
        CATCH_REQUIRE(rbuffer_remove(rb, &e) == OK);
        CATCH_REQUIRE(arr[i] == e.value1 +1);
    } /* for() */

    rbuffer_destroy(rb);
} /* map_test() */

static void fifo_test(int len, struct ds_params * params) {
    struct rbuffer *rb;
    struct rbuffer myrb;
    int i;
    struct element arr[len];

    if (!(params->flags &= DS_RBUFFER_AS_FIFO)) {
        return;
    }

    rb = rbuffer_init(&myrb, params);
    CATCH_REQUIRE(NULL != rb);

    /* fill the FIFO, verifying it does not allow addition of items once full */
    unsigned curr_old, start_old;
    for (i = 0; i < len*2; ++i) {
      struct element e = {.value1 = i, .value2 = 17};
        if (i < len) {
            arr[i] = e;
            CATCH_REQUIRE(rbuffer_add(rb, &e) == OK);
        } else {
            curr_old = rb->current;
            start_old = rb->start;
            CATCH_REQUIRE(rbuffer_add(rb, &e) == ERROR);
            CATCH_REQUIRE(errno == ENOSPC);
            CATCH_REQUIRE((curr_old == rb->current && start_old == rb->start));
        }
    } /* for() */

    /* perform a rolling test */
    for (i = 0; i < len; ++i) {
      if (rb->current == (unsigned)len) {
            struct element e;
            CATCH_REQUIRE(rbuffer_remove(rb, &e) == OK);
            CATCH_REQUIRE(arr[0].value1 == e.value1);
            memmove(arr, arr+1, sizeof(arr) - sizeof(struct element));
        } else {
          struct element e = {.value1 = i, .value2 = 17};
            arr[len -1] = e;
            CATCH_REQUIRE(rbuffer_add(rb, &e) == OK);
        }
    } /* for() */

    rbuffer_destroy(rb);
} /* fifo_test() */

static void inject_test(int len, struct ds_params * params) {
    struct rbuffer *rb;
    struct rbuffer myrb;
    int i;
    int sum = 0;

    rb = rbuffer_init(&myrb, params);
    CATCH_REQUIRE(NULL != rb);

    for (i = 0; i < len; i++) {
      struct element e = {.value1 = i, .value2 = 17};
        sum +=i;
        CATCH_REQUIRE(rbuffer_add(rb, &e) == OK);
    } /* for() */

    int total = 0;
    CATCH_REQUIRE(rbuffer_inject(rb, th_inject_func, &total) == OK);
    CATCH_REQUIRE(total == sum);

    rbuffer_destroy(rb);
} /* inject_test() */

static void iter_test(int len, struct ds_params * params) {
    struct rbuffer *rb;
    struct rbuffer myrb;
    int i;
    int arr[len * len];

    params->flags &= ~DS_RBUFFER_AS_FIFO;
    rb = rbuffer_init(&myrb, params);
    CATCH_REQUIRE(rb);

    for (i = 0; i < len; i++) {
      struct element e = {.value1 = i, .value2 = 17};
        CATCH_REQUIRE(rbuffer_add(rb, &e) == OK);
    } /* for() */

    /* test iteration of without overwriting */
    struct element * e;
    struct ds_iterator * iter = ds_iter_init(DS_RBUFFER, rb, th_iter_func);
    CATCH_REQUIRE(iter != NULL);
    while ((e = (element*)ds_iter_next(iter)) != NULL) {
      CATCH_REQUIRE(e->value1 % 2 == 0);
    } /* while() */

    iter = ds_iter_init(DS_RBUFFER, rb, NULL);
    CATCH_REQUIRE(iter);
    unsigned count = 0;
    while ((e = (element*)ds_iter_next(iter)) != NULL) {
      CATCH_REQUIRE((unsigned)e->value1 == count);
        count++;
    } /* while() */
    CATCH_REQUIRE(count == rb->current);

    /* test iteration with overwriting */
    CATCH_REQUIRE(rbuffer_clear(rb) == OK);
    unsigned tail = 0;
    for (i = 0; i < len * len; i++) {
      struct element e2 = {.value1 = i, .value2 = 17};
        CATCH_REQUIRE(rbuffer_add(rb, &e2) == OK);
        arr[i] = i;
        if (i >= len) {
            tail++;
        }
        /* verify iteration */
        count = 0;
        struct element * ep = NULL;
        iter = ds_iter_init(DS_RBUFFER, rb, NULL);
        while ((ep = (element*)ds_iter_next(iter)) != NULL) {
          CATCH_REQUIRE(ep->value1 == arr[tail+count+rb->start]);
            count++;
        } /* while() */
   } /* for() */

    rbuffer_destroy(rb);
} /* iter_test() */
