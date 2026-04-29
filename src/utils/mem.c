/**
 * \file
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/utils/mem.h"

#include "rcsw/core/fpc.h"
#include "rcsw/er/client.h"
#include "rcsw/utils/align.h"
#include "rcsw/utils/byteops.h"

/*******************************************************************************
 * Public API
 ******************************************************************************/
BEGIN_C_DECLS

status_t utils_mem_write32(size_t addr, uint32_t wval) {
  RCSW_FPC_NV(ERROR, RCSW_IS_MEM_ALIGNED(addr, sizeof(uint32_t)));
  *((volatile uint32_t*)addr) = wval;
  return OK;
}

uint32_t utils_mem_read32(size_t addr) {
  RCSW_FPC_NV(0, (RCSW_IS_MEM_ALIGNED(addr, sizeof(uint32_t))));
  return *((volatile uint32_t*)addr);
}

status_t utils_mem_rmwr32(uint32_t addr, uint32_t wval, uint32_t mask) {
  RCSW_FPC_NV(ERROR, RCSW_IS_MEM_ALIGNED(addr, sizeof(uint32_t)));

  volatile uint32_t curr_val = 0;

  /*
   * If some bits masked, read the memory address, mask off all bits
   * NOT in mask in the result. OR this value with all bits in wval that
   * ARE in the mask.
   */
  if (mask != 0) {
    curr_val = utils_mem_read32(addr);
    wval     = (curr_val & ~mask) | (wval & mask);
  }

  /*
   * Write masked off bits back to memory/register, or just write wval to
   * memory, if mask was 0.
   */
  utils_mem_write32(addr, wval);

  /* read & compare */
  curr_val = utils_mem_read32(addr);
  RCSW_CHECK((curr_val & mask) == (wval & mask));

  return OK;

error:
  return ERROR;
}

void* utils_mem_cpy32(void* const __restrict__ dest,
                      const void* const __restrict__ src,
                      size_t n_bytes) {
  RCSW_FPC_NV(dest,
              RCSW_IS_MEM_ALIGNED(dest, sizeof(uint32_t)),
              RCSW_IS_MEM_ALIGNED(src, sizeof(uint32_t)),
              RCSW_IS_SIZE_ALIGNED(n_bytes, sizeof(uint32_t)));

  for (size_t i = 0; i < n_bytes / sizeof(uint32_t); ++i) {
    ((volatile uint32_t*)dest)[i] = ((const volatile uint32_t*)src)[i];
  } /* for() */
  return dest;
}

status_t utils_mem_dump32(const void* const buf, size_t n_bytes) {
  RCSW_FPC_NV(ERROR, RCSW_IS_MEM_ALIGNED(buf, sizeof(uint32_t)));

  for (uint32_t i = 0; i < n_bytes / sizeof(uint32_t); i++) {
    DPRINTF("%08x ", ((const uint32_t*)buf)[i]);
    if ((i + 1) % 8 == 0) {
      DPRINTF("\n");
    }
  }
  DPRINTF("\n");
  return OK;
}

status_t utils_mem_dump16(const void* const buf, size_t n_bytes) {
  RCSW_FPC_NV(ERROR, RCSW_IS_MEM_ALIGNED(buf, sizeof(uint16_t)));

  for (uint32_t i = 0; i < n_bytes / sizeof(uint16_t); i++) {
    DPRINTF("%04x ", ((const uint16_t*)buf)[i]);
    if ((i + 1) % 16 == 0) {
      DPRINTF("\n");
    }
  }
  DPRINTF("\n");
  return OK;
}

void utils_mem_dump8(RCSW_UNUSED const void* const buf, size_t n_bytes) {
  for (uint32_t i = 0; i < n_bytes; i++) {
    DPRINTF("%02x ", ((const uint8_t*)buf)[i]);
    if ((i + 1) % 32 == 0) {
      DPRINTF("\n");
    }
  }
  DPRINTF("\n");
}

status_t utils_mem_dump32v(const void* const buf, size_t n_bytes) {
  RCSW_FPC_NV(ERROR, RCSW_IS_MEM_ALIGNED(buf, sizeof(uint32_t)));
  DPRINTF("Offset:   ");

  for (uint32_t i = 0; i < 8; i++) {
    DPRINTF("%02x       ", i);
  }
  DPRINTF("\n\n");

  for (uint32_t i = 0; i < n_bytes / sizeof(uint32_t); i++) {
    if (i % 8 == 0) {
      DPRINTF("%08x  ", i * 4);
    }
    DPRINTF("%08x ", ((const uint32_t*)buf)[i]);
    if ((i + 1) % 8 == 0) {
      DPRINTF("\n");
    }
  }
  DPRINTF("\n");
  return OK;
}

status_t utils_mem_dump16v(const void* const buf, size_t n_bytes) {
  RCSW_FPC_NV(ERROR, RCSW_IS_MEM_ALIGNED(buf, sizeof(uint16_t)));

  DPRINTF("Offset:   ");
  for (uint32_t i = 0; i < 16; i++) {
    DPRINTF("%02x   ", i);
  }
  DPRINTF("\n\n");
  for (uint32_t i = 0; i < n_bytes / sizeof(uint16_t); i++) {
    if (i % 16 == 0) {
      DPRINTF("%08x  ", i * 2);
    }

    DPRINTF("%04x ", ((const uint16_t*)buf)[i]);
    if ((i + 1) % 16 == 0) {
      DPRINTF("\n");
    }
  }
  DPRINTF("\n");
  return OK;
}

void utils_mem_dump8v(RCSW_UNUSED const void* const buf, size_t n_bytes) {
  uint32_t i;
  DPRINTF("Offset:   ");
  for (i = 0; i < 32; i++) {
    DPRINTF("%02x ", i);
  }
  DPRINTF("\n\n");
  for (i = 0; i < n_bytes; i++) {
    if (i % 32 == 0) {
      DPRINTF("%08x  ", i);
    }
    DPRINTF("%02x ", ((const uint8_t*)buf)[i]);
    if ((i + 1) % 32 == 0) {
      DPRINTF("\n");
    }
  }
  DPRINTF("\n");
}

status_t utils_mem_bswap16(uint16_t* const buf, size_t n_bytes) {
  RCSW_FPC_NV(ERROR,
              RCSW_IS_MEM_ALIGNED(buf, sizeof(uint16_t)),
              RCSW_IS_SIZE_ALIGNED(n_bytes, sizeof(uint16_t)));

  for (size_t i = 0; i < n_bytes / sizeof(uint16_t); ++i) {
    buf[i] = (uint16_t)RCSW_BSWAP16(buf[i]);
  } /* for() */

  return OK;
}

status_t utils_mem_bswap32(uint32_t* const buf, size_t n_bytes) {
  RCSW_FPC_NV(ERROR,
              RCSW_IS_MEM_ALIGNED(buf, sizeof(uint32_t)),
              RCSW_IS_SIZE_ALIGNED(n_bytes, sizeof(uint32_t)));

  uint32_t i;
  for (i = 0; i < n_bytes / sizeof(uint32_t); ++i) {
    buf[i] = RCSW_BSWAP32(buf[i]);
  }

  return OK;
}

END_C_DECLS
