/**
 * \file mt_cvm.c
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
#include "rcsw/multithread/mt_cvm.h"
#include "rcsw/common/dbg.h"
#include "rcsw/common/fpc.h"
#include "rcsw/utils/time_utils.h"

/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS

mt_cvm_t* cvm_init(mt_cvm_t* const cvm_in, uint32_t flags) {
  mt_cvm_t* cvm = NULL;
  if (flags & MT_APP_DOMAIN_MEM) {
    cvm = cvm_in;
  } else {
    cvm = calloc(1, sizeof(mt_cvm_t));
  }
  RCSW_CHECK_PTR(cvm);
  cvm->flags = flags;

  RCSW_CHECK(OK == mt_cond_init(&cvm->cv, cvm->flags));
  RCSW_CHECK(OK == mt_mutex_init(&cvm->mutex, cvm->flags));
  return cvm;

error:
  mt_cvm_destroy(cvm);
  return NULL;
} /* mt_cvm_init() */

void mt_cvm_destroy(mt_cvm_t* const cvm) {
  RCSW_FPC_V(NULL != cvm);

  mt_cond_destroy(&cvm->cv);
  mt_mutex_destroy(&cvm->mutex);
  if (cvm->flags & MT_APP_DOMAIN_MEM) {
    free(cvm);
  }
} /* mt_cvm_destroy() */
status_t cvm_signal(mt_cvm_t* const cvm) {
  RCSW_FPC_NV(ERROR, NULL != cvm);
  RCSW_CHECK(0 == mt_cond_signal(&cvm->cv));
  return OK;

error:
  return ERROR;
} /* mt_cvm_signal() */

status_t cvm_wait(mt_cvm_t* const cvm) {
  RCSW_FPC_NV(ERROR, NULL != cvm);
  RCSW_CHECK(0 == mt_cond_wait(&cvm->cv, &cvm->mutex));
  return OK;

error:
  return ERROR;
} /* mt_cvm_wait() */

status_t cvm_timedwait(mt_cvm_t* const cvm, const struct timespec* const to) {
  RCSW_FPC_NV(ERROR, NULL != cvm, NULL != to);
  struct timespec ts = {.tv_sec = 0, .tv_nsec = 0};

  RCSW_CHECK(OK == time_ts_ref_conv(to, &ts));
  RCSW_CHECK(0 == mt_cond_timedwait(&cvm->cv, &cvm->mutex, &ts));
  return OK;

error:
  return ERROR;
} /* mt_cvm_timedwait() */

status_t cvm_broadcast(mt_cvm_t* const cvm) {
  RCSW_FPC_NV(ERROR, NULL != cvm);
  RCSW_CHECK(0 == mt_cond_broadcast(&cvm->cv));
  return OK;

error:
  return ERROR;
} /* mt_cvm_broadcast() */

END_C_DECLS
