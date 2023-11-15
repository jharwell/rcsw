/**
 * \file printf_gadget.h
 *
 * \author (c) Eyal Rozenberg <eyalroz1@gmx.com>
 *             2021-2022, Haifa, Palestine/Israel
 * \author (c) Marco Paland (info@paland.com)
 *             2014-2019, PALANDesign Hannover, Germany
 *
 * SPDX-License Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/stdio/printf_config.h"

#include "rcsw/al/types.h"
#include "rcsw/stdio/stdio.h"
#include "rcsw/common/common.h"

/* \cond INTERNAL */

/*******************************************************************************
 * Structure Definitions
 ******************************************************************************/
// wrapper (used as buffer) for output function type
//
// One of the following must hold:
// 1. max_chars is 0
// 2. buffer is non-null
// 3. function is non-null
//
// ... otherwise bad things will happen.
struct printf_output_gadget {
  void (*cb)(char c, void* extra_arg);
  void* extra_cb_arg;
  char* buffer;
  printf_size_t pos;
  printf_size_t max_chars;
};

/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS

static inline struct printf_output_gadget gadget_init(void) {
  struct printf_output_gadget gadget;
  gadget.cb = NULL;
  gadget.extra_cb_arg = NULL;
  gadget.buffer = NULL;
  gadget.pos = 0;
  gadget.max_chars = 0;
  return gadget;
}

static inline struct printf_output_gadget gadget_init_with_cb(
    void (*cb)(char, void*),
    void* extra_arg) {
  struct printf_output_gadget result = gadget_init();
  result.cb = cb;
  result.extra_cb_arg = extra_arg;
  result.max_chars = PRINTF_MAX_BUF_SIZE;
  return result;
}

static inline struct printf_output_gadget gadget_init_with_buf(
    char* buffer,
    size_t buffer_size) {
  printf_size_t usable_buffer_size = RCSW_MIN(buffer_size,
                                              (size_t)PRINTF_MAX_BUF_SIZE);

  struct printf_output_gadget result = gadget_init();
  if (buffer != NULL) {
    result.buffer = buffer;
    result.max_chars = usable_buffer_size;
  }
  return result;
}

/*
 * Note: This function currently assumes it is not passed a '\0' c, or
 * alternatively, that '\0' can be passed to the function in the output
 * gadget. The former assumption holds within the printf library. It also
 * assumes that the output gadget has been properly initialized.
 */
static inline void gadget_putchar(struct printf_output_gadget* gadget, char c) {
  printf_size_t write_pos = gadget->pos++;
    // We're _always_ increasing pos, so as to count how may characters
    // _would_ have been written if not for the max_chars limitation
  if (write_pos >= gadget->max_chars) {
    return;
  }
  if (gadget->cb != NULL) {
    // No check for c == '\0' .
    gadget->cb(c, gadget->extra_cb_arg);
  } else {
    // it must be the case that gadget->buffer != NULL , due to the constraint
    // on struct printf_output_gadget ; and note we're relying on write_pos
    // being non-negative.
    gadget->buffer[write_pos] = c;
  }
}

// Possibly-write the string-terminating '\0' character
static inline void gadget_append_termination(
    struct printf_output_gadget* gadget) {
  if (gadget->cb != NULL || gadget->max_chars == 0) {
    return;
  }
  if (gadget->buffer == NULL) {
    return;
  }
  printf_size_t null_char_pos = gadget->pos < gadget->max_chars ? gadget->pos : gadget->max_chars - 1;
  gadget->buffer[null_char_pos] = '\0';
}

static inline void putchar_wrapper(char c, void* unused) {
  (void) unused;
  stdio_putchar(c);
}

static inline struct printf_output_gadget gadget_putchar_extern(void) {
  return gadget_init_with_cb(putchar_wrapper, NULL);
}

/* \endcond */

END_C_DECLS
