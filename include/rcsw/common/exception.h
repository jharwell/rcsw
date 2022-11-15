/**
 * \file exception.h
 * \ingroup common
 * \brief Exception handling in C (woot woot!)
 *
 * Taken from online source and modified (original copyright below).
 */

#ifndef INCLUDE_RCSW_COMMON_EXCEPTION_H_
#define INCLUDE_RCSW_COMMON_EXCEPTION_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <setjmp.h>

/*******************************************************************************
 * Macros
 ******************************************************************************/
/**
 * Copyright (C) 2009-2015 Francesco Nidito
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without __restrict__ion, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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

#endif /* INCLUDE_RCSW_COMMON_EXCEPTION_H_ */
