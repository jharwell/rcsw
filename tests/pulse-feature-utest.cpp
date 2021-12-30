/**
 * @file pulse-feature-test.cpp
 * @brief Test of basic PULSE features.
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
#include "rcsw/pulse/pulse.h"
#include "pulse_test.h"
#include "rcsw/common/dbg.h"
#define CATCH_CONFIG_MAIN
#define CATCH_CONFIG_PREFIX_ALL
#include <catch.hpp>

/*******************************************************************************
 * Constant Definitions
 ******************************************************************************/
#define NUM_TESTS            3
#define MODULE_ID M_TESTING
#define MAX_POOLS 16
#define MAX_RXQS 16
#define MAX_SUBS 128

/*******************************************************************************
 * Test Helper Functions
 ******************************************************************************/
static void test_runner1(void (*test)(struct pulse_params* params)) {
  struct pulse_params bus_params;
  bus_params.n_pools = MAX_POOLS;
  memset(bus_params.name, 0, sizeof(bus_params.name));
  dbg_init();
  dbg_insmod(M_PULSE, "PULSE");
  dbg_insmod(M_DS_RBUFFER, "RBUFFER");
  dbg_mod_lvl_set(M_PULSE, DBG_V);
  dbg_mod_lvl_set(M_DS_RBUFFER, DBG_V);

  bus_params.pools = (pulse_bp_params*)malloc(sizeof(struct pulse_bp_params)*MAX_POOLS);
  for (int i = 0; i < MAX_POOLS; ++i) {
    bus_params.pools[i].elements = (uint8_t*)malloc(pulse_pool_space(MAX_BUFFER_SIZE,
                                                                        1024));
    bus_params.pools[i].nodes = (uint8_t*)malloc(pulse_node_space(1024));
    CATCH_REQUIRE(NULL != bus_params.pools[i].elements);
    CATCH_REQUIRE(NULL != bus_params.pools[i].nodes);
    bus_params.pools[i].n_bufs = 1024;
    bus_params.pools[i].buf_size = MAX_BUFFER_SIZE;
    bus_params.max_rxqs = MAX_RXQS;
    bus_params.max_subs = MAX_SUBS;
  } /* for() */

  for (int i = 0x4; i < 0x10; ++i) {
    bus_params.flags = i;
    test(&bus_params);
  } /* for(i..) */


  for (int i = 0; i < MAX_POOLS; ++i) {
    free(bus_params.pools[i].elements);
    free(bus_params.pools[i].nodes);
  } /* for() */
} /* test_runner1() */

/*******************************************************************************
 * Test Functions
 ******************************************************************************/
static void init_test(struct pulse_params* params);
static void subscribe_test(struct pulse_params *params);
static void publish_test(struct pulse_params *params);
static void fill_test(struct pulse_params *params);

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("PULSE Init Test", "[PULSE]") { test_runner1(init_test); }
/* CATCH_TEST_CASE("PULSE Subscribe Test", "[PULSE]") { test_runner1(subscribe_test); } */
/* CATCH_TEST_CASE("PULSE Fill Test", "[PULSE]") { test_runner1(fill_test); } */

static void init_test(struct pulse_params * params) {
  struct pulse_inst mypulse;
  struct pulse_inst *pulse;

  pulse = pulse_init(&mypulse, params);
  CATCH_REQUIRE(NULL != pulse);

  pulse_destroy(pulse);
} /* init_test() */

static void subscribe_test(struct pulse_params * params) {
  struct pulse_rxq_ent rxq_buf[RXQ_SIZE];
  struct pulse_inst mypulse;
  struct pulse_inst * pulse;

  pulse = pulse_init(&mypulse, params);
  CATCH_REQUIRE(NULL != pulse);

  struct mt_queue * rxq = pulse_rxq_init(pulse, rxq_buf, RXQ_SIZE);
  CATCH_REQUIRE(NULL != rxq);

  int i;
  for (i = 0; i < 0xFF; ++i) {
    CATCH_REQUIRE(pulse_subscribe(pulse, rxq, i) == OK);
  } /* for() */
  for (i = 0; i < 0xFF; ++i) {
    CATCH_REQUIRE(pulse_unsubscribe(pulse, rxq, i) == OK);
  } /* for() */

  CATCH_REQUIRE(llist_n_elts(pulse->sub_list) == 0);
  pulse_destroy(pulse);
} /* subscribe_test() */

static void fill_test(struct pulse_params * params) {
  struct pulse_inst mypulse;
  struct pulse_inst * pulse;

  pulse = pulse_init(&mypulse, params);
  CATCH_REQUIRE(NULL != pulse);
  uint8_t* buf = (uint8_t*)malloc(1024);
  struct mt_queue * rxq = pulse_rxq_init(pulse, NULL, 1024);
  struct mt_queue * rxq2 = pulse_rxq_init(pulse, NULL, 1024);
  CATCH_REQUIRE(NULL != rxq);
  CATCH_REQUIRE(NULL != rxq2);
  /*
   * Subscribe to a packet ID, and fill the bus with packets corresponding to
   * that ID. The published data is invalid/ not initialized.
   */
  CATCH_REQUIRE(pulse_subscribe(pulse,rxq,0) == OK);
  CATCH_REQUIRE(pulse_subscribe(pulse,rxq2,0) == OK);
  for (int i = 0; i < params->n_pools; ++i) {
    for (int j = 0; j < params->pools[i].n_bufs; ++j) {
      CATCH_REQUIRE(pulse_publish(pulse,
                          0,
                          rand() % params->pools[i].buf_size + 1,
                          buf) == OK);
    } /* for(j..) */
    /* pulse_print(pulse); */
    PRINTF("FREE: %zu ALLOC: %zu\n", llist_n_elts(&pulse->buffer_pools[i].pool.free),
           llist_n_elts(&pulse->buffer_pools[i].pool.alloc));

    CATCH_REQUIRE(llist_isfull(&pulse->buffer_pools[i].pool.alloc));
    CATCH_REQUIRE(llist_isempty(&pulse->buffer_pools[i].pool.free));
    for (int j = 0; j < params->pools[i].n_bufs; ++j) {
      uint8_t* ptr = (uint8_t*)pulse_wait_front(rxq);
      uint8_t* ptr2 = (uint8_t*)pulse_wait_front(rxq2);
      CATCH_REQUIRE(NULL != ptr);
      CATCH_REQUIRE(ptr == ptr2);

      CATCH_REQUIRE(mpool_ref_query(&pulse->buffer_pools[i].pool, ptr) == 2);
      CATCH_REQUIRE(OK == pulse_pop_front(rxq));

      CATCH_REQUIRE(mpool_ref_query(&pulse->buffer_pools[i].pool, ptr) == 1);
      CATCH_REQUIRE(OK == pulse_pop_front(rxq2));
      CATCH_REQUIRE(mpool_ref_query(&pulse->buffer_pools[i].pool, ptr) == -1);
    } /* for(j..) */
  } /* for(i..) */

  pulse_destroy(pulse);
  free(buf);
} /* fill_test() */
