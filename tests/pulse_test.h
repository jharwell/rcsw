/**
 * \file pulse_test.h
 * \brief PULSE test harness definitions.
 *
 * \copyright 2017 John Harwell, All rights reserved.
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

#ifndef _PULSE_TEST_H_
#define _PULSE_TEST_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/common/common.h"

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
  struct pulse_inst *pulse;
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


#endif /*  _PULSE_TEST_H_  */
