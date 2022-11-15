/**
 * \file test_frmwk.c
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

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/common/test_frmwk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rcsw/common/dbg.h"

/*******************************************************************************
 * Functions
 ******************************************************************************/
BEGIN_C_DECLS
struct test_frmwk* test_frmwk_init(size_t n_inst) {
  struct test_frmwk* tests = malloc(sizeof(struct test_frmwk));
  RCSW_CHECK(tests != NULL);

  tests->test_insts = malloc(sizeof(struct test_inst) * n_inst);
  RCSW_CHECK(tests->test_insts != NULL);

  tests->n_tests = n_inst;
  tests->status = OK;
  return tests;

error:
  test_frmwk_shutdown(tests);
  return NULL;
} /* test_frmwk_init */

void test_frmwk_shutdown(struct test_frmwk* tests) {
  if (tests) {
    if (tests->test_insts) {
      free(tests->test_insts);
    }
    free(tests);
  }
} /* test_frmwk_shutdown() */

void test_frmwk_test_status(struct test_frmwk* const tests, /* test array */
                            int index) /* index of test to show status for */
{
  PRINTF("%-32.32s ", tests->test_insts[index].name);
  if (tests->test_insts[index].status == OK) {
    PRINTF("%sPASSED%s\n", DBG_OKC, DBG_ENDC);
  } else {
    PRINTF("%sFAILED%s\n", DBG_FAILC, DBG_ENDC);
    tests->status = ERROR;
  }
} /* test_frmwk_test_status() */

void test_frmwk_summary(struct test_frmwk* const tests) /* test array */
{
  size_t passcount = 0;
  for (size_t i = 0; i < tests->n_tests; ++i) {
    if (tests->test_insts[i].status == OK) {
      passcount++;
    }
  }
  PRINTF("%sPassed %zu/%zu tests.%s\n\n",
         DBG_HEADC,
         passcount,
         tests->n_tests,
         DBG_ENDC);
  if (passcount < tests->n_tests) {
    PRINTF("%sTEST SUITE FAILED%s\n", DBG_FAILC, DBG_ENDC);
    tests->status = ERROR;
  } else {
    PRINTF("%sTEST SUITE PASSED%s\n", DBG_OKC, DBG_ENDC);
  }
} /* test_frmwk_summary() */

END_C_DECLS
