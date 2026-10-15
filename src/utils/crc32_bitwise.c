/**
 * \file
 *
 * \copyright 2017 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * Bitwise CRC-32: no table, eight shift/XOR steps per byte.
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/utils/checksum.h"

#include "rcsw/core/fpc.h"

/*******************************************************************************
 * Constant Definitions
 ******************************************************************************/
/* 0x04C11DB7 bit-reversed, for the LSB-first (reflected) computation */
#define CRC32_POLYNOMIAL_REFLECTED 0xEDB88320U

/*******************************************************************************
 * Public API
 ******************************************************************************/
BEGIN_C_DECLS

uint32_t utils_crc32_bitwise(const uint8_t* const buf,
                             size_t               n_bytes,
                             uint32_t             crc) {
  RCSW_FPC_NV(0xFFFFFFFF, NULL != buf || 0 == n_bytes);

  /* Same pre-/post-inversion as utils_crc32(), so the results are identical */
  crc = ~crc;
  for (size_t i = 0; i < n_bytes; ++i) {
    crc ^= buf[i];
    for (int bit = 0; bit < 8; ++bit) {
      uint32_t mask = -(crc & 1U);
      crc           = (crc >> 1) ^ (CRC32_POLYNOMIAL_REFLECTED & mask);
    } /* for(bit..) */
  } /* for(i..) */
  return ~crc;
} /* utils_crc32_bitwise() */

END_C_DECLS
