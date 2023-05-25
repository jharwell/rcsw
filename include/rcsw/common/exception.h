/**
 * \file exception.h
 *
 * \copyright 2009-2015 Francesco Nidito
 * \copyright 2022 John Harwell All rights reserved.
 *
 * \ingroup common
 *
 * \brief Exception handling in C (woot woot!)
 *
 * SPDX-License-Identifier: MIT
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
#define TRY                                     \
    do {                                        \
        jmp_buf ex_buf__;                       \
        switch (setjmp(ex_buf__)) {             \
        case 0:                                 \
            while (1) {
/**
 * \brief Catch an exception. In C, this is a number which can be anything your
 * platform supports.
 */
#define CATCH(x)                                \
    break;                                      \
case x:

/**
 * \brief A finally block, just like in OOP languages
 *
 */
#define FINALLY                                 \
    break;                                      \
    }                                           \
default:
#define ETRY                                    \
    break;                                      \
    }                                           \
        }                                       \
    } while (0)

/**
 * \brief Throw an exception, which in C is just an integer
 */
#define THROW(x) longjmp(ex_buf__, x)

