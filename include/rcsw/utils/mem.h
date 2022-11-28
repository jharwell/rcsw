/**
 * \file mem.h
 * \ingroup utils
 * \brief Memory manipulation and dumping routines for use on emedded systems.
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef INCLUDE_RCSW_UTILS_MEM_H_
#define INCLUDE_RCSW_UTILS_MEM_H_

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/common/common.h"
#include "rcsw/common/fpc.h"
#include "rcsw/utils/utils.h"

/*******************************************************************************
 * Inline Functions
 *
 * This functions are inlined, rather than prototyped, so that the compiler can
 * still optimize them as if it was just a direct memory assignment without
 * checks.
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Write a memory location
 *
 * \param addr The address of register/memory location to write
 * \param wval The 32-bit value to write
 *
 * \return \ref status_t
 */
static inline status_t mem_write(size_t addr, size_t wval) {
    RCSW_FPC_NV(ERROR, RCSW_IS_MEM_ALIGNED(addr, sizeof(uint32_t)));
    *((volatile uintptr_t *)addr) = wval;
    return OK;
} /* mem_write() */

/**
 * \brief Read a 32-bit register/memory location
 *
 * \param addr The address of register/memory location to read from
 *
 * \return: The value of the register, or 0xFFFFFFFF if non word-aligned
 */
static inline uint32_t mem_read(size_t addr) {
    RCSW_FPC_NV(0, (RCSW_IS_MEM_ALIGNED(addr, sizeof(uint32_t))));
    return *((volatile uint32_t*)addr);
} /* mem_read() */

/**
 * \brief Read, modify, write, and readback a memory value
 *
 * Reading back the value written verifies the write was successful.
 *
 * \param addr The address of register/memory location to write
 * \param wval The 32-bit value to write
 * \param mask The 32-bit insert mask. Bits that are 0 will not be altered,
 * regardless of the value written
 *
 * \return \ref status_t
 */
static inline status_t mem_rmwr(uint32_t addr, uint32_t wval, uint32_t mask) {
    RCSW_FPC_NV(ERROR, RCSW_IS_MEM_ALIGNED(addr, sizeof(uint32_t)));

    volatile uint32_t curr_val = 0;

    /*
     * If some bits masked, read the memory address, mask off all bits
     * NOT in mask in the result. OR this value with all bits in wval that
     * ARE in the mask.
     */
    if (mask != 0) {
        curr_val = mem_read(addr);
        wval     = (curr_val & ~mask) | (wval & mask);
    }

    /*
     * Write masked off bits back to memory/register, or just write wval to
     * memory, if mask was 0.
     */
    mem_write(addr, wval);

    /* read & compare */
    curr_val = mem_read(addr);
    RCSW_CHECK((curr_val & mask) == (wval & mask));

    return OK;

error:
    return ERROR;
} /* mem_rmwr() */

/*******************************************************************************
 * Function Protoypes
 ******************************************************************************/
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
void *mem_cpy32(void * __restrict__ dest, const void * __restrict__ src,
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
status_t mem_dump32(const void * buf, size_t n_bytes);

/**
 * \brief Dump 2 byte words in memory to stdout in hexadecimal
 *
 * \param buf Address for start of dump. If this is not 16-bit aligned, ERROR is
 * returned.
 * \param n_bytes How large of a dump to take, in bytes
 *
 * \return \ref status_t
 */
status_t mem_dump16(const void * buf, size_t n_bytes);

/**
 * \brief Dump 1 byte words in memory to stdout in hexadecimal
 *
 * \param buf Address for start of dump
 * \param n_bytes How large of a dump to take, in bytes
 *
 */
void mem_dump8(const void * buf, size_t n_bytes);

/**
 * \brief Dump 4 byte words in memory to stdout in hexadecimal, with offsets
 *
 * \param buf Address for start of dump. If this is not 32-bit aligned, ERROR is
 * returned.
 * \param n_bytes How large of a dump to take, in bytes
 *
 * \return \ref status_t
 */
status_t mem_dump32v(const void * buf, size_t n_bytes);

/**
 * \brief Dump 2 byte words in memory to stdout in hexadecimal, with offsets
 *
 * \param buf Address for start of dump. If this is not 16-bit aligned, ERROR is
 * returned.
 * \param n_bytes How large of a dump to take, in bytes
 *
 * \return \ref status_t
 */
status_t mem_dump16v(const void * buf, size_t n_bytes);

/**
 * \brief Dump 1 byte words in memory to stdout in hexadecimal, with offsets
 *
 * \param buf Address for start of dump
 * \param n_bytes How large of a dump to take, in bytes
 *
 */
void mem_dump8v(const void * buf, size_t n_bytes);

/**
 * \brief Byte swap memory in 1 byte chunks (upper 8/lower 8)
 *
 * \param buf Memory to swap. Must be 16-bit aligned or no action is
 * performed.
 * \param n_bytes # bytes to swap. Must be 16-bit aligned or no action is
 * performed.
 *
 * \return \ref status_t
 *
 */
status_t mem_bswap16(uint16_t * buf, size_t n_bytes);

/**
 * \brief Byte swap memory in 2 byte chunks (upper 16/lower 16)
 *
 * \param buf Memory to swap. Must be 32-bit aligned or no action is
 * performed.
 * \param n_bytes # bytes to swap. Must be 32-bit aligned or no action is
 * performed.
 *
 * \return \ref status_t
 *
 */
status_t mem_bswap32(uint32_t * buf, size_t n_bytes);

END_C_DECLS

#endif /* INCLUDE_RCSW_UTILS_MEM_H_ */
