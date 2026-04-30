/**
 * \file
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * \ingroup utils
 *
 * \brief Memory manipulation and dumping routines for use on emedded systems.
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
 *
 * Some of these functions are inlined, rather than prototyped, so that the
 * compiler can still optimize them as if it was just a direct memory assignment
 * without checks.
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Write a memory location
 *
 * \param addr The address of register/memory location to write
 * \param wval The value to write
 *
 * Use size_t for the addr so that it compiles correctly for both Linux and
 * embedded systems.
 *
 * \return \ref status_t
 */
RCSW_API status_t utils_mem_write32(size_t addr, uint32_t wval);

/**
 * \brief Read a 32-bit register/memory location
 *
 * \param addr The address of register/memory location to read from
 *
 * Uses size_t for the addr so that it compiles correctly for both Linux and
 * embedded systems.
 *
 * \return: The value of the register, or 0xFFFFFFFF if non word-aligned.
 */
RCSW_API uint32_t utils_mem_read32(size_t addr);

/**
 * \brief Read, modify, write, and readback a memory value.
 *
 * Reading back the value written verifies the write was successful.
 *
 * \param addr The address of register/memory location to write.
 *
 * \param wval The 32-bit value to write.
 *
 * \param mask The 32-bit insert mask. Bits that are 0 will not be altered,
 * regardless of the value written.
 *
 * \return \ref status_t
 */
RCSW_API static inline status_t utils_mem_rmwr32(uint32_t addr,
                                                 uint32_t wval,
                                                 uint32_t mask);

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
 * \brief Dump 4 byte words in memory to stdout in hexadecimal
 *
 * \param buf Address for start of dump. If this is not 32-bit aligned, ERROR is
 * returned.
 * \param n_bytes How large of a dump to take, in bytes
 *
 * \return \ref status_t
 */
RCSW_API status_t utils_mem_dump32(const void* buf, size_t n_bytes);

/**
 * \brief Dump 2 byte words in memory to stdout in hexadecimal
 *
 * \param buf Address for start of dump. If this is not 16-bit aligned, ERROR is
 * returned.
 * \param n_bytes How large of a dump to take, in bytes
 *
 * \return \ref status_t
 */
RCSW_API status_t utils_mem_dump16(const void* buf, size_t n_bytes);

/**
 * \brief Dump 1 byte words in memory to stdout in hexadecimal
 *
 * \param buf Address for start of dump
 * \param n_bytes How large of a dump to take, in bytes
 *
 */
RCSW_API void utils_mem_dump8(const void* buf, size_t n_bytes);

/**
 * \brief Dump 4 byte words in memory to stdout in hexadecimal, with offsets
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
 * \brief Dump 2 byte words in memory to stdout in hexadecimal, with offsets
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
 * \brief Dump 1 byte words in memory to stdout in hexadecimal, with offsets
 *
 * \param buf Address for start of dump
 * \param n_bytes How large of a dump to take, in bytes
 */
RCSW_API void utils_mem_dump8v(const void* buf, size_t n_bytes);

/**
 * \brief Byte swap memory in 1 byte chunks (upper 8/lower 8) in place.
 *
 * \param buf Memory to swap. Must be 16-bit aligned or no action is
 *            performed.
 *
 * \param n_bytes # bytes to swap. Must be 16-bit aligned or no action is
 *                performed.
 *
 * \return \ref status_t
 */
RCSW_API status_t utils_mem_bswap16(uint16_t* buf, size_t n_bytes);

/**
 * \brief Byte swap memory in 2 byte chunks (upper 16/lower 16) in place.
 *
 * \param buf Memory to swap. Must be 32-bit aligned or no action is
 *            performed.
 *
 * \param n_bytes # bytes to swap. Must be 32-bit aligned or no action is
 *                performed.
 *
 * \return \ref status_t
 */
RCSW_API status_t utils_mem_bswap32(uint32_t* buf, size_t n_bytes);

END_C_DECLS
