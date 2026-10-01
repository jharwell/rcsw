/**
 * \file
 *
 * \copyright 2017 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * \brief Test of SWBUS features.
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#define CATCH_CONFIG_PREFIX_ALL
#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <map>
#include <mutex>
#include <thread>

#include <catch2/catch_test_macros.hpp>

#include "rcsw/er/client.h"
#include "rcsw/swbus/swbus.h"
#include "tests/unit/element.hpp"
#include "tests/unit/swbus/swbus_test.h"

/*******************************************************************************
 * Namespaces/Decls
 ******************************************************************************/
using swbus_test = void (*)(const struct swbus_config* const config,
                            size_t                           n_threads);
#define TH_MAX_POOLS 16UL
#define TH_MAX_RXQS 16UL
#define TH_MAX_SUBS 512UL
#define TH_RXQ_SIZE 1024UL
#define TH_MAX_BUFSIZE 512UL
#define TH_MAX_PID 16

/*******************************************************************************
 * Test Harness
 ******************************************************************************/
static void run_test(swbus_test test, size_t n_threads = 1) {
  RCSW_ER_INIT(TH_ZLOG_CONF);

  struct swbus_config bus_config;
  bus_config.max_pools = TH_MAX_POOLS;
  bus_config.max_rxqs  = TH_MAX_RXQS;
  bus_config.max_subs  = TH_MAX_SUBS;
  bus_config.pools =
    (struct mpool_config*)malloc(sizeof(struct mpool_config) * TH_MAX_POOLS);
  strncpy(bus_config.name, "TESTBUS", sizeof(bus_config.name));

  for (size_t i = 0; i < TH_MAX_POOLS; ++i) {
    bus_config.pools[i].elements =
      (dptr_t*)malloc(mpool_element_space(TH_MAX_BUFSIZE, TH_RXQ_SIZE));
    bus_config.pools[i].meta = (dptr_t*)malloc(mpool_meta_space(TH_RXQ_SIZE));
    CATCH_REQUIRE(nullptr != bus_config.pools[i].elements);
    CATCH_REQUIRE(nullptr != bus_config.pools[i].meta);
    bus_config.pools[i].max_elts = TH_RXQ_SIZE;
    bus_config.pools[i].elt_size = TH_MAX_BUFSIZE / (TH_MAX_POOLS - i);
    bus_config.pools[i].flags    = RCSW_NONE;
  } /* for() */

  uint32_t flags[] = {RCSW_NONE, RCSW_NOALLOC_HANDLE, RCSW_SWBUS_ASYNC};

  for (size_t i = 0; i < RCSW_ARRAY_ELTS(flags); ++i) {
    bus_config.flags = flags[i];
    test(&bus_config, n_threads);
  } /* for(i..) */

  for (size_t i = 0; i < TH_MAX_POOLS; ++i) {
    free(bus_config.pools[i].elements);
    free(bus_config.pools[i].meta);
  } /* for() */
  free(bus_config.pools);

  RCSW_ER_DEINIT();
} /* test_runner() */

struct swbus*   g_bus;
struct pcqueue* g_rxq;

struct swbus* make_bus(struct mpool_config* pool, uint32_t flags) {
  memset(pool, 0, sizeof(*pool));
  pool->elt_size = 64;
  pool->max_elts = 8;

  struct swbus_config c;
  memset(&c, 0, sizeof(c));
  c.pools     = pool;
  c.max_pools = 1;
  c.max_rxqs  = 1;
  c.max_subs  = 4;
  c.flags     = flags;
  strcpy(c.name, "test");
  return swbus_init(nullptr, &c);
}

void* slow_subscriber(void*) {
  for (int i = 0; i < 5; ++i) {
    struct swbus_rxq_ent* e = swbus_rxq_wait(g_bus, g_rxq);
    usleep(50 * 1000);
    swbus_rxq_pop_front(g_rxq, e);
  }
  return nullptr;
}

void fast_publisher_slow_subscriber(uint32_t flags) {
  struct mpool_config pool;
  g_bus = make_bus(&pool, flags);
  CATCH_REQUIRE(nullptr != g_bus);
  g_rxq = swbus_rxq_init(g_bus, nullptr, 1);
  CATCH_REQUIRE(nullptr != g_rxq);
  CATCH_REQUIRE(OK == swbus_subscribe(g_bus, g_rxq, 1));

  pthread_t t;
  pthread_create(&t, nullptr, slow_subscriber, nullptr);
  char pkt[16] = {0};
  for (int i = 0; i < 5; ++i) {
    /* A full RXQ rejects the publish (ENOSPC); retry until there is space */
    while (OK != swbus_publish(g_bus, 1, sizeof(pkt), pkt)) {
      usleep(10 * 1000);
    }
  }
  pthread_join(t, nullptr);
  swbus_destroy(g_bus);
}

/*******************************************************************************
 * Test Functions
 ******************************************************************************/
/**
 * \brief Test SWBUS initialization (verrrryyy simplistic sanity test).
 */
static void init_test(const struct swbus_config* config, size_t) {
  struct swbus  myswbus;
  struct swbus* swbus;

  swbus = swbus_init(&myswbus, config);
  CATCH_REQUIRE(nullptr != swbus);

  swbus_destroy(swbus);
} /* init_test() */

/**
 * \brief Test subscribing an RXQ to multiple packets. Doesn't test publishing.
 */
static void subscribe_test(const struct swbus_config* config, size_t) {
  struct swbus_rxq_ent rxq_buf[RXQ_SIZE];
  struct swbus         myswbus;
  struct swbus*        swbus;

  swbus = swbus_init(&myswbus, config);
  CATCH_REQUIRE(nullptr != swbus);

  struct pcqueue* rxq = swbus_rxq_init(swbus, rxq_buf, RXQ_SIZE);
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

// NOLINTNEXTLINE(readability-function-size)
static void serial_stress_test(const struct swbus_config* config, size_t) {
  struct swbus  myswbus;
  struct swbus* swbus;

  swbus = swbus_init(&myswbus, config);
  CATCH_REQUIRE(nullptr != swbus);
  uint8_t*        buf  = (uint8_t*)malloc(TH_RXQ_SIZE);
  struct pcqueue* rxq  = swbus_rxq_init(swbus, nullptr, TH_RXQ_SIZE);
  struct pcqueue* rxq2 = swbus_rxq_init(swbus, nullptr, TH_RXQ_SIZE);
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

  for (size_t i = 0; i < config->max_pools; ++i) {
    struct mpool* bp = &swbus->pools[i];
    for (size_t j = 0; j < mpool_capacity(bp) / 2; ++j) {
      CATCH_REQUIRE(pcqueue_size(rxq) == 2 * j);
      CATCH_REQUIRE(pcqueue_size(rxq2) == 2 * j);
      CATCH_REQUIRE(swbus_publish(swbus, 0, config->pools[i].elt_size, buf) ==
                    OK);
      CATCH_REQUIRE(pcqueue_size(rxq) == (2 * j) + 1);
      CATCH_REQUIRE(pcqueue_size(rxq2) == (2 * j) + 1);
      CATCH_REQUIRE(swbus_publish(swbus, 1, config->pools[i].elt_size, buf) ==
                    OK);
      CATCH_REQUIRE(pcqueue_size(rxq) == (2 * j) + 2);
      CATCH_REQUIRE(pcqueue_size(rxq2) == (2 * j) + 2);
    } /* for(j..) */

    CATCH_REQUIRE(llist_isfull(&bp->alloc));
    CATCH_REQUIRE(llist_isempty(&bp->free));

    /*
     * Dequeue packets for this buffer pool, verifying everything as we go.
     */
    for (size_t j = 0; j < mpool_capacity(bp); ++j) {
      struct swbus_rxq_ent* ptr  = swbus_rxq_front(rxq);
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

// NOLINTNEXTLINE(readability-function-size)
static void concurrent_stress_test(const struct swbus_config* config,
                                   size_t                     n_threads) {
  struct swbus  myswbus;
  struct swbus* swbus;

  swbus = swbus_init(&myswbus, config);
  CATCH_REQUIRE(nullptr != swbus);
  uint8_t*        buf          = (uint8_t*)malloc(TH_RXQ_SIZE);
  size_t          n_publishers = n_threads;
  size_t          n_consumers  = 10; /* always 10 to make RXQ usage easier here */
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
    {2, {4, 8, 7, 6}},
    {3, {4, 13, 7, 6}},
    {4, {12, 1}},
    {5, {3, 9, 2, 6}},
    {6, {4, 10, 7, 15}},
    {7, {4, 8, 7, 6}},
    {8, {4, 10, 7, 14}},
    {9, {4, 8, 7, 10}},
  };
  for (auto& pair : subscriptions) {
    for (size_t i = 0; i < pair.second.size(); ++i) {
      CATCH_REQUIRE(swbus_subscribe(swbus, rxqs[pair.first], pair.second[i]) ==
                    OK);
    } /* for(i..) */
  } /* for(&pair..) */

  std::vector<bool> checks;
  std::mutex        mtx;

  auto dist   = std::uniform_int_distribution<size_t>(0, config->max_pools - 1);
  auto pub_cb = [&]() {
    for (size_t i = 0; i < TH_RXQ_SIZE * 2 / n_threads; ++i) {
      status_t rval = swbus_publish(swbus,
                                    i % TH_MAX_PID,
                                    config->pools[dist(th::make_rng())].elt_size,
                                    buf);
      mtx.lock();
      checks.push_back(rval == OK);
      mtx.unlock();
    } /* for(i..) */
  };
  auto dist2  = std::uniform_int_distribution<size_t>(0, 1000);
  auto sub_cb = [&](size_t id) {
    size_t          count = 0;
    status_t        rval;
    struct timespec to = {.tv_sec = 0, .tv_nsec = (int64_t)dist2(th::make_rng())};
    auto&           subs = subscriptions[id];
    while (count < 100) {
      struct swbus_rxq_ent* ent = swbus_rxq_timedwait(swbus, rxqs[id], &to);
      if (nullptr == ent) {
        continue;
      }
      ++count;
      mtx.lock();
      checks.push_back(std::ranges::find(subs, ent->pid) != subs.end());

      mtx.unlock();

      rval = swbus_rxq_pop_front(rxqs[id], ent);
      mtx.lock();
      checks.push_back(rval == OK);
      mtx.unlock();
    }
  };

  CATCH_REQUIRE(std::ranges::all_of(checks, [&](bool val) { return val; }));

  std::vector<std::thread> consumers;
  std::vector<std::thread> publishers;
  consumers.reserve(n_consumers);
  publishers.reserve(n_publishers);
  for (size_t i = 0; i < n_consumers; ++i) {
    consumers.push_back(std::thread(sub_cb, i));
  } /* for(i..) */

  for (size_t i = 0; i < n_publishers; ++i) {
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
      CATCH_REQUIRE(std::ranges::find(subs, ent->pid) != subs.end());
      CATCH_REQUIRE(OK == swbus_rxq_pop_front(rxqs[i], ent));
    }
  } /* for(i..) */

  for (size_t i = 0; i < config->max_pools; ++i) {
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

CATCH_TEST_CASE(
  "swbus (async mode): fast publisher, slow subscriber, depth-1 "
  "RXQ completes",
  "[swbus]") {
  fast_publisher_slow_subscriber(RCSW_SWBUS_ASYNC);
}

CATCH_TEST_CASE(
  "swbus (sync mode): fast publisher, slow subscriber, depth-1 "
  "RXQ does not deadlock",
  "[swbus]") {
  fast_publisher_slow_subscriber(RCSW_NONE);
}

CATCH_TEST_CASE(
  "swbus_publish to a full RXQ returns ERROR instead of "
  "blocking",
  "[swbus]") {
  struct mpool_config pool;
  struct swbus*       bus = make_bus(&pool, RCSW_NONE);
  CATCH_REQUIRE(nullptr != bus);
  struct pcqueue* rxq = swbus_rxq_init(bus, nullptr, 1);
  CATCH_REQUIRE(nullptr != rxq);
  CATCH_REQUIRE(OK == swbus_subscribe(bus, rxq, 1));

  char pkt[16] = {0};
  CATCH_REQUIRE(OK == swbus_publish(bus, 1, sizeof(pkt), pkt));
  /* RXQ is now full: the publish fails immediately */
  errno = 0;
  CATCH_REQUIRE(ERROR == swbus_publish(bus, 1, sizeof(pkt), pkt));
  CATCH_REQUIRE(ENOSPC == errno);
  swbus_destroy(bus);
}

CATCH_TEST_CASE("swbus_init does not modify the caller's pool configs",
                "[swbus]") {
  struct mpool_config pool;
  struct swbus*       bus = make_bus(&pool, RCSW_NONE);
  CATCH_REQUIRE(nullptr != bus);
  CATCH_REQUIRE(RCSW_NONE == pool.flags);
  swbus_destroy(bus);
}

CATCH_TEST_CASE(
  "swbus with RCSW_NOALLOC_META keeps its metadata in the "
  "caller's buffer",
  "[swbus]") {
  if (RCSW_CONFIG_PTR_ALIGN < alignof(void*)) {
    CATCH_SKIP("RCSW_CONFIG_PTR_ALIGN is below pointer alignment on this host");
  }
  struct mpool_config pool;
  memset(&pool, 0, sizeof(pool));
  pool.elt_size = 64;
  pool.max_elts = 8;

  struct swbus_config c;
  memset(&c, 0, sizeof(c));
  c.pools     = &pool;
  c.max_pools = 1;
  c.max_rxqs  = 2;
  c.max_subs  = 4;
  c.flags     = RCSW_NOALLOC_META;
  strcpy(c.name, "meta");

  /* heap, exactly sized, so ASan catches any carving past the end */
  size_t   bytes = swbus_meta_space(c.max_pools, c.max_rxqs, c.max_subs);
  uint8_t* meta  = static_cast<uint8_t*>(malloc(bytes));
  c.meta         = reinterpret_cast<dptr_t*>(meta);

  struct swbus* bus = swbus_init(nullptr, &c);
  CATCH_REQUIRE(nullptr != bus);
  auto inside = [&](const void* p) {
    auto* b = static_cast<const uint8_t*>(p);
    return b >= meta && b < meta + bytes;
  };
  CATCH_REQUIRE(inside(bus->pools));
  CATCH_REQUIRE(inside(bus->rxqs));
  CATCH_REQUIRE(inside(bus->subscribers));

  struct pcqueue* q0 = swbus_rxq_init(bus, nullptr, 4);
  struct pcqueue* q1 = swbus_rxq_init(bus, nullptr, 4);
  CATCH_REQUIRE(nullptr != q0);
  CATCH_REQUIRE(nullptr != q1);
  for (uint32_t pid = 1; pid <= 2; ++pid) {
    CATCH_REQUIRE(OK == swbus_subscribe(bus, q0, pid));
    CATCH_REQUIRE(OK == swbus_subscribe(bus, q1, pid));
  }
  /* max_subs reached */
  CATCH_REQUIRE(ERROR == swbus_subscribe(bus, q0, 3));

  char pkt[16] = "hello";
  CATCH_REQUIRE(OK == swbus_publish(bus, 2, sizeof(pkt), pkt));
  struct swbus_rxq_ent* e = swbus_rxq_front(q1);
  CATCH_REQUIRE(nullptr != e);
  CATCH_REQUIRE(2 == e->pid);
  CATCH_REQUIRE(0 == strcmp(reinterpret_cast<char*>(e->data), "hello"));

  swbus_destroy(bus);
  free(meta);
}

CATCH_TEST_CASE(
  "swbus_publish_reserve records the size, and fails with "
  "ENOSPC when nothing fits",
  "[swbus]") {
  struct mpool_config pool;
  struct swbus*       bus = make_bus(&pool, RCSW_NONE);
  CATCH_REQUIRE(nullptr != bus);

  struct swbus_rsrvn res;
  memset(&res, 0, sizeof(res));
  CATCH_REQUIRE(OK == swbus_publish_reserve(bus, &res, 10));
  CATCH_REQUIRE(10 == res.pkt_size);
  CATCH_REQUIRE(nullptr != res.data);
  CATCH_REQUIRE(OK == mpool_release(res.bp, res.data));

  errno = 0;
  CATCH_REQUIRE(ERROR == swbus_publish_reserve(bus, &res, 65)); /* pool: 64 */
  CATCH_REQUIRE(ENOSPC == errno);
  swbus_destroy(bus);
}

CATCH_TEST_CASE("swbus_rxq_front does not wait on an empty RXQ", "[swbus]") {
  struct mpool_config pool;
  struct swbus*       bus = make_bus(&pool, RCSW_NONE);
  CATCH_REQUIRE(nullptr != bus);
  struct pcqueue* rxq = swbus_rxq_init(bus, nullptr, 4);
  CATCH_REQUIRE(nullptr != rxq);

  errno = 0;
  CATCH_REQUIRE(nullptr == swbus_rxq_front(rxq));
  CATCH_REQUIRE(EAGAIN == errno);
  swbus_destroy(bus);
}

CATCH_TEST_CASE("swbus_publish_release publishes the reservation's pkt_size",
                "[swbus]") {
  struct mpool_config pool;
  struct swbus*       bus = make_bus(&pool, RCSW_NONE);
  CATCH_REQUIRE(nullptr != bus);
  struct pcqueue* rxq = swbus_rxq_init(bus, nullptr, 4);
  CATCH_REQUIRE(nullptr != rxq);
  CATCH_REQUIRE(OK == swbus_subscribe(bus, rxq, 1));

  /* Reserve the most that could be needed, then publish less */
  struct swbus_rsrvn res;
  CATCH_REQUIRE(OK == swbus_publish_reserve(bus, &res, 32));
  memcpy(res.data, "abc", 3);
  res.pkt_size = 3;
  CATCH_REQUIRE(OK == swbus_publish_release(bus, 1, &res));

  struct swbus_rxq_ent* e = swbus_rxq_wait(bus, rxq);
  CATCH_REQUIRE(nullptr != e);
  CATCH_REQUIRE(3 == e->pkt_size);
  CATCH_REQUIRE(0 == memcmp(e->data, "abc", 3));
  CATCH_REQUIRE(OK == swbus_rxq_pop_front(rxq, e));
  CATCH_REQUIRE(mpool_isempty(&bus->pools[0])); /* chunk returned */
  swbus_destroy(bus);
}

CATCH_TEST_CASE("swbus_publish_release accepts an application-built "
                "reservation",
                "[swbus]") {
  struct mpool_config pool;
  struct swbus*       bus = make_bus(&pool, RCSW_NONE);
  CATCH_REQUIRE(nullptr != bus);
  struct pcqueue* q0 = swbus_rxq_init(bus, nullptr, 4);
  CATCH_REQUIRE(nullptr != q0);
  CATCH_REQUIRE(OK == swbus_subscribe(bus, q0, 1));

  /* The application owns the buffer: no pool, no reference counting */
  static dptr_t      buf[4];
  struct swbus_rsrvn res;
  res.data     = buf;
  res.pkt_size = sizeof(buf);
  res.bp       = nullptr;
  CATCH_REQUIRE(OK == swbus_publish_release(bus, 1, &res));

  struct swbus_rxq_ent* e = swbus_rxq_wait(bus, q0);
  CATCH_REQUIRE(nullptr != e);
  CATCH_REQUIRE(buf == e->data);
  CATCH_REQUIRE(sizeof(buf) == e->pkt_size);
  CATCH_REQUIRE(nullptr == e->bp);
  CATCH_REQUIRE(OK == swbus_rxq_pop_front(q0, e));
  CATCH_REQUIRE(mpool_isempty(&bus->pools[0])); /* the pool was not touched */
  swbus_destroy(bus);
}
