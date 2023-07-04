/**
 * \file pulse-test.cpp
 * \brief Test of PULSE features.
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

#include "rcsw/pulse/pulse.h"
#include "tests/pulse_test.h"
#include "rcsw/er/client.h"

/*******************************************************************************
 * Namespaces/Decls
 ******************************************************************************/
using pulse_test = void(*)(const struct pulse_params* const params,
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
static void run_test(pulse_test test, size_t n_threads = 1) {
  RCSW_ER_INIT();
  RCSW_ER_INSMOD(M_PULSE, "rcsw.pulse");
  RCSW_ER_INSMOD(M_MT_MPOOL, "rcsw.mt.mpool");
  log4cl_mod_lvl_set(M_PULSE, RCSW_ERL_DEBUG);
  log4cl_mod_lvl_set(M_MT_MPOOL, RCSW_ERL_INFO);
  struct pulse_params bus_params;
  bus_params.max_pools = TH_MAX_POOLS;
  bus_params.max_rxqs = TH_MAX_RXQS;
  bus_params.max_subs = TH_MAX_SUBS;
  bus_params.pools = (struct mpool_params*)malloc(sizeof(struct mpool_params)*TH_MAX_POOLS);
  strncpy(bus_params.name, "TESTBUS", sizeof(bus_params.name));

  for (size_t i = 0; i < TH_MAX_POOLS; ++i) {
    bus_params.pools[i].elements = (uint8_t*)malloc(pulse_pool_space(TH_MAX_BUFSIZE,
                                                                     TH_RXQ_SIZE));
    bus_params.pools[i].nodes = (uint8_t*)malloc(pulse_meta_space(TH_RXQ_SIZE));
    CATCH_REQUIRE(nullptr != bus_params.pools[i].elements);
    CATCH_REQUIRE(nullptr != bus_params.pools[i].nodes);
    bus_params.pools[i].max_elts = TH_RXQ_SIZE;
    bus_params.pools[i].elt_size = TH_MAX_BUFSIZE / (TH_MAX_POOLS - i);
    bus_params.pools[i].flags = RCSW_NONE;
  } /* for() */

  uint32_t flags[] = {
    RCSW_NONE,
    RCSW_NOALLOC_HANDLE,
    RCSW_PULSE_ASYNC
  };

  for (size_t i = 0; i < RCSW_ARRAY_ELTS(flags); ++i) {
    bus_params.flags = flags[i];
    test(&bus_params, n_threads);
  } /* for(i..) */


  for (size_t i = 0; i < TH_MAX_POOLS; ++i) {
    free(bus_params.pools[i].elements);
    free(bus_params.pools[i].nodes);
  } /* for() */

  RCSW_ER_DEINIT();
} /* test_runner() */

/*******************************************************************************
 * Test Functions
 ******************************************************************************/
/**
 * \brief Test PULSE initialization (verrrryyy simplistic sanity test).
 */
static void init_test(const struct pulse_params * params, size_t) {
  struct pulse mypulse;
  struct pulse *pulse;

  pulse = pulse_init(&mypulse, params);
  CATCH_REQUIRE(nullptr != pulse);

  pulse_destroy(pulse);
} /* init_test() */

/**
 * \brief Test subscribing an RXQ to multiple packets. Doesn't test publishing.
 */
static void subscribe_test(const struct pulse_params * params, size_t) {
  struct pulse_rxq_ent rxq_buf[RXQ_SIZE];
  struct pulse mypulse;
  struct pulse * pulse;

  pulse = pulse_init(&mypulse, params);
  CATCH_REQUIRE(nullptr != pulse);

  struct pcqueue * rxq = pulse_rxq_init(pulse, rxq_buf, RXQ_SIZE);
  CATCH_REQUIRE(nullptr != rxq);

  for (size_t i = 0; i < TH_MAX_SUBS; ++i) {
    CATCH_REQUIRE(llist_size(pulse->subscribers) == i);
    CATCH_REQUIRE(pulse_subscribe(pulse, rxq, i) == OK);
    CATCH_REQUIRE(llist_size(pulse->subscribers) == i + 1);
  } /* for() */

  for (size_t i = 0; i < TH_MAX_SUBS; ++i) {
    CATCH_REQUIRE(llist_size(pulse->subscribers) == TH_MAX_SUBS - i);
    CATCH_REQUIRE(pulse_unsubscribe(pulse, rxq, i) == OK);
    CATCH_REQUIRE(llist_size(pulse->subscribers) == TH_MAX_SUBS - i - 1);
  } /* for() */

  CATCH_REQUIRE(llist_isempty(pulse->subscribers));

  pulse_destroy(pulse);
} /* subscribe_test() */

/**
 * \brief Test serially filling all PULSE buffer pools.
 *
 * - Publishing multiple packet types
 * - Pushing packets to multiple to multiple RXQs
 */

static void serial_stress_test(const struct pulse_params * params, size_t) {
  struct pulse mypulse;
  struct pulse * pulse;

  pulse = pulse_init(&mypulse, params);
  CATCH_REQUIRE(nullptr != pulse);
  uint8_t* buf = (uint8_t*)malloc(TH_RXQ_SIZE);
  struct pcqueue * rxq = pulse_rxq_init(pulse, nullptr, TH_RXQ_SIZE);
  struct pcqueue * rxq2 = pulse_rxq_init(pulse, nullptr, TH_RXQ_SIZE);
  CATCH_REQUIRE(nullptr != rxq);
  CATCH_REQUIRE(nullptr != rxq2);

  /*
   * Subscribe to a packet ID, and fill the bus with packets corresponding to
   * that ID. The published data is invalid/ not initialized.
   */
  CATCH_REQUIRE(pulse_subscribe(pulse, rxq, 0) == OK);
  CATCH_REQUIRE(pulse_subscribe(pulse, rxq2, 0) == OK);
  CATCH_REQUIRE(pulse_subscribe(pulse, rxq, 1) == OK);
  CATCH_REQUIRE(pulse_subscribe(pulse, rxq2, 1) == OK);

  for (size_t i = 0; i < params->max_pools; ++i) {
    struct mpool* bp = &pulse->pools[i];
    for (size_t j = 0; j < mpool_capacity(bp) / 2; ++j) {
      CATCH_REQUIRE(pcqueue_size(rxq) == 2 * j);
      CATCH_REQUIRE(pcqueue_size(rxq2) == 2 * j);
      CATCH_REQUIRE(pulse_publish(pulse,
                                  0,
                                  params->pools[i].elt_size,
                                  buf) == OK);
      CATCH_REQUIRE(pcqueue_size(rxq) == 2* j + 1);
      CATCH_REQUIRE(pcqueue_size(rxq2) == 2 * j + 1);
      CATCH_REQUIRE(pulse_publish(pulse,
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
      struct pulse_rxq_ent* ptr = pulse_rxq_front(rxq);
      struct pulse_rxq_ent* ptr2 = pulse_rxq_front(rxq2);

      CATCH_REQUIRE(nullptr != ptr);
      CATCH_REQUIRE(nullptr != ptr2);
      CATCH_REQUIRE(ptr->data == ptr2->data);
      CATCH_REQUIRE(ptr->pid == ptr2->pid);
      CATCH_REQUIRE(ptr->pkt_size == ptr2->pkt_size);

      CATCH_REQUIRE(pcqueue_size(rxq) == TH_RXQ_SIZE - j);
      CATCH_REQUIRE(pcqueue_size(rxq2) == TH_RXQ_SIZE - j);
      CATCH_REQUIRE(mpool_ref_count(bp, ptr->data) == 2);

      CATCH_REQUIRE(OK == pulse_rxq_pop_front(rxq, ptr));
      CATCH_REQUIRE(pcqueue_size(rxq) == TH_RXQ_SIZE - j - 1);
      CATCH_REQUIRE(mpool_ref_count(bp, ptr->data) == 1);

      CATCH_REQUIRE(OK == pulse_rxq_pop_front(rxq2, ptr2));
      CATCH_REQUIRE(pcqueue_size(rxq2) == TH_RXQ_SIZE - j - 1);
      CATCH_REQUIRE(mpool_ref_count(bp, ptr->data) == 0);
    } /* for(j..) */
  } /* for(i..) */

  pulse_destroy(pulse);
  free(buf);
}
/**
 * \brief Test PULSE in a concurrent setting
 *
 * - Publishing multiple packet types
 * - Pushing packets to multiple to multiple RXQs
 */

static void concurrent_stress_test(const struct pulse_params * params,
                                   size_t n_threads) {
  struct pulse mypulse;
  struct pulse * pulse;

  pulse = pulse_init(&mypulse, params);
  CATCH_REQUIRE(nullptr != pulse);
  uint8_t* buf = (uint8_t*)malloc(TH_RXQ_SIZE);
  size_t n_publishers = n_threads;
  size_t n_consumers = 10; /* always 10 to make RXQ usage easier here */
  struct pcqueue* rxqs[10];

  for (size_t i = 0; i < n_consumers; ++i) {
    rxqs[i] = pulse_rxq_init(pulse, nullptr, TH_RXQ_SIZE);
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
      CATCH_REQUIRE(pulse_subscribe(pulse,
                                    rxqs[pair.first],
                                    pair.second[i]) == OK);
    } /* for(i..) */
  } /* for(&pair..) */


  std::vector<bool> checks;
  std::mutex mtx;

  auto pub_cb = [&]() {
                  for (size_t i = 0; i < TH_RXQ_SIZE * 2 / n_threads; ++i) {
                    status_t rval = pulse_publish(pulse,
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
                    struct pulse_rxq_ent* ent = pulse_rxq_timedwait(pulse,
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

                    rval = pulse_rxq_pop_front(rxqs[id],
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
      struct pulse_rxq_ent* ent = pulse_rxq_front(rxqs[i]);
      CATCH_REQUIRE(std::find(subs.begin(),
                              subs.end(),
                              ent->pid) != subs.end());
      CATCH_REQUIRE(OK == pulse_rxq_pop_front(rxqs[i],
                                              ent));
    }
  } /* for(i..) */


  for (size_t i = 0; i < params->max_pools; ++i) {
    struct mpool* bp = &pulse->pools[i];
    CATCH_REQUIRE(llist_isfull(&bp->free));
    CATCH_REQUIRE(llist_isempty(&bp->alloc));
  }

  pulse_destroy(pulse);
  free(buf);
}

/*******************************************************************************
 * Test Cases
 ******************************************************************************/
CATCH_TEST_CASE("Init Test", "[pulse]") { run_test(init_test); }
CATCH_TEST_CASE("Subscribe Test", "[pulse]") { run_test(subscribe_test); }
CATCH_TEST_CASE("Serial Multi-RXQ, Multi-PID", "[pulse]") {
  run_test(serial_stress_test);
}
CATCH_TEST_CASE("Concurrent Multi-RXQ, Multi-PID", "[pulse]") {
  for (size_t i = 2; i < 10; ++i) {
    run_test(concurrent_stress_test, i);
  } /* for(i..) */
}
