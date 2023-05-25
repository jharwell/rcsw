/**
 * \file checksum.h
 * \ingroup utils
 * \brief Checksumming functions for embedded applications.
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
 * Function Prototypes
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Compute an 8-bit checksum (XOR, rotate)
 *
 * \param buf Buffer to compute over
 * \param n_bytes # bytes in buffer
 * \param seed Starting value for checksum
 *
 * \return computed 8-bit checksum
 */
uint8_t xchks8(const uint8_t * buf, size_t n_bytes, uint8_t seed) RCSW_PURE;

/**
 * \brief Compute an 16-bit checksum (XOR, rotate)
 *
 * Both buf and n_bytes must be 16-bit aligned, or no action is performed.
 *
 * \param buf Buffer to compute over
 * \param n_bytes # bytes in buffer
 * \param seed Starting value for checksum
 *
 * \return computed 16-bit checksum
 */
uint16_t xchks16(const uint16_t * buf, size_t n_bytes, uint16_t seed) RCSW_PURE;

/**
 * \brief Compute an 32-bit checksum (XOR, rotate)
 *
 * Both buf and n_bytes must be 32-bit aligned, or no action is performed.
 *
 * \param buf Buffer to compute over
 * \param n_bytes # bytes in buffer
 * \param seed Starting value for checksum
 *
 * \return Computed 32-bit checksum
 */
uint32_t xchks32(const uint32_t * buf, size_t n_bytes, uint32_t seed) RCSW_PURE;

/**
 * \brief Compute an 8-bit checksum (add-ignore-carry)
 *
 * \param buf Buffer to compute over
 * \param n_bytes # bytes in buffer
 * \param seed Starting value for checksum
 *
 * \return computed 8-bit checksum
 */
uint8_t achks8(const uint8_t * buf, size_t n_bytes, uint8_t seed) RCSW_PURE;

/**
 * \brief Compute an 16-bit checksum (add-ignore-carry)
 *
 * Both buf and n_bytes must be 16-bit aligned, or no action is performed
 *
 * \param buf Buffer to compute over
 * \param n_bytes # bytes in buffer
 * \param seed Starting value for checksum
 *
 * \return computed 16-bit checksum
 */
uint16_t achks16(const uint16_t * buf, size_t n_bytes, uint16_t seed) RCSW_PURE;

/**
 * \brief Compute an 32-bit checksum (add-ignore-carry)
 *
 * Both buf and n_bytes must be 16-bit aligned, or no action is performed
 *
 * \param buf Buffer to compute over
 * \param n_bytes # bytes in buffer
 * \param seed Starting value for checksum
 *
 * \return computed 32-bit checksum
 */
uint32_t achks32(const uint32_t * buf, size_t n_bytes, uint32_t seed) RCSW_PURE;

/**
 * \brief Compute a 16-bit checksum (add-ignore-carry) over 8-bit values
 *
 * \param buf Buffer to compute over
 * \param n_bytes # bytes in buffer
 * \param seed Starting value for checksum
 *
 * \return computed 16-bit checksum
 */
uint16_t achks8_16(const uint8_t * buf, size_t n_bytes, uint8_t seed) RCSW_PURE;

/**
 * \brief Compute a 32-bit CRC using the work of Gary S. Brown
 *
 *  COPYRIGHT (C) 1986 Gary S. Brown.  You may use this program, or
 *  code or tables extracted from it, as desired without restriction.
 *
 *  First, the polynomial itself and its table of feedback terms.  The
 *  polynomial is
 *  X^32+X^26+X^23+X^22+X^16+X^12+X^11+X^10+X^8+X^7+X^5+X^4+X^2+X^1+X^0
 *
 *  Note that we take it "backwards" and put the highest-order term in
 *  the lowest-order bit.  The X^32 term is "implied"; the LSB is the
 *  X^31 term, etc.  The X^0 term (usually shown as "+1") results in
 *  the MSB being 1
 *
 *  Note that the usual hardware shift register implementation, which
 *  is what we're using (we're merely optimizing it by doing eight-bit
 *  chunks at a time) shifts bits into the lowest-order term.  In our
 *  implementation, that means shifting towards the right.  Why do we
 *  do it this way?  Because the calculated CRC must be transmitted in
 *  order from highest-order term to lowest-order term.  UARTs transmit
 *  characters in order from LSB to MSB.  By storing the CRC this way
 *  we hand it to the UART in the order low-byte to high-byte; the UART
 *  sends each low-bit to hight-bit; and the result is transmission bit
 *  by bit from highest- to lowest-order term without requiring any bit
 *  shuffling on our part.  Reception works similarly
 *
 *  The feedback terms table consists of 256, 32-bit entries.  Notes
 *
 *      The table can be generated at runtime if desired; code to do so
 *      is shown later.  It might not be obvious, but the feedback
 *      terms simply represent the results of eight shift/xor opera
 *      tions for all combinations of data and CRC register values
 *
 *      The values must be right-shifted by eight bits by the "updcrc
 *      logic; the shift must be size_t (bring in zeroes).  On some
 *      hardware you could probably optimize the shift in assembler by
 *      using byte-swap instructions
 *      polynomial $edb88320
 *
 * \param buf Buffer to compute over
 * \param size # bytes in buffer
 * \param crc Starting value for crc
 *
 * \return The CRC
 */
uint32_t crc32_brown(const uint8_t * buf, uint32_t crc, size_t size);

/**
 * \brief Initialize Ethernet 32 bit CRC lookup table
 */
void crc32_ethl_init(void);

/**
 *
 * \brief 32 bit CRC according to IEEE 802.3 Ethernet standard
 *
 * Same as crc32_eth(), but uses a precomputed lookup table
 *
 * \param buf Buffer to compute over
 * \param n_bytes # bytes in buffer
 *
 * You have to call crc32_eth1_init() first, CRC will be wrong
 *
 * \return CRC of data
 *
 */
uint32_t crc32_ethl(const uint8_t * buf, size_t n_bytes);

/**
 *
 * \brief 32 bit CRC according to IEEE 802.3 Ethernet standard
 *
 * According to the standard, the CRC is:
 * - 32 bits
 * - The polynomial used to build lookup table is 0x04G11DB7,
 * - Initial remainder 0xFFFFFFFF
 * - 8 bit reflected input at EACH stage of the CRC calculation,
 * - Final 32 bit remainder reflected and THEN XORed with 0xFFFFFFFF
 *
 * \param buf Buffer to compute over
 * \param n_bytes # bytes in buffer
 *
 * \return CRC of data
 */
uint32_t crc32_eth(const uint8_t * buf, size_t n_bytes) RCSW_PURE;

END_C_DECLS

