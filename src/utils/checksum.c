/**
 * \file
 *
 * \copyright 2017 John Harwell
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/utils/checksum.h"

#include "rcsw/core/fpc.h"
#include "rcsw/er/client.h"
#include "rcsw/utils/align.h"

BEGIN_C_DECLS


/*******************************************************************************
 * Public API
 ******************************************************************************/
uint8_t utils_xchks8(const uint8_t* const buf, size_t n_bytes, uint8_t seed) {
  RCSW_FPC_NV((uint8_t)0xFF, buf != NULL);

  uint8_t chks8 = seed;

  for (size_t i = 0; i < n_bytes; ++i) {
    /* rotate checksum */
    chks8 = (uint8_t)(chks8 << 1) | ((chks8 & 0x80) >> 7);

    /* XOR in next data value */
    chks8 ^= buf[i];
  }
  return chks8;
}

uint16_t utils_xchks16(const uint16_t* const buf, size_t n_bytes, uint16_t seed) {
  RCSW_FPC_NV((uint16_t)0xFFFF,
              buf != NULL,
              RCSW_IS_MEM_ALIGNED(buf, sizeof(uint16_t)),
              RCSW_IS_SIZE_ALIGNED(n_bytes, sizeof(uint16_t)));

  uint16_t chks16 = seed;

  for (size_t i = 0; i < (n_bytes / sizeof(uint16_t)); ++i) {
    /* rotate checksum */
    chks16 = (uint16_t)(chks16 << 1) | ((chks16 & 0x8000) >> 15);

    /* XOR in next data value */
    chks16 ^= buf[i];
  }
  return chks16;
}

uint32_t utils_xchks32(const uint32_t* const buf, size_t n_bytes, uint32_t seed) {
  RCSW_FPC_NV(0xFFFFFFFF,
              buf != NULL,
              RCSW_IS_MEM_ALIGNED(buf, sizeof(uint32_t)),
              RCSW_IS_SIZE_ALIGNED(n_bytes, sizeof(uint32_t)));

  uint32_t chks32 = seed;

  for (size_t i = 0; i < (n_bytes / sizeof(uint32_t)); ++i) {
    /* rotate checksum */
    chks32 = (chks32 << 1) | ((chks32 & 0x80000000) >> 31);

    /* XOR in next data value */
    chks32 ^= buf[i];
  }
  return chks32;
}

uint8_t utils_achks8(const uint8_t* const buf, size_t n_bytes, uint8_t seed) {
  RCSW_FPC_NV(0xFF, buf != NULL);

  uint8_t chks8 = seed;

  for (size_t i = 0; i < n_bytes; ++i) {
    chks8 += buf[i];
  }

  return chks8;
}

uint16_t utils_achks16(const uint16_t* const buf, size_t n_bytes, uint16_t seed) {
  RCSW_FPC_NV(0xFFFF,
              buf != NULL,
              RCSW_IS_MEM_ALIGNED(buf, sizeof(uint16_t)),
              RCSW_IS_SIZE_ALIGNED(n_bytes, sizeof(uint16_t)));

  uint16_t chks16 = seed;
  for (size_t i = 0; i < (n_bytes / sizeof(uint16_t)); ++i) {
    chks16 += buf[i];
  }
  return chks16;
} /* achks16() */

uint32_t utils_achks32(const uint32_t* const buf, size_t n_bytes, uint32_t seed) {
  RCSW_FPC_NV(0xFFFFFFFF,
              buf != NULL,
              RCSW_IS_MEM_ALIGNED(buf, sizeof(uint32_t)),
              RCSW_IS_SIZE_ALIGNED(n_bytes, sizeof(uint32_t)));

  uint32_t chks32 = seed;

  for (size_t i = 0; i < (n_bytes / sizeof(uint32_t)); ++i) {
    chks32 += buf[i];
  }

  return chks32;
} /* achks32() */

END_C_DECLS
