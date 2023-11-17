/**
 * \file swbus_test.h
 * \brief SWBUS test harness definitions.
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/common/common.h"
#include "tests/test.h"

/*******************************************************************************
 * Constant Definitions
 ******************************************************************************/
#define MAX_PRODUCERS 2
#define MAX_CONSUMERS 20
#define MAX_THREADS (MAX_PRODUCERS+MAX_CONSUMERS)
#define MAX_ITERATIONS 10


#define RCSW_MIN_PKT_SIZE 8
#define MAX_PKT_SIZE 512

#define PRODUCER_START_DELAY  100000000 /* 10ms */

#define MAX_BUFFER_SIZE      512
#define RXQ_SIZE             32

/*******************************************************************************
 * Structure Definitions
 ******************************************************************************/
struct thread_data {
  bool_t kill; /* Kill the thread */
  pthread_t thread; /* Pointer to thread */
  int index; /* Current thread index */
  void (*func)(void* thr_p); /* Thread main() function */
};

struct parms { /* program parameters */
  uint32_t n_thr_prod; /* # of producer threads */
  uint32_t n_thr_cons; /* # of consumer threads */
  uint32_t pid_first;
  uint32_t pid_last;
  uint32_t delay_usecs;
};

struct cvt { /* current value table */
  uint32_t thr_cnt;  /* # of pthreads */
  struct thread_data thr_tbl[MAX_THREADS]; /* array of thread data */
  struct parms parms;       /* program parameters */
  struct swbus_inst *swbus;
};

/*******************************************************************************
 * Global Variables
 ******************************************************************************/
extern struct cvt cvt;

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
int thr_create(uint32_t thr_i);
status_t join_threads(void);
int thr_init(void);
void thr_consumer(struct thread_data * thr_p);
void thr_producer(struct thread_data * thr_p);
void thr_sig_int(void *thr_arg);

int create_threads(uint32_t n_cons,
                   uint32_t n_prods,
                   uint32_t pid_first,
                   uint32_t pid_last,
                   uint32_t delay_usecs);


