/**
 * \file
 *
 * \copyright 2017 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * \ingroup utils
 *
 * \brief Memory manipulation and dumping routines for use on embedded systems.
 *
 * The dump functions print through \ref DPRINTF(), i.e. the configured ER
 * plugin's print function, and print nothing when \ref RCSW_ERL is NONE.
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <stddef.h>
#include <stdint.h>

#include "rcsw/core/compilers.h"
#include "rcsw/core/core.h"

/*******************************************************************************
 * Public API
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Write a 32-bit register/memory location with a single 32-bit access.
 *
 * \param addr The address of register/memory location to write. Must be 32-bit
 *             aligned.
 * \param wval The value to write
 *
 * \return \ref status_t
 */
RCSW_API status_t utils_mem_write32(uintptr_t addr, uint32_t wval);

/**
 * \brief Read a 32-bit register/memory location
 *
 * \param addr The address of register/memory location to read from
 *
 * \return The value of the register, or 0xFFFFFFFF if \p addr is not 32-bit
 * aligned.
 */
RCSW_API uint32_t utils_mem_read32(uintptr_t addr);

/**
 * \brief Read, modify, write, and readback a memory value.
 *
 * Reading back the value written verifies the write was successful.
 *
 * \param addr The address of register/memory location to write.
 *
 * \param wval The 32-bit value to write.
 *
 * \param mask The 32-bit insert mask. Only bits that are 1 are taken from
 * \p wval; bits that are 0 keep their current value. Pass 0xFFFFFFFF to write
 * all 32 bits (the location is then not read first). Must not be 0. Every
 * written bit is verified on readback.
 *
 * \return \ref status_t
 */
RCSW_API status_t utils_mem_rmwr32(uintptr_t addr, uint32_t wval, uint32_t mask);

/**
 * \brief Copy memory from source to dest in 32 bit chunks
 *
 * If src, dest or n_bytes is not 32-bit aligned, no action is performed. The
 * source and destination must not overlap.
 *
 * \param dest The destination
 * \param src The source
 * \param n_bytes How many bytes to copy
 *
 * \return The destination
 */
RCSW_API void* utils_mem_cpy32(void* __restrict__ dest,
                               const void* __restrict__ src,
                               size_t n_bytes);

/**
 * \brief Dump 4 byte words in memory in hexadecimal (see the file description)
 *
 * \param buf Address for start of dump. If this is not 32-bit aligned, ERROR is
 * returned.
 * \param n_bytes How large of a dump to take, in bytes
 *
 * \return \ref status_t
 */
RCSW_API status_t utils_mem_dump32(const void* buf, size_t n_bytes);

/**
 * \brief Dump 2 byte words in memory in hexadecimal (see the file description)
 *
 * \param buf Address for start of dump. If this is not 16-bit aligned, ERROR is
 * returned.
 * \param n_bytes How large of a dump to take, in bytes
 *
 * \return \ref status_t
 */
RCSW_API status_t utils_mem_dump16(const void* buf, size_t n_bytes);

/**
 * \brief Dump 1 byte words in memory in hexadecimal (see the file description)
 *
 * \param buf Address for start of dump
 * \param n_bytes How large of a dump to take, in bytes
 *
 */
RCSW_API void utils_mem_dump8(const void* buf, size_t n_bytes);

/**
 * \brief Dump 4 byte words in memory in hexadecimal (see the file description), with offsets
 *
 * \param buf Address for start of dump. If this is not 32-bit aligned, ERROR is
 *            returned.
 *
 * \param n_bytes How large of a dump to take, in bytes
 *
 * \return \ref status_t
 */
RCSW_API status_t utils_mem_dump32v(const void* buf, size_t n_bytes);

/**
 * \brief Dump 2 byte words in memory in hexadecimal (see the file description), with offsets
 *
 * \param buf Address for start of dump. If this is not 16-bit aligned, ERROR is
 *            returned.
 *
 * \param n_bytes How large of a dump to take, in bytes
 *
 * \return \ref status_t
 */
RCSW_API status_t utils_mem_dump16v(const void* buf, size_t n_bytes);

/**
 * \brief Dump 1 byte words in memory in hexadecimal (see the file description), with offsets
 *
 * \param buf Address for start of dump
 * \param n_bytes How large of a dump to take, in bytes
 */
RCSW_API void utils_mem_dump8v(const void* buf, size_t n_bytes);

/**
 * \brief Reverse the byte order of each 16-bit word in a buffer, in place.
 *
 * \param buf Memory to swap. Must be 16-bit aligned.
 *
 * \param n_bytes # bytes to swap. Must be a multiple of 2.
 *
 * \return \ref status_t. ERROR (nothing swapped) if an alignment
 * requirement is not met.
 */
RCSW_API status_t utils_mem_bswap16(uint16_t* buf, size_t n_bytes);

/**
 * \brief Reverse the byte order of each 32-bit word in a buffer, in place
 * (0xAABBCCDD becomes 0xDDCCBBAA).
 *
 * \param buf Memory to swap. Must be 32-bit aligned.
 *
 * \param n_bytes # bytes to swap. Must be a multiple of 4.
 *
 * \return \ref status_t. ERROR (nothing swapped) if an alignment
 * requirement is not met.
 */
RCSW_API status_t utils_mem_bswap32(uint32_t* buf, size_t n_bytes);

END_C_DECLS
