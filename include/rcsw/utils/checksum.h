/**
 * \file
 *
 * \copyright 2017 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * \ingroup utils
 *
 * \brief Checksumming functions for embedded applications.
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include <stddef.h>
#include <stdint.h>

#include "rcsw/core/compilers.h"

/*******************************************************************************
 * Public API
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Compute an 8-bit checksum (XOR, rotate)
 *
 * \param buf Buffer to compute over
 * \param n_bytes # bytes in buffer
 * \param seed Starting value for checksum
 *
 * \return Computed 8-bit checksum
 */
RCSW_API uint8_t utils_xchks8(const uint8_t* buf,
                              size_t         n_bytes,
                              uint8_t        seed) RCSW_PURE;

/**
 * \brief Compute an 16-bit checksum (XOR, rotate)
 *
 * buf must be 16-bit aligned and n_bytes a multiple of 2; otherwise
 * 0xFFFF is returned (indistinguishable from a real checksum, so check
 * alignment beforehand).
 *
 * \param buf Buffer to compute over
 * \param n_bytes # bytes in buffer
 * \param seed Starting value for checksum
 *
 * \return computed 16-bit checksum
 */
RCSW_API uint16_t utils_xchks16(const uint16_t* buf,
                                size_t          n_bytes,
                                uint16_t        seed) RCSW_PURE;

/**
 * \brief Compute an 32-bit checksum (XOR, rotate)
 *
 * buf must be 32-bit aligned and n_bytes a multiple of 4; otherwise
 * 0xFFFFFFFF is returned (indistinguishable from a real checksum, so check
 * alignment beforehand).
 *
 * \param buf Buffer to compute over
 * \param n_bytes # bytes in buffer
 * \param seed Starting value for checksum
 *
 * \return Computed 32-bit checksum
 */
RCSW_API uint32_t utils_xchks32(const uint32_t* buf,
                                size_t          n_bytes,
                                uint32_t        seed) RCSW_PURE;

/**
 * \brief Compute an 8-bit checksum (add-ignore-carry)
 *
 * \param buf Buffer to compute over
 * \param n_bytes # bytes in buffer
 * \param seed Starting value for checksum
 *
 * \return computed 8-bit checksum
 */
RCSW_API uint8_t utils_achks8(const uint8_t* buf,
                              size_t         n_bytes,
                              uint8_t        seed) RCSW_PURE;

/**
 * \brief Compute an 16-bit checksum (add-ignore-carry)
 *
 * buf must be 16-bit aligned and n_bytes a multiple of 2; otherwise
 * 0xFFFF is returned (indistinguishable from a real checksum, so check
 * alignment beforehand).
 *
 * \param buf Buffer to compute over
 * \param n_bytes # bytes in buffer
 * \param seed Starting value for checksum
 *
 * \return computed 16-bit checksum
 */
RCSW_API uint16_t utils_achks16(const uint16_t* buf,
                                size_t          n_bytes,
                                uint16_t        seed) RCSW_PURE;

/**
 * \brief Compute an 32-bit checksum (add-ignore-carry)
 *
 * buf must be 32-bit aligned and n_bytes a multiple of 4; otherwise
 * 0xFFFFFFFF is returned (indistinguishable from a real checksum, so check
 * alignment beforehand).
 *
 * \param buf Buffer to compute over
 * \param n_bytes # bytes in buffer
 * \param seed Starting value for checksum
 *
 * \return computed 32-bit checksum
 */
RCSW_API uint32_t utils_achks32(const uint32_t* buf,
                                size_t          n_bytes,
                                uint32_t        seed) RCSW_PURE;

/**
 * \brief Compute the standard CRC-32 (IEEE 802.3 / Ethernet, zlib, PNG) of a
 * buffer, using a 256-entry lookup table.
 *
 * Polynomial 0x04C11DB7, reflected input and output, initial value and final
 * XOR 0xFFFFFFFF: the CRC of "123456789" is 0xCBF43926. Derived from Gary S.
 * Brown's public-domain table-driven implementation (1986).
 *
 * The table is 1 KiB of constant data. Where that is too much, use \ref
 * utils_crc32_bitwise(), which returns the same values without a table and is
 * several times slower. The two are in separate object files, so only the one
 * used is linked in.
 *
 * \param buf Buffer to compute over. May be NULL if \p n_bytes is 0.
 * \param n_bytes # bytes in buffer.
 * \param crc 0 to start a new CRC, or the result of a previous call to
 *            continue it over another buffer.
 *
 * \return The CRC.
 */
RCSW_API uint32_t utils_crc32(const uint8_t* buf, size_t n_bytes, uint32_t crc);

/**
 * \brief Same as \ref utils_crc32(), computed bit by bit without a lookup
 * table: smaller, but slower.
 */
RCSW_API uint32_t utils_crc32_bitwise(const uint8_t* buf,
                                      size_t         n_bytes,
                                      uint32_t       crc);

END_C_DECLS
