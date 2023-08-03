/**
 * \file mcm_opt.h
 * \ingroup algorithm
 * \brief Matrix Chain Multiplication Optimizer.
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
 * Structure Definitions
 ******************************************************************************/
/**
 * \brief Matrix chain multiplication optimizer.
 */
struct mcm_optimizer {
    size_t min_mults;  /// Minimum # of scalar multiplications need for the
                       /// chain.
    /**
     * Input matrices, specified as an array of the outer dimensions.
     */
    const size_t* matrices;
    size_t size;       /// # of input matrices.
    size_t *results;   /// N x N  array of optimil multiplication costs.
    size_t *route;     /// N x N  array storing the optimal results route.
    /**
     * Optimal multiplicative ordering, specified as 0-indexed matrix IDs.
     */
    size_t *ordering;
};

BEGIN_C_DECLS

/*******************************************************************************
 * API Functions
 ******************************************************************************/
/**
 * \brief Initialize a Matrix Chain Multiplication (MCM) optimizer
 *
 * \param mcm The MCM to initialize
 * \param size # of input matrices -1.
 * \param matrices The matrix chain, but 0 indexed instead of 1 indexed.
 *
 * \return \ref status_t
 */
RCSW_API status_t mcm_opt_init(struct mcm_optimizer * mcm,
                               const size_t * matrices,
                               size_t size);

/**
 * \brief Destroy an initialized optimizer
 *
 * Any further use of the MCM after calling this function is undefined.
 *
 * \param mcm The MCM handle
 */
RCSW_API void mcm_opt_destroy(struct mcm_optimizer * mcm);

/**
 * \brief Compute the best way to multiply a sequence of N matrices via
 * iterative dynamic programming.
 *
 * \param mcm The MCM handle
 *
 * \return \ref status_t
 */
RCSW_API status_t mcm_opt_optimize(struct mcm_optimizer * mcm);

/**
 * \brief Print the results of the chain order optimization to stdout
 *
 * \param mcm The MCM handle
 *
 * \return \ref status_t
 */
RCSW_API status_t mcm_opt_print(const struct mcm_optimizer * mcm);

/**
 * \brief Report the results of chain order optimization via filling an array of
 * integers with the indices of the matrix chain (0 for p[0], 1 for p[1], etc.).
 *
 * \param mcm The MCM handle
 *
 * \param ordering Array to be filled with indices of optimal multiplication
 *
 * \return \ref status_t
 */
RCSW_API status_t mcm_opt_report(const struct mcm_optimizer * mcm,
                                 size_t * ordering);

END_C_DECLS
