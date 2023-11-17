/**
 * \file swbus-test.cpp
 * \brief Test of SWBUS features.
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
#include <thread>
#include <mutex>

#include "rcsw/swbus/swbus.h"
#include "tests/swbus_test.h"
#include "rcsw/er/client.h"
#include "tests/ds_test.h"

/*******************************************************************************
 * Namespaces/Decls
 ******************************************************************************/
using swbus_test = void(*)(const struct swbus_params* const params,
                           size_t n_threads);
#define TH_MAX_POOLS 16
#define TH_MAX_RXQS 16
#define TH_MAX_SUBS 512
#define TH_RXQ_SIZE 1024
#define TH_MAX_BUFSIZE 512
#define TH_MAX_PID 16

/*******************************************************************************
 * Test Helper Functions
 ******************************************************************************/
static void run_test(swbus_test test, size_t n_threads = 1) {
  RCSW_ER_INIT(TH_ZLOG_CONF);

  struct swbus_params bus_params;
  bus_params.max_pools = TH_MAX_POOLS;
  bus_params.max_rxqs = TH_MAX_RXQS;
  bus_params.max_subs = TH_MAX_SUBS;
  bus_params.pools = (struct mpool_params*)malloc(sizeof(struct mpool_params)*TH_MAX_POOLS);
  strncpy(bus_params.name, "TESTBUS", sizeof(bus_params.name));

  for (size_t i = 0; i < TH_MAX_POOLS; ++i) {
    bus_params.pools[i].elements = (dptr_t*)malloc(mpool_element_space(TH_MAX_BUFSIZE,
                                                                     TH_RXQ_SIZE));
    bus_params.pools[i].meta = (dptr_t*)malloc(mpool_meta_space(TH_RXQ_SIZE));
    CATCH_REQUIRE(nullptr != bus_params.pools[i].elements);
    CATCH_REQUIRE(nullptr != bus_params.pools[i].meta);
    bus_params.pools[i].max_elts = TH_RXQ_SIZE;
    bus_params.pools[i].elt_size = TH_MAX_BUFSIZE / (TH_MAX_POOLS - i);
    bus_params.pools[i].flags = RCSW_NONE;
  } /* for() */

  uint32_t flags[] = {
    RCSW_NONE,
    RCSW_NOALLOC_HANDLE,
    RCSW_SWBUS_ASYNC
  };

  for (size_t i = 0; i < RCSW_ARRAY_ELTS(flags); ++i) {
    bus_params.flags = flags[i];
    test(&bus_params, n_threads);
  } /* for(i..) */


  for (size_t i = 0; i < TH_MAX_POOLS; ++i) {
    free(bus_params.pools[i].elements);
    free(bus_params.pools[i].meta);
  } /* for() */
  free(bus_params.pools);

  RCSW_ER_DEINIT();
} /* test_runner() */

/*******************************************************************************
 * Test Functions
 ******************************************************************************/
/**
 * \brief Test SWBUS initialization (verrrryyy simplistic sanity test).
 */
static void init_test(const struct swbus_params * params, size_t) {
  struct swbus myswbus;
  struct swbus *swbus;

  swbus = swbus_init(&myswbus, params);
  CATCH_REQUIRE(nullptr != swbus);

  swbus_destroy(swbus);
} /* init_test() */

/**
 * \brief Test subscribing an RXQ to multiple packets. Doesn't test publishing.
 */
static void subscribe_test(const struct swbus_params * params, size_t) {
  struct swbus_rxq_ent rxq_buf[RXQ_SIZE];
  struct swbus myswbus;
  struct swbus * swbus;

  swbus = swbus_init(&myswbus, params);
  CATCH_REQUIRE(nullptr != swbus);

  struct pcqueue * rxq = swbus_rxq_init(swbus, rxq_buf, RXQ_SIZE);
  CATCH_REQUIRE(nullptr != rxq);

  for (size_t i = 0; i < TH_MAX_SUBS; ++i) {
    CATCH_REQUIRE(llist_size(swbus->subscribers) == i);
    CATCH_REQUIRE(swbus_subscribe(swbus, rxq, i) == OK);
    CATCH_REQUIRE(llist_size(swbus->subscribers) == i + 1);
  } /* for() */

  for (size_t i = 0; i < TH_MAX_SUBS; ++i) {
    CATCH_REQUIRE(llist_size(swbus->subscribers) == TH_MAX_SUBS - i);
    CATCH_REQUIRE(swbus_unsubscribe(swbus, rxq, i) == OK);
    CATCH_REQUIRE(llist_size(swbus->subscribers) == TH_MAX_SUBS - i - 1);
  } /* for() */

  CATCH_REQUIRE(llist_isempty(swbus->subscribers));

  swbus_destroy(swbus);
} /* subscribe_test() */

/**
 * \brief Test serially filling all SWBUS buffer pools.
 *
 * - Publishing multiple packet types
 * - Pushing packets to multiple to multiple RXQs
 */

static void serial_stress_test(const struct swbus_params * params, size_t) {
  struct swbus myswbus;
  struct swbus * swbus;

  swbus = swbus_init(&myswbus, params);
  CATCH_REQUIRE(nullptr != swbus);
  uint8_t* buf = (uint8_t*)malloc(TH_RXQ_SIZE);
  struct pcqueue * rxq = swbus_rxq_init(swbus, nullptr, TH_RXQ_SIZE);
  struct pcqueue * rxq2 = swbus_rxq_init(swbus, nullptr, TH_RXQ_SIZE);
  CATCH_REQUIRE(nullptr != rxq);
  CATCH_REQUIRE(nullptr != rxq2);

  /*
   * Subscribe to a packet ID, and fill the bus with packets corresponding to
   * that ID. The published data is invalid/ not initialized.
   */
  CATCH_REQUIRE(swbus_subscribe(swbus, rxq, 0) == OK);
  CATCH_REQUIRE(swbus_subscribe(swbus, rxq2, 0) == OK);
  CATCH_REQUIRE(swbus_subscribe(swbus, rxq, 1) == OK);
  CATCH_REQUIRE(swbus_subscribe(swbus, rxq2, 1) == OK);

  for (size_t i = 0; i < params->max_pools; ++i) {
    struct mpool* bp = &swbus->pools[i];
    for (size_t j = 0; j < mpool_capacity(bp) / 2; ++j) {
      CATCH_REQUIRE(pcqueue_size(rxq) == 2 * j);
      CATCH_REQUIRE(pcqueue_size(rxq2) == 2 * j);
      CATCH_REQUIRE(swbus_publish(swbus,
                                  0,
                                  params->pools[i].elt_size,
                                  buf) == OK);
      CATCH_REQUIRE(pcqueue_size(rxq) == 2* j + 1);
      CATCH_REQUIRE(pcqueue_size(rxq2) == 2 * j + 1);
      CATCH_REQUIRE(swbus_publish(swbus,
                                  1,
                                  params->pools[i].elt_size,
                                  buf) == OK);
      CATCH_REQUIRE(pcqueue_size(rxq) == 2* j + 2);
      CATCH_REQUIRE(pcqueue_size(rxq2) == 2* j + 2);
    } /* for(j..) */

    CATCH_REQUIRE(llist_isfull(&bp->alloc));
    CATCH_REQUIRE(llist_isempty(&bp->free));

    /*
     * Dequeue packets for this buffer pool, verifying everything as we go.
     */
    for (size_t j = 0; j < mpool_capacity(bp); ++j) {
      struct swbus_rxq_ent* ptr = swbus_rxq_front(rxq);
      struct swbus_rxq_ent* ptr2 = swbus_rxq_front(rxq2);

      CATCH_REQUIRE(nullptr != ptr);
      CATCH_REQUIRE(nullptr != ptr2);
      CATCH_REQUIRE(ptr->data == ptr2->data);
      CATCH_REQUIRE(ptr->pid == ptr2->pid);
      CATCH_REQUIRE(ptr->pkt_size == ptr2->pkt_size);

      CATCH_REQUIRE(pcqueue_size(rxq) == TH_RXQ_SIZE - j);
      CATCH_REQUIRE(pcqueue_size(rxq2) == TH_RXQ_SIZE - j);
      CATCH_REQUIRE(mpool_ref_count(bp, ptr->data) == 2);

      CATCH_REQUIRE(OK == swbus_rxq_pop_front(rxq, ptr));
      CATCH_REQUIRE(pcqueue_size(rxq) == TH_RXQ_SIZE - j - 1);
      CATCH_REQUIRE(mpool_ref_count(bp, ptr->data) == 1);

      CATCH_REQUIRE(OK == swbus_rxq_pop_front(rxq2, ptr2));
      CATCH_REQUIRE(pcqueue_size(rxq2) == TH_RXQ_SIZE - j - 1);
      CATCH_REQUIRE(mpool_ref_count(bp, ptr->data) == 0);
    } /* for(j..) */
  } /* for(i..) */

  swbus_destroy(swbus);
  free(buf);
}
/**
 * \brief Test SWBUS in a concurrent setting
 *
 * - Publishing multiple packet types
 * - Pushing packets to multiple to multiple RXQs
 */

static void concurrent_stress_test(const struct swbus_params * params,
                                   size_t n_threads) {
  struct swbus myswbus;
  struct swbus * swbus;

  swbus = swbus_init(&myswbus, params);
  CATCH_REQUIRE(nullptr != swbus);
  uint8_t* buf = (uint8_t*)malloc(TH_RXQ_SIZE);
  size_t n_publishers = n_threads;
  size_t n_consumers = 10; /* always 10 to make RXQ usage easier here */
  struct pcqueue* rxqs[10];

  for (size_t i = 0; i < n_consumers; ++i) {
    rxqs[i] = swbus_rxq_init(swbus, nullptr, TH_RXQ_SIZE);
    CATCH_REQUIRE(nullptr != rxqs[i]);
  } /* for(i..) */

  /*
   * Each RXQ is subscribed to a random subset of PIDs [0,...,15].
   */
  std::map<size_t, std::vector<size_t>> subscriptions = {
    {0, {19, 1, 2}},
    {1, {11, 4}},
    {2, {4,8,7,6}},
    {3, {4,13,7,6}},
    {4, {12, 1}},
    {5, {3,9,2,6}},
    {6, {4,10,7,15}},
    {7, {4,8,7,6}},
    {8, {4,10,7,14}},
    {9, {4,8,7,10}},
  };
  for (auto &pair : subscriptions) {
    for (size_t i = 0; i < pair.second.size(); ++i) {
      CATCH_REQUIRE(swbus_subscribe(swbus,
                                    rxqs[pair.first],
                                    pair.second[i]) == OK);
    } /* for(i..) */
  } /* for(&pair..) */


  std::vector<bool> checks;
  std::mutex mtx;

  auto pub_cb = [&]() {
                  for (size_t i = 0; i < TH_RXQ_SIZE * 2 / n_threads; ++i) {
                    status_t rval = swbus_publish(swbus,
                                         i % TH_MAX_PID,
                                         params->pools[rand() % params->max_pools].elt_size,
                                         buf);
                    mtx.lock();
                    checks.push_back(rval == OK);
                    mtx.unlock();
                  } /* for(i..) */
                };
  auto sub_cb = [&](size_t id) {
                  size_t count = 0;
                  status_t rval;
                  struct timespec to = {.tv_sec = 0,
                                        .tv_nsec = (rand()  % 1000) + 1};
                  auto& subs = subscriptions[id];
                  while (count < 100) {
                    struct swbus_rxq_ent* ent = swbus_rxq_timedwait(swbus,
                                                                    rxqs[id],
                                                                    &to);
                    if (nullptr == ent) {
                      continue;
                    }
                    ++count;
                    mtx.lock();
                    checks.push_back(std::find(subs.begin(),
                                               subs.end(),
                                               ent->pid) != subs.end());

                    mtx.unlock();

                    rval = swbus_rxq_pop_front(rxqs[id],
                                               ent);
                    mtx.lock();
                    checks.push_back(rval == OK);
                    mtx.unlock();
                  }
                };

  CATCH_REQUIRE(std::all_of(std::begin(checks),
                            std::end(checks),
                            [&](bool val) {
                              return val;
                            }));

  std::vector<std::thread> consumers;
  std::vector<std::thread> publishers;
  for (size_t i = 0; i < n_consumers ; ++i) {
    consumers.push_back(std::thread(sub_cb, i));
  } /* for(i..) */

  for (size_t i = 0; i < n_publishers ; ++i) {
    publishers.push_back(std::thread(pub_cb));
  } /* for(i..) */

  for (size_t i = 0; i < n_consumers; ++i) {
    consumers[i].join();
  } /* for(i..) */

  for (size_t i = 0; i < n_publishers; ++i) {
    publishers[i].join();
  } /* for(i..) */

  /* drain RXQs */
  for (size_t i = 0; i < n_consumers; ++i) {
    auto& subs = subscriptions[i];
    while (!pcqueue_isempty(rxqs[i])) {
      struct swbus_rxq_ent* ent = swbus_rxq_front(rxqs[i]);
      CATCH_REQUIRE(std::find(subs.begin(),
                              subs.end(),
                              ent->pid) != subs.end());
      CATCH_REQUIRE(OK == swbus_rxq_pop_front(rxqs[i],
                                              ent));
    }
  } /* for(i..) */


  for (size_t i = 0; i < params->max_pools; ++i) {
    struct mpool* bp = &swbus->pools[i];
    CATCH_REQUIRE(llist_isfull(&bp->free));
    CATCH_REQUIRE(llist_isempty(&bp->alloc));
  }

  swbus_destroy(swbus);
  free(buf);
}

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("Init Test", "[swbus]") { run_test(init_test); }
CATCH_TEST_CASE("Subscribe Test", "[swbus]") { run_test(subscribe_test); }
CATCH_TEST_CASE("Serial Multi-RXQ, Multi-PID", "[swbus]") {
  run_test(serial_stress_test);
}
CATCH_TEST_CASE("Concurrent Multi-RXQ, Multi-PID", "[swbus]") {
  for (size_t i = 2; i < 10; ++i) {
    run_test(concurrent_stress_test, i);
  } /* for(i..) */
}
