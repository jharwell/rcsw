/**
 * \file mcm_opt.h
 * \ingroup algorithm
 * \brief Matrix Chain Multiplication Optimizer.
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

#ifndef INCLUDE_RCSW_ALGORITHM_MCM_OPT_H_
#define INCLUDE_RCSW_ALGORITHM_MCM_OPT_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/common/common.h"

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
 * Forward Declarations
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
status_t mcm_opt_init(struct mcm_optimizer * mcm, const size_t * matrices,
                      size_t size);

/**
 * \brief Destroy an initialized optimizer
 *
 * Any further use of the MCM after calling this function is undefined.
 *
 * \param mcm The MCM handle
 */
void mcm_opt_destroy(struct mcm_optimizer * mcm);

/**
 * \brief Compute the best way to multiply a sequence of N matrices via
 * iterative dynamic programming
 *
 * \param mcm The MCM handle
 *
 * \return \ref status_t
 */
status_t mcm_opt_optimize(struct mcm_optimizer * mcm);

/**
 * \brief Print the results of the chain order optimization to stdout
 *
 * \param mcm The MCM handle
 *
 * \return \ref status_t
 */
status_t mcm_opt_print(const struct mcm_optimizer * mcm);

/**
 * \brief Report the results of chain order optimization via filling an array of
 * integers with the indices of the matrix chain (0 for p[0], 1 for p[1], etc.)
 *
 * \param mcm The MCM handle
 * \param ordering Pointer to array of ints to be filled with indices of optimal
 * multiplication
 *
 * \return \ref status_t
 */
status_t mcm_opt_report(const struct mcm_optimizer * mcm,
                        size_t * ordering);

END_C_DECLS

#endif /* INCLUDE_RCSW_ALGORITHM_MCM_OPT_H_ */
