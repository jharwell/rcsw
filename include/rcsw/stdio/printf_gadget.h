/**
 * \file printf_gadget.h
 *
 * \copyright 2023 John Harwell, All rights reserved.
 *
 * SPDX-License Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/stdio/printf_config.h"

#include "rcsw/common/types.h"
#include "rcsw/stdio/stdio.h"

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
  void (*function)(char c, void* extra_arg);
  void* extra_function_arg;
  char* buffer;
  printf_size_t pos;
  printf_size_t max_chars;
};

/*******************************************************************************
 * API Functions
 ******************************************************************************/
BEGIN_C_DECLS
// Note: This function currently assumes it is not passed a '\0' c,
// or alternatively, that '\0' can be passed to the function in the output
// gadget. The former assumption holds within the printf library. It also
// assumes that the output gadget has been properly initialized.
static inline void putchar_via_gadget(struct printf_output_gadget* gadget, char c) {
  printf_size_t write_pos = gadget->pos++;
    // We're _always_ increasing pos, so as to count how may characters
    // _would_ have been written if not for the max_chars limitation
  if (write_pos >= gadget->max_chars) {
    return;
  }
  if (gadget->function != NULL) {
    // No check for c == '\0' .
    gadget->function(c, gadget->extra_function_arg);
  }
  else {
    // it must be the case that gadget->buffer != NULL , due to the constraint
    // on struct printf_output_gadget ; and note we're relying on write_pos
    // being non-negative.
    gadget->buffer[write_pos] = c;
  }
}

// Possibly-write the string-terminating '\0' character
static inline void append_termination_with_gadget(struct printf_output_gadget* gadget)
{
  if (gadget->function != NULL || gadget->max_chars == 0) {
    return;
  }
  if (gadget->buffer == NULL) {
    return;
  }
  printf_size_t null_char_pos = gadget->pos < gadget->max_chars ? gadget->pos : gadget->max_chars - 1;
  gadget->buffer[null_char_pos] = '\0';
}

// We can't use putchar_ as is, since our output gadget
// only takes pointers to functions with an extra argument
static inline void putchar_wrapper(char c, void* unused)
{
  (void) unused;
  stdio_putchar(c);
}

static inline struct printf_output_gadget discarding_gadget(void)
{
  struct printf_output_gadget gadget;
  gadget.function = NULL;
  gadget.extra_function_arg = NULL;
  gadget.buffer = NULL;
  gadget.pos = 0;
  gadget.max_chars = 0;
  return gadget;
}

static inline struct printf_output_gadget buffer_gadget(char* buffer, size_t buffer_size)
{
  printf_size_t usable_buffer_size = (buffer_size > PRINTF_MAX_POSSIBLE_BUFFER_SIZE) ?
    PRINTF_MAX_POSSIBLE_BUFFER_SIZE : (printf_size_t) buffer_size;
  struct printf_output_gadget result = discarding_gadget();
  if (buffer != NULL) {
    result.buffer = buffer;
    result.max_chars = usable_buffer_size;
  }
  return result;
}

static inline struct printf_output_gadget function_gadget(void (*function)(char, void*), void* extra_arg)
{
  struct printf_output_gadget result = discarding_gadget();
  result.function = function;
  result.extra_function_arg = extra_arg;
  result.max_chars = PRINTF_MAX_POSSIBLE_BUFFER_SIZE;
  return result;
}

static inline struct printf_output_gadget extern_putchar_gadget(void)
{
  return function_gadget(putchar_wrapper, NULL);
}

END_C_DECLS
