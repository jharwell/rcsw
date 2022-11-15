/**
 * \file mt.h
 * \ingroup multithread
 * \brief Common multithread definitions.
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

#ifndef INCLUDE_RCSW_MULTITHREAD_MT_H_
#define INCLUDE_RCSW_MULTITHREAD_MT_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/

/*******************************************************************************
 * Constant Definitions
 ******************************************************************************/
/**
 * Passing this flag will cause  multithreaded locks/semaphores, etc. to
 * malloc() for their memory, as opposed to requiring the application to provide
 * it.
 */
#define MT_APP_DOMAIN_MEM 0x1

#endif /* INCLUDE_RCSW_MULTITHREAD_MT_H_ */
