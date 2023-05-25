/**
 * \file test_frmwk.h
 * \ingroup common
 * \brief Simple testing framework.
 *
 * This should really only be used if you do not have access to a C++ test
 * framework (i.e. on an embedded platform). It's something I hacked together in
 * a few hours that violates a whole bunch of good programming practices. It
 * isn't that elegant either. But it WORKS, and can be a time saver.
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/rcsw.h"

/*******************************************************************************
 * Constant Definitions
 ******************************************************************************/
/**
 * \brief Function pointer type tags used by the application to determine #
 * params, type when using variable argument function pointers.
 */
enum test_type { TEST_FP0, TEST_FP1, TEST_FP2, TEST_FP3, TEST_FP4, TEST_FP5 };

/*******************************************************************************
 * Structure Definitions
 ******************************************************************************/
/**
 * \brief Representation of a test within the framework.
 *
 * Each test has a function pointer that points to the test function, setup
 * callback for test setup, teardown pointer for post-test cleanup.
 *
 * Note that there are NO arguments specified for the callbacks. This is allowed
 * by gcc, though it will give a warning (hence the pragmas). This was necessary
 * so that I could have different function signatures without having to have an
 * endless array of function pointers. The down side to doing things this way is
 * that you need zero parameter checking help from the compiler.
 */
RCSW_WARNING_DISABLE_PUSH()
RCSW_WARNING_DISABLE_STRICT_PROTO()
struct test_inst {
    char name[32];
    enum test_type tag;
    status_t status;

    status_t (*fp)();
    status_t (*setup)(status_t (*)());
    status_t (*teardown)(status_t (*)());
};
RCSW_WARNING_DISABLE_POP()

/**
 * \brief The test framework, which contains all registered tests, as well as a
 * cumulative pass/fail value.
 */
struct test_frmwk {
    struct test_inst *test_insts;  /// The test instances (each test has one).
    size_t n_tests;                ///  Number of tests in the framework.
    /**
     * Cumulative status of all tests; logical OR of all test_inst statuses
     **/
    status_t status;
};

/*******************************************************************************
 * Macros
 ******************************************************************************/
/**
 * \brief A checkpoint/condition check
 *
 * Like the RCSW_CHECK() macro, but also prints if something was OK. Very useful in
 * unit tests so you can see the progress of the test, instead of it just
 * sitting there. Should really only be used in unit tests.
 *
 * \param cond The condition to check
 */
#define RCSW_CHECKPOINT(cond)                                                \
    if (!(cond)) {                                                      \
        PRINTF(                                                         \
            __FILE__ ":%s:%d RCSW_CHECKPOINT %sFAILED%s: " STR(cond) "\n", __func__, \
            __LINE__, DBG_FAILC, DBG_ENDC);                             \
        goto error;                                                     \
    } else {                                                            \
        PRINTF(                                                         \
            __FILE__ ":%s:%d RCSW_CHECKPOINT %sOK%s: " STR(cond) "\n", __func__, \
            __LINE__, DBG_OKC, DBG_ENDC);                               \
    }
/**
 * The pragma is needed to all the test framework to compile cleanly. Having
 * function pointers with () is not a prototype definition, strictly speaking,
 * so the compiler will complain. This module provides test code, so I think
 * doing this is a-OK.
 *
 * Comment it out if it makes you uncomfortable.
 */
RCSW_WARNING_DISABLE_PUSH()
RCSW_WARNING_DISABLE_STRICT_PROTO()

/**
 * \brief Initialize a test
 *
 * \param tests A pointer to a test_frmwk structure.
 * \param index The index for the test. Tests are run in the order specified by their
 * indices. Indices can be overwritten without issue.
 *
 * \param fp_in The function pointer that corresponds to the main function for
 * the test. This is a variable argument function pointer.
 *
 * \param tag_in This is a field which the application can choose to use or
 * ignore. It's intent to to provide a way to define how many/what type of
 * function arguments to different test functions.
 *
 * \param name_in A string representing the name of the test. This name is
 * printed when the test is finished along with the pass/fail status.
 */

#define TEST_INIT(tests, index, fp_in, tag_in, name_in)         \
    do {                                                        \
        (tests)->test_insts[(index)].tag      = (tag_in);                 \
        (tests)->test_insts[(index)].fp       = (status_t (*)())(fp_in); \
        (tests)->test_insts[(index)].status   = TESTFAIL;           \
        (tests)->test_insts[(index)].setup    = NULL;               \
        (tests)->test_insts[(index)].teardown = NULL;               \
        strcpy((tests)->test_insts[(index)].name, (name_in));       \
    } while (0);
RCSW_WARNING_DISABLE_POP()

/**
 * \brief Define a setup function for a test
 *
 * The provided callback must return OK if the setup was successful, nonzero
 * otherwise.  The callback is passed the function pointer for the current test
 * so that different setup can be performed for different tests if desired.
 *
 * \param tests The test framework pointer
 * \param index Index of the test
 * \param fp_in Pointer to function to perform the setup
 */
#define TEST_REQ_SETUP(tests, index, fp_in, ...)        \
    tests->test_insts[(index)].setup = fp_in;

/**
 * \brief Define a teardown function for a test
 *
 * The provided callback must return OK if the teardown was successful, nonzero
 * otherwise.  The callback is passed the function pointer for the current test
 * so that different teardown can be performed for different tests if desired.
 *
 * \param tests The test framework pointer
 * \param index Index of the test
 * \param fp_in Pointer to function to perform the teardown
 */
#define TEST_REQ_TEARDOWN(tests, index, fp_in)  \
    tests->test_insts[(index)].teardown = fp_in;

/**
 * \brief Called automatically as part of TEST_RUN()--don't call directly
 */
#define TEST_SETUP(tests, index)                                        \
  if ((tests)->test_insts[(index)].setup) {                             \
    if ((tests)->test_insts[(index)].setup((tests)->test_insts[(index)].fp) != OK) { \
            goto test_finish;                                           \
        }                                                               \
    }

/**
 * \brief Called automatically as part of TEST_RUN() (don't call directly)
 */
#define TEST_TEARDOWN(tests, index)                                     \
    if ((tests)->test_insts[(index)].teardown) {                            \
        if ((tests)->test_insts[(index)].teardown((tests)->test_insts[(index)].fp) != OK) { \
            goto test_finish;                                           \
        }                                                               \
    }

/**
 * \brief Run a test, including setup/teardown
 *
 * \param tests The test framework handle
 * \param index Index of test to run
 * ... Any arguments to pass to the test function
 */
#define TEST_RUN(tests, index, ...)                                     \
    do {                                                                \
        TEST_SETUP(tests, index);                                       \
        (tests)->test_insts[i].status = (tests)->test_insts[i].fp(__VA_ARGS__); \
        if (TEST_STATUS(tests, index) == TESTFAIL) {                    \
            goto test_finish;                                           \
        }                                                               \
        TEST_TEARDOWN(tests, index);                                    \
    } while (0);

/**
 * \brief Get the pass/fail status of a test
 */
#define TEST_STATUS(tests, index) (tests)->test_insts[i].status

/**
 * \brief Get the test tag
 */
#define TEST_TAG(tests, index) (tests)->test_insts[i].tag

/**
 * \brief A colorized test header
 */
#define TEST_HDR(str, ...)                                      \
    DPRINTF("%s" str "%s", DBG_HEADC, ##__VA_ARGS__, DBG_ENDC)

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Initialize the test framework
 *
 * \param n_inst How many tests this framework will handle
 *
 * \return The initialized framework or NULL if an ERROR occurred
 */
struct test_frmwk *test_frmwk_init(size_t n_inst);

/**
 * \brief Shutdown the test framework
 *
 * \param tests The framework handle
 */
void test_frmwk_shutdown(struct test_frmwk *tests);

/**
 * \brief Print the status of the selected test
 *
 * \param tests The framework handle
 * \param index The index of the test to print
 */
void test_frmwk_test_status(struct test_frmwk *tests, int index);

/**
 * \brief Get the pass/fail summary of last round of tests
 *
 * \param tests The framework handle
 */
void test_frmwk_summary(struct test_frmwk *tests);

END_C_DECLS

