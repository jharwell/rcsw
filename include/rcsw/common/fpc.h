/**
 * \file fpc.h
 * \ingroup common
 * \brief Function precondition/post-condition definitions (very useful!).
 *
 * Allows you to define a set of conditions that must be met for a function to
 * proceed/must be true when it returns. If any of the conditions fails, the
 * function will return the specified return value if RCSW_FPC_RETURN=\ref
 * RCSW_FPC_RETURN is passed at compile time. Otherwise, RCSW_FPC_RETURN=\ref
 * RCSW_FPC_ABORT is assumed, and the failure of any condition will cause the
 * program to halt.
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <assert.h>
#include "rcsw/common/common.h"

/*******************************************************************************
 * Constant Definitions
 ******************************************************************************/
/**
 * \brief Readability constants for determining whether or not the program
 * should halt if a function pre/post condition fails.
 */
#define RCSW_FPC_ABORT 0

/**
 * \brief Readability constants for determining whether or not the program
 * should return an error code of some kind when a function pre/post condition
 * fails.
 */
#define RCSW_FPC_RETURN 1

/*******************************************************************************
 * Function Precondition Checking Macros
 ******************************************************************************/
/**
 * \def RCSW_FPC_RET_NV(X, v)
 *
 * Check a single function pre/post condition \a X, returning a value \a v if
 * the condition is not met. Requires that the function does not return
 * void. The value to return must be convertible to the return type of the
 * function.
 *
 * This macro can be used to unconditionally return if a precondition fails,
 * rather than the behavior of \ref RCSW_FPC_NV, which is dependent on the
 * value of \ref RCSW_FPC_TYPE.
 */
#define RCSW_FPC_RET_NV(X, v)                                        \
  {                                                                     \
      if (!(X)) {                                                       \
        errno = EINVAL;                                                 \
        return v;                                                       \
      }                                                                 \
  }

/**
 * \def RCSW_FPC_RET_V(X)
 *
 * Check a single function pre/post condition \a X, returning if the condition
 * is not met. Requires that the function returns void.
 *
 * This macro can be used to unconditionally return if a precondition fails,
 * rather than the behavior of \ref RCSW_FPC_NV_NV_V, which is dependent on the
 * value of \ref RCSW_FPC_TYPE.
 */
#define RCSW_FPC_RET_V(X)                     \
  {                                             \
    if (!(X)) {                                 \
      errno = EINVAL;                           \
      return;                                   \
    }                                           \
  }

/**
 * \def RCSW_FPC_ASSERT(X)
 *
 * Check a single function pre/post condition, halting the program if the
 * condition \a X fails.
 */
#define RCSW_FPC_ASSERT(X) { assert(X); }

/**
 * \def RCSW_FPC_ABORT_NV(X)
 *
 * Check a single function pre/post condition \a X, aborting if the condition
 * is not met.
 *
 * This macro can be used to unconditionally abort if a precondition fails,
 * rather than the behavior of \ref RCSW_FPC_NV_NV_NV, which is dependent on the
 * value of \ref RCSW_FPC_TYPE.
 */
#define RCSW_FPC_ABORT_NV(X, v) RCSW_FPC_ASSERT(X)

/**
 * \def RCSW_FPC_NV_ABORT_V(X)
 *
 * Check a single function pre/post condition \a X, aborting if the condition
 * is not met.
 *
 * This macro can be used to unconditionally abort if a precondition fails,
 * rather than the behavior of \ref RCSW_FPC_NV_NV_V, which is dependent on the
 * value of \ref RCSW_FPC_TYPE.
 */
#define RCSW_FPC_ABORT_V(X) RCSW_FPC_ASSERT(X)

#if(RCSW_FPC_TYPE == RCSW_FPC_RETURN)

/**
 * \def RCSW_FPC_NV(v, ...)
 *
 * Function pre/post condition macro.
 *
 * It performs short circuit evaluation, in that if the check for precondition
 * i-1 fails, conditions [i,n] are not checked, and the function returns/program
 * exits (depending on configuration).
 *
 * \a v is returned if parameter validation fails. Do not use this macro in
 * functions that return void.
 *
 * \a ... is a list of conditions that need to be met before the function can
 * execute/must be true when the function returns.
 */
#define RCSW_FPC_NV(v, ...)                               \
    { RCSW_XFOR_EACH2(RCSW_FPC_RET_NV, v, __VA_ARGS__); }

/**
 * \def RCSW_FPC_V(v, ...)
 *
 * Function pre/post condition macro for functions that return void.
 *
 * It performs short circuit evaluation, in that if the check for precondition
 * i-1 fails, conditions [i,n] are not checked, and the function returns/program
 * exits (depending on configuration).
 *
 * \a ... is a list of conditions that need to be met before the function can
 * execute/must be true when the function returns.
 */
#define RCSW_FPC_V(...)                                 \
  { RCSW_XFOR_EACH1(RCSW_FPC_RET_V, __VA_ARGS__); }

#elif(RCSW_FPC_TYPE == RCSW_FPC_ABORT)

#define RCSW_FPC_NV(v, ...)                       \
    { RCSW_XFOR_EACH2(RCSW_FPC_ABORT_NV, v, __VA_ARGS__); }
#define RCSW_FPC_V(...)                               \
  { RCSW_XFOR_EACH1(RCSW_FPC_ABORT_V, __VA_ARGS__); }
#else
#define RCSW_FPC_NV(v, ...)
#define RCSW_FPC_V(...)
#endif /* RCSW_FPC_TYPE */

