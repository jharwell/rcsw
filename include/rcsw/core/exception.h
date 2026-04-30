/**
 * \file
 *
 * \copyright 2009-2015 Francesco Nidito
 * \copyright 2022 John Harwell All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * \ingroup common
 *
 * \brief Exception handling in C.
 *
 * \warning It is undefined behaviour if longjmp unwinds past a VLA, a C99
 * variably-modified type, or (in C++ builds) any object with a non-trivial
 * destructor. Only use this if you actually need it, not because it's cool and
 * shiny.
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <setjmp.h>

/*******************************************************************************
 * Macros
 ******************************************************************************/
/**
 * \brief Start of a try-catch block (just like in OOP languages)
 *
 */
#define RCSW_TRY                \
  do {                          \
    jmp_buf ex_buf__;           \
    switch (setjmp(ex_buf__)) { \
      case 0:                   \
        while (1) {
/**
 * \brief Catch an exception. In C, this is a number which can be anything your
 * platform supports.
 */
#define RCSW_CATCH(x) \
  break;              \
  case x:

/**
 * \brief A finally block, just like in OOP languages
 *
 */
#define RCSW_FINALLY \
  break;             \
  }                  \
  default:

/**
 * \brief End of a try-catch block (just like in OOP languages)
 */
#define RCSW_ETRY \
  break;          \
  }               \
  }               \
  }               \
  while (0)

/**
 * \brief Throw an exception, which in C is just an integer
 */
#define RCSW_THROW(x) longjmp(ex_buf__, x)
