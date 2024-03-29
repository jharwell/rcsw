/**
 * \file utils.c
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/utils/utils.h"

#include <math.h>
#include <stdlib.h>

#include "rcsw/common/fpc.h"
#include "rcsw/er/client.h"

/*******************************************************************************
 * Global Variables
 ******************************************************************************/
const uint8_t rcsw_util_revtable[] = {
  0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30,
  0xB0, 0x70, 0xF0, 0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98,
  0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8, 0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64,
  0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4, 0x0C, 0x8C, 0x4C, 0xCC,
  0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC, 0x02,
  0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2,
  0x72, 0xF2, 0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A,
  0xDA, 0x3A, 0xBA, 0x7A, 0xFA, 0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6,
  0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6, 0x0E, 0x8E, 0x4E, 0xCE, 0x2E,
  0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE, 0x01, 0x81,
  0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71,
  0xF1, 0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9,
  0x39, 0xB9, 0x79, 0xF9, 0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15,
  0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5, 0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD,
  0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD, 0x03, 0x83, 0x43,
  0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
  0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B,
  0xBB, 0x7B, 0xFB, 0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97,
  0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7, 0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F,
  0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF
};

/*******************************************************************************
 * Time Functions
 ******************************************************************************/
BEGIN_C_DECLS

/*******************************************************************************
 * Checksum Functions
 ******************************************************************************/
status_t util_string_gen(char* const buf, size_t len) {
  RCSW_FPC_NV(ERROR, buf != NULL);

  /* ASCII characters 33 to 126 */
  size_t n_chars = len - 1;
  size_t i;
  for (i = 0; i < n_chars; ++i) {
    buf[i] = random() % (126 - 33 + 1) + 33;
  }
  buf[n_chars] = '\0';

  return OK;
} /* string_gen() */

uint32_t util_reflect32(uint32_t data, size_t n_bits) {
  uint32_t reflection = 0x00000000;

  /* reflect the data about the center bit */
  for (uint8_t bit = 0; bit < n_bits; ++bit) {
    /* if the LSB bit is set, set the reflection of it */
    if (data & 0x01) {
      reflection |= (1 << ((n_bits - 1) - bit));
    }
    data = (data >> 1);
  }
  return reflection;
} /* reflect32() */

void arr8_reverse(void* const arr, size_t size) {
  for (size_t i = 0, j = size - 1; i < j; i++, j--) {
    uint8_t tmp = ((uint8_t*)arr)[i];
    ((uint8_t*)arr)[i] = ((uint8_t*)arr)[j];
    ((uint8_t*)arr)[j] = tmp;
  } /* for() */
} /* arr8_reverse() */

void arr32_permute(uint32_t* arr,
                   size_t size,
                   size_t start,
                   void (*fp)(uint32_t* const elt)) {
  /*
   * If we are at the end of the array, we have one permutation we can use
   */
  if (start == size) {
    fp(arr);
  } else {
    /*
     * recursively explore the permutations starting
     * at index start going through index size - 1
     */
    for (size_t j = start; j < size; ++j) {
      /* try the array with start and j switched */
      arr32_elt_swap(arr, start, j);
      arr32_permute(arr, size, start + 1, fp);

      /* swap them back the way they were */
      arr32_elt_swap(arr, start, j);
    }
  }
} /* arr_permute() */

void arr32_elt_swap(uint32_t* const v, size_t i, size_t j) {
  uint32_t t = v[i];
  v[i] = v[j];
  v[j] = t;
} /* arr32_elt_swap() */

bool_t util_zchk(void* const elt, size_t elt_size) {
  RCSW_FPC_NV(false, NULL != elt, elt_size > 0);
  bool_t sum = 0;

  switch (elt_size) {
    case sizeof(uint8_t):
      return *((uint8_t*)(elt)) == 0;
    case sizeof(uint16_t):
      return *((uint16_t*)(elt)) == 0;
    case sizeof(uint32_t):
      return *((uint32_t*)(elt)) == 0;

      /*
       * sizeof(float) is the same as sizeof(uint32_t) on most platforms, but
       * not all.
       */
#if __SIZEOF_FLOAT__ != 4
    case sizeof(float):
      return fabs(*((double*)(elt))) <= RCSW_FLOAT_EPSILON;
      break;
#endif
    case sizeof(double):
      return fabs(*((double*)(elt))) <= RCSW_DOUBLE_EPSILON;
      break;
    default:
      for (size_t i = 0; i < elt_size; ++i) {
        sum |= ((uint8_t*)elt)[i];
      } /* for(i..) */
      return sum;
  } /* switch() */
} /* ds_elt_zchk() */

END_C_DECLS
