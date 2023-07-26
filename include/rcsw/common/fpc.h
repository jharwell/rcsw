/**
 * \file fpc.h
 * \ingroup common
 * \brief Function precondition/post-condition definitions (very useful!).
 *
 * Allows you to define a set of conditions that must be met for a function to
 * proceed (preconditions) or that must be true when it returns (post
 * conditions). If a condition is not met:
 *
 * - Return a specified return value if \ref RCSW_FPC=\ref RCSW_FPC_RETURN
 * - Abort if \ref RCSW_FPC=\ref RCSW_FPC_ABORT
 * - Ignore if \ref RCSW_FPC=\ref RCSW_FPC_NONE
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
#include "rcsw/rcsw.h"

/*******************************************************************************
 * Constant Definitions
 ******************************************************************************/
/* \cond INTERNAL */
#define LIBRA_FPC_NONE 0
#define LIBRA_FPC_ABORT 1
#define LIBRA_FPC_RETURN 2

#if defined(LIBRA_FPC_INHERIT) && !defined(LIBRA_FPC)
#error LIBRA_FPC_INHERIT defined but LIBRA_FPC not defined!
#endif

/*
 * If rcsw is used in a context where this is not defined it is almost
 * assuredly an error, buuuttttt RCSW might be needed to compile in weird
 * environments.
 */
#if !defined(LIBRA_FPC)
#define LIBRA_FPC LIBRA_FPC_RETURN
#endif
/* \endcond */

/**
 * \brief The configured FPC definition.
 */
#define RCSW_FPC LIBRA_FPC

/**
 * \brief Indicate that failure of FPC should cause a return of an error value.
 */
#define RCSW_FPC_RETURN LIBRA_FPC_RETURN

/**
 * \brief Indicate that failure of FPC should cause program abort.
 */
#define RCSW_FPC_ABORT LIBRA_FPC_ABORT

/**
 * \brief Indicate that failure of FPC should be ignored.
 */
#define RCSW_FPC_NONE LIBRA_FPC_NONE

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
 * value of \ref RCSW_FPC.
 */
#define RCSW_FPC_RET_NV(X, v)                                        \
  {                                                                  \
    if (!RCSW_UNLIKELY(X)) {                                         \
      errno = EINVAL;                                                \
      return v;                                                      \
    }                                                                \
  }

/**
 * \def RCSW_FPC_RET_V(X)
 *
 * Check a single function pre/post condition \a X, returning if the condition
 * is not met. Requires that the function returns void.
 *
 * This macro can be used to unconditionally return if a precondition fails,
 * rather than the behavior of \ref RCSW_FPC_NV, which is dependent on the
 * value of \ref RCSW_FPC.
 */
#define RCSW_FPC_RET_V(X)                       \
  {                                             \
    if (!RCSW_UNLIKELY(X)) {                    \
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
 * rather than the behavior of \ref RCSW_FPC_NV, which is dependent on the
 * value of \ref RCSW_FPC.
 */
#define RCSW_FPC_ABORT_NV(X, v) RCSW_FPC_ASSERT(X)

/**
 * \def RCSW_FPC_ABORT_V(X)
 *
 * Check a single function pre/post condition \a X, aborting if the condition
 * is not met.
 *
 * This macro can be used to unconditionally abort if a precondition fails,
 * rather than the behavior of \ref RCSW_FPC_V, which is dependent on the
 * value of \ref RCSW_FPC.
 */
#define RCSW_FPC_ABORT_V(X) RCSW_FPC_ASSERT(X)

#if(RCSW_FPC == RCSW_FPC_RETURN)

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
#define RCSW_FPC_NV(v, ...)                             \
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

#elif(RCSW_FPC == RCSW_FPC_ABORT)

#define RCSW_FPC_NV(v, ...)                                     \
  { RCSW_XFOR_EACH2(RCSW_FPC_ABORT_NV, v, __VA_ARGS__); }

#define RCSW_FPC_V(...)                                 \
    { RCSW_XFOR_EACH1(RCSW_FPC_ABORT_V, __VA_ARGS__); }

#else

#define RCSW_FPC_NV(v, ...)
#define RCSW_FPC_V(...)

#endif /* RCSW_FPC */
