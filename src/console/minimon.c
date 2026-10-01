/**
 * \file
 *
 * \copyright 2023 John Harwell
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/console/minimon.h"

#include <inttypes.h>

#include "rcsw/core/core.h"
#include "rcsw/er/client.h"
#include "rcsw/stdio/printf.h"
#include "rcsw/stdio/stdio.h"
#include "rcsw/stdio/string.h"
#include "rcsw/utils/checksum.h"
#include "rcsw/version/version.h"

/*******************************************************************************
 * Constants
 ******************************************************************************/
/**
 * \brief The interval for pushing updates on send/receiver progress out over
 * stream0, in units of data bytes transmitted.
 */

#define MINIMON_DIAG_INTERVAL 256

/**
 * \brief The # of bytes to send in a single message when using the NMEA-like
 * protocol.
 */
#define MINIMON_NMEA_CHUNK_SIZE 64

/*******************************************************************************
 * Types
 ******************************************************************************/
/**
 * \brief Convenience buffer for sending out bytes from a \p uint32_t pointer.
 */
union word_buf {
  uint8_t  byte[sizeof(uint32_t)];
  uint32_t word;
};

/*******************************************************************************
 * Private API
 ******************************************************************************/
/**
 * \brief Displays the help message for minimon showing valid commands and what
 * the syntax for each is.
 */
static void mini_cmd_help_all(const char*, ...);

/**
 * \brief Read 1 or more words of memory, starting at the specified address. The
 * contents of memory are printed to the serial terminal. The address is not
 * checked for alignment.
 */
static void mini_cmd_read(const char*, ...);

/**
 * \brief Writes 1 or more words of memory, starting at the specified
 * address, with the specified value. All words are written with the same value.

 ** For large writes, the progress is printed to the serial terminal. The
 * starting address is not checked for word-alignment.
 */
static void mini_cmd_write(const char* cmdname, ...);

/**
 * \brief Jump to another memory location and begin execution there. The
 * validity of the address to jump to is not checked.
 *
 * This function only applies to the CPU that minimon is running on;
 * multiprocessing jumps not supported.
 */
static void mini_cmd_jump(const char* cmdname, ...);

/**
 * \brief Receives a program byte by byte through the serial terminal
 * and then writes it to the specified memory location.
 */
static void mini_cmd_load(const char* cmdname, ...);

/**
 * \brief Send bytes from a specified memory location through the serial
 * terminal.
 */
static void mini_cmd_send(const char* cmdname, ...);

/*******************************************************************************
 * Private Functions
 ******************************************************************************/
/**
 * \brief Unused command/parameter table entries have NULL or empty names.
 */
static bool_t mini_str_empty(const char* s) { return NULL == s || '\0' == *s; }

static void mini_data_byte_put(char c, void* extra) {
  struct minimon* m = extra;
  if (NULL != m->stream1.putchar) {
    m->stream1.putchar(c);
  } else {
    stdio_putchar(c);
  }
}
static int mini_data_byte_get(void* extra) {
  struct minimon* m = extra;
  if (NULL != m->stream1.getchar) {
    return m->stream1.getchar();
  }
  return stdio_getchar();
}

/*******************************************************************************
 * Global Variables
 ******************************************************************************/
static const struct minimon_cmd g_minimon_builtin_cmds[] = {
  {
    .name   = "help",
    .alias  = "h",
    .help   = "Print help for all cmds or a specific cmd",
    .hook   = mini_cmd_help_all,
    .config = {{.name       = "CMD",
                .type       = MINIMON_PARAM_STR,
                .short_help = NULL,
                .long_help  = "The specific cmd to see detailed help for",
                .required   = false,
                .dflt       = {.s = NULL}}},
  },
  {.name   = "read",
   .alias  = "r",
   .help   = "Read from memory",
   .hook   = mini_cmd_read,
   .config = {{
                .name       = "ADDR",
                .type       = MINIMON_PARAM_UINT32,
                .short_help = NULL,
                .long_help =
                  "The 32-bit address in memory (alignment not checked)",
                .required = true,
              },
              {.name       = "SIZE",
               .type       = MINIMON_PARAM_UINT32,
               .short_help = NULL,
               .long_help  = "The number of 32-bit words to read",
               .required   = false,
               .dflt       = {.num = 1}}}},
  {.name   = "load",
   .alias  = "l",
   .help   = "Load bytes via stdio_getchar() from the remote target into memory",
   .hook   = mini_cmd_load,
   .config = {{
                .name       = "ADDR",
                .type       = MINIMON_PARAM_UINT32,
                .short_help = NULL,
                .long_help =
                  "The 32-bit dest address in memory (alignment not checked)",
                .required = true,
              },
              {
                .name       = "SIZE",
                .type       = MINIMON_PARAM_UINT32,
                .short_help = NULL,
                .long_help  = "The number of bytes to receive",
                .required   = true,
              }}},
  {.name  = "send",
   .alias = "s",
   .help  = "Send from memory via stdio_getchar() to the remote target",
   .hook  = mini_cmd_send,
   .config =
     {{
        .name       = "ADDR",
        .type       = MINIMON_PARAM_UINT32,
        .short_help = NULL,
        .long_help =
          "The 32-bite source address in memory (alignment not checked)",
        .required = true,
      },
      {
        .name       = "SIZE",
        .type       = MINIMON_PARAM_UINT32,
        .short_help = NULL,
        .long_help  = "The number of bytes to send",
        .required   = true,
      },
      {
        .name       = "PROTOCOL",
        .type       = MINIMON_PARAM_STR,
        .short_help = "The protocol to use [raw, NMEA]",
        .long_help =
          "The protocol to use. Options are:\r\n\r\n"
          "\t\traw - What is sent is exactly what is in the source buffer.\r\n"

          "\t\tNMEA - What is sent is first converted to ASCII, then placed\r\n"
          "\t\t       in simple text packets. Each packet contains " RCSW_XSTR(
            MINIMON_NMEA_CHUNK_SIZE) " bytes of data.\r\n"
                                     "\t\t       Packets have the following "
                                     "form:\r\n"
                                     "\t\t\r\n"
                                     "\t\t       $SEND,ID,X,Y,Z,...*C\r\n"
                                     "\t\t\r\n"
                                     "\t\t       ID is the 0-based index of the "
                                     "chunk of data in the packet.\r\n"
                                     "\t\t       X,Y,Z,... are the ASCII "
                                     "representations of 32-bit\r\n"
                                     "\t\t       WORDS (not bytes) of data, "
                                     "followed by a '*'. C is a one byte\r\n"
                                     "\t\t       rotating XOR checksum in hex. "
                                     "See docs for more details.\r\n"
                                     "\t\t\r\n",
        .required = true,
      }}},
  {.name   = "write",
   .alias  = "w",
   .help   = "Write to memory",
   .hook   = mini_cmd_write,
   .config = {{
                .name       = "ADDR",
                .type       = MINIMON_PARAM_UINT32,
                .short_help = NULL,
                .long_help =
                  "The 32-bit address in memory (alignment not checked)",
                .required = true,
              },
              {
                .name       = "VALUE",
                .type       = MINIMON_PARAM_UINT32,
                .short_help = NULL,
                .long_help  = "The 32-bit value to write",
                .required   = true,
              },
              {.name       = "SIZE",
               .type       = MINIMON_PARAM_UINT32,
               .short_help = NULL,
               .long_help  = "The number of 32-bit values to write",
               .required   = false,
               .dflt       = {.num = 1}}}},
  {.name  = "jump",
   .alias = "j",
   .help =
     "Jump to a new execution address, masking/clearing all IRQs beforehand",
   .hook = mini_cmd_jump,
   .config =
     {
       {
         .name       = "ADDR",
         .type       = MINIMON_PARAM_UINT32,
         .short_help = NULL,
         .long_help  = "The address in memory (validity not checked)",
         .required   = true,
       },
     }},
};

static struct minimon g_minimon;

/*******************************************************************************
 * Private Functions
 ******************************************************************************/
/**
 * \brief Validate a string representing an integer, checking that there are no
 * illegal characters in it
 *
 * Hex representations of numbers are supported. If a number is a hexadecimal
 * number, it MUST be preceded by 0x or 0X at the prompt, otherwise minimon will
 * assume it is a decimal number, and convert it accordingly.
 *
 * \param numstr  String to be validated
 * \param num Pointer to the parsed number, to be filled
 */
static status_t mini_validate_int(char* numstr, uint32_t* num) {
  size_t length;
  bool_t is_hex = 0;

  length = stdio_strlen(numstr);

  /* validate string length */
  if ((length == 0) || (length > 10)) {
    ER_ERR("Token length %zu outside of permissible range [1-10]", length);
    return ERROR;
  }
  if (stdio_strncmp(numstr, "0X", 2) == 0 ||
      stdio_strncmp(numstr, "0x", 2) == 0) {
    is_hex = true;
    ER_TRACE("Token '%s' is hexadecimal", numstr);
    numstr += 2;
    length -= 2;
  }

  /* check string contents for illegal characters */
  for (size_t i = 0; i < length; i++) {
    if (!RCSW_STDIO_ISHEX(numstr[i])) {
      ER_ERR("Token contains illegial char '%c' at idx=%zu", numstr[i], i);
      return ERROR;
    }
  }

  if (is_hex) {
    numstr -= 2;
    *num = (uint32_t)stdio_atoi(numstr, 16);
  } else {
    *num = (uint32_t)stdio_atoi(numstr, 10);
  }
  return OK;
} /* mini_validate_int() */

/**
 * \brief Validate the arguments for a given command.
 *
 * \param argc Number of space-delimited tokens on the command line including
 *             the command name.
 *
 * \param argv Array with string representations of all args to the command.
 *
 * \param config Parameter array with \a effective parameters; defaults already
 *               copied in.
 *
 * \param args Parameter definition containing how many, what type, etc. of
 *             config are required.
 */
static status_t mini_validate_args(uint32_t                        argc,
                                   char**                          argv,
                                   const struct minimon_cmd_param* config,
                                   union minimon_cmd_arg*          args) {
  status_t rstat = OK;

  size_t min_config = 0;
  size_t max_config = 0;

  for (uint32_t i = 0; i < MINIMON_CMD_MAX_ARGS; ++i) {
    if (!mini_str_empty(config[i].name)) {
      ++max_config;

      if (config[i].required) {
        ++min_config;
      }
    }
  } /* for(i..) */

  /* validate parameter count */
  if ((argc - 1) < min_config) {
    ER_ERR("Too few arguments--type 'help %s' for help", argv[0]);
    return ERROR;
  }
  if ((argc - 1) > max_config) {
    ER_ERR("Too many arguments--type 'help %s' for help", argv[0]);
    return ERROR;
  }

  /* validate arguments */
  for (uint32_t i = 0; i < argc - 1; i++) {
    /* validate numerics */
    switch (config[i].type) {
      case MINIMON_PARAM_UINT32:
        rstat |= mini_validate_int(argv[i + 1], &args[i].num);
        break;
      case MINIMON_PARAM_STR:
        args[i].s = argv[i + 1];
        break;
      default:
        break;
    } /* switch() */

    if (OK != rstat) {
      ER_ERR("Invalid arg[%d] = '%s'", (i + 1), argv[i + 1]);
    }
  }

  return rstat;
} /* mini_validate_args() */

/**
 * \brief Read a line from the serial terminal.
 *
 * Lines are delimited by newlines or carriage returns. Backspace and DEL erase
 * the previous character. Characters beyond \p size - 1 are dropped until the
 * end of the line.
 *
 * A negative value from \ref stdio_getchar() (EOF, or "no character
 * available" from a polling driver) is not a character and is ignored.
 *
 * \param buf To fill with read-in line; always NUL-terminated.
 * \param size Size of \p buf, including the '\0'.
 */
static void mini_readline(char* buf, size_t size) {
  size_t i = 0;

  for (;;) {
    int c = stdio_getchar();
    if (c <= 0) { /* nothing received yet, or NUL */
      continue;
    }
    if (c == '\n' || c == '\r') {
      break;
    }
    if (c == '\b' || c == 0x7F) {
      if (i > 0) {
        stdio_printf("\b \b");
        i--;
      }
      continue;
    }
    if (i + 1 < size) {
      buf[i++] = (char)c;
      stdio_putchar((char)c); /* echo back to the terminal */
    }
  } /* for(;;) */
  buf[i] = '\0';
} /* mini_readline() */

/**
 * \brief Parse the command string into cmd + a set of arguments.
 *
 * \return The number of delimited arguments found + 1.
 */
static uint32_t mini_parse(char* buffer, char** argv) {
  uint32_t i              = 0;     /* string char index */
  uint32_t j              = 0;     /* the number of tokens found */
  bool_t   last_was_space = false; /* was last char a delimiter? */

  /*
   * argv holds the command + MINIMON_CMD_MAX_ARGS arguments. Every token is
   * counted, but only those that fit are stored, so an over-long line fails
   * argument validation ("Too many arguments").
   */
  argv[j++] = &buffer[0]; /* argv[0] is cmd name */

  while (buffer[i]) {
    if ((buffer[i] == ' ') || (buffer[i] == ',')) {
      buffer[i++]    = 0;
      last_was_space = true;
    } else if (last_was_space) {
      if (j <= MINIMON_CMD_MAX_ARGS) {
        argv[j] = &buffer[i];
      }
      j++;
      i++;
      last_was_space = false;
    } else {
      i++;
    }
  }
  if (j <= MINIMON_CMD_MAX_ARGS) {
    argv[j] = "";
  }
  return j;
} /* mini_parse() */

/**
 * \brief The number of registered commands: the built-ins in use, then the
 * caller's table.
 */
static size_t mini_n_cmds(void) {
  return g_minimon.n_builtin_cmds + g_minimon.n_user_cmds;
}

/**
 * \brief Get registered command \p i (0 <= i < mini_n_cmds()). Built-ins come
 * first, then the caller's commands; neither table is copied.
 */
static const struct minimon_cmd* mini_cmd_get(size_t i) {
  if (i < g_minimon.n_builtin_cmds) {
    return &g_minimon_builtin_cmds[i];
  }
  return &g_minimon.user_cmds[i - g_minimon.n_builtin_cmds];
}

/**
 * \brief Parse, validate, and execute the command read in from the serial
 *        terminal.
 */
static void mini_dispatch(int argc, char** argv) {
  bool_t                cmd_found = false;
  union minimon_cmd_arg args[MINIMON_CMD_MAX_ARGS];
  stdio_memset(&args, 0, sizeof(args));

  if (*argv[0] == '\0') { /* ignore null commands */
    return;
  }

  /* select a command from dispatch list and call the corresponding handler. */
  for (size_t i = 0; i < mini_n_cmds(); ++i) {
    const struct minimon_cmd* cmd = mini_cmd_get(i);

    if (mini_str_empty(cmd->name)) {
      continue;
    }

    if (0 == stdio_strcmp(cmd->name, argv[0]) ||
        (NULL != cmd->alias && 0 == stdio_strcmp(cmd->alias, argv[0]))) {
      ER_DEBUG("Found matching cmd '%s'", cmd->name);
      /* setup default arguments */
      for (size_t j = 0; j < MINIMON_CMD_MAX_ARGS; ++j) {
        if (!cmd->config[j].required) {
          args[j] = cmd->config[j].dflt;
        }
      } /* for(j..) */

      /* validate arguments */
      status_t status =
        mini_validate_args((uint32_t)argc, argv, cmd->config, args);

      /* call command handler */
      if (OK == status) {
        (*cmd->hook)(cmd->name, args[0], args[1], args[2], args[3]);
      }

      cmd_found = true;
      break;
    }
  }

  if (!cmd_found) {
    ER_ERR("Unknown command '%s'", argv[0]);
  }
} /* mini_dispatch() */

/*******************************************************************************
 * Private Functions
 ******************************************************************************/
/* NOLINTNEXTLINE(readability-function-cognitive-complexity) */
static void mini_cmd_help(const struct minimon_cmd* cmd,
                          enum minimon_help_type    type) {
  if (mini_str_empty(cmd->name)) {
    return;
  }
  stdio_printf("\r\n%s:\r\n\tSynopsis: %s.\r\n", cmd->name, cmd->help);

  stdio_printf("\tSyntax: %s", cmd->name);
  for (size_t j = 0; j < MINIMON_CMD_MAX_ARGS; ++j) {
    const struct minimon_cmd_param* param = &cmd->config[j];
    if (mini_str_empty(param->name)) {
      continue;
    }
    stdio_printf(" %c%s%c",
                 param->required ? '<' : '[',
                 param->name,
                 param->required ? '>' : ']');
  } /* for(j..) */
  stdio_printf("\r\n");

  if (!mini_str_empty(cmd->alias)) {
    stdio_printf("\t        %s", cmd->alias);
    for (size_t j = 0; j < MINIMON_CMD_MAX_ARGS; ++j) {
      const struct minimon_cmd_param* param = &cmd->config[j];
      if (mini_str_empty(param->name)) {
        continue;
      }
      stdio_printf(" %c%s%c",
                   param->required ? '<' : '[',
                   param->name,
                   param->required ? '>' : ']');
    } /* for(j..) */
    stdio_printf("\r\n");
  }
  if (MINIMON_HELP_SHORT == type) {
    return;
  }

  /* print each parameter's help for the cmd */
  for (size_t j = 0; j < MINIMON_CMD_MAX_ARGS; ++j) {
    const struct minimon_cmd_param* param = &cmd->config[j];
    if (mini_str_empty(param->name)) {
      continue;
    }
    stdio_printf("\t%c%s%c %s\r\n",
                 param->required ? '<' : '[',
                 param->name,
                 param->required ? '>' : ']',
                 param->long_help);
  } /* for(j..) */
} /* mini_cmd_help() */

static void mini_cmd_help_all(RCSW_UNUSED const char* cmdname, ...) {
  va_list args;

  va_start(args, cmdname);
  char* target = va_arg(args, char*);
  va_end(args);

  if (NULL == target) {
    stdio_printf("AVAILABLE COMMANDS:\r\n");

    for (size_t i = 0; i < mini_n_cmds(); ++i) {
      /* print the help for one cmd */
      mini_cmd_help(mini_cmd_get(i), MINIMON_HELP_SHORT);
    } /* for(i..) */

    stdio_printf(
      "\r\n"
      "All cmds are shown in lower case and all parameters are "
      "shown in UPPER case\r\n"
      "for clarity. Parameters in [] are OPTIONAL; parameters in <> "
      "are REQUIRED.\r\n\r\n"
      "To see detailed info about a specific cmd, do 'help CMD', "
      "where CMD is the cmd\r\nyou're interested in.\r\n\r\n");
  } else {
    for (size_t i = 0; i < mini_n_cmds(); ++i) {
      /* print the help for one cmd */
      const struct minimon_cmd* cmd = mini_cmd_get(i);
      if (!mini_str_empty(cmd->name) &&
          0 == stdio_strncmp(cmd->name, target, MINIMON_CMD_MAX_NAMELEN)) {
        mini_cmd_help(cmd, MINIMON_HELP_LONG);
      }
    } /* for(i..) */
  }
} /* mini_cmd_help() */

static void mini_cmd_read(RCSW_UNUSED const char* cmdname, ...) {
  va_list args;

  va_start(args, cmdname);
  uint32_t* addrp = va_arg(args, uint32_t*);
  uint32_t  size  = va_arg(args, uint32_t);
  va_end(args);

  for (uint32_t i = 0; i < size; i++) {
    if ((i % sizeof(uint32_t)) == 0) {
      /* show starting address for next set of 4 words */
      stdio_printf("\r\n0x%" PRIx64 ": ", (uintptr_t)(addrp + i));
    }

    /* do the actual read and display it */
    stdio_printf("0x%8X ", addrp[i]);
  }
} /* mini_cmd_read() */

static void mini_cmd_write(RCSW_UNUSED const char* cmdname, ...) {
  va_list args;

  va_start(args, cmdname);
  uint32_t* addrp = va_arg(args, uint32_t*);
  uint32_t  value = va_arg(args, uint32_t);
  uint32_t  size  = va_arg(args, uint32_t);
  va_end(args);

  for (uint32_t i = 0; i < size; ++i) {
    addrp[i] = value;
  }
} /* mini_cmd_write() */

static void mini_cmd_jump(RCSW_UNUSED const char* cmdname, ...) {
  va_list args;
  va_start(args, cmdname);
  vfp_t func = va_arg(args, vfp_t);
  va_end(args);

  /* mask and clear interrupts */
  if (NULL != g_minimon.irqcb.clear_all) {
    g_minimon.irqcb.clear_all();
  }
  if (NULL != g_minimon.irqcb.mask_all) {
    g_minimon.irqcb.mask_all();
  }

  /* branch */
  func();
  asm volatile("nop ; nop ; nop");
} /* mini_cmd_jump() */

static void mini_cmd_load(RCSW_UNUSED const char* cmdname, ...) {
  va_list args;

  va_start(args, cmdname);
  uint32_t* addrp = va_arg(args, uint32_t*);
  uint32_t  size  = va_arg(args, uint32_t);
  va_end(args);

  uint32_t       n_bytes_rx = 0;
  union word_buf hold_buf;
  hold_buf.word = 0;

  stdio_printf("Receiving %d bytes...", size);

  while (n_bytes_rx < size) {
    int c = mini_data_byte_get(&g_minimon);
    if (c < 0) {
      continue; /* no byte available yet */
    }
    hold_buf.byte[n_bytes_rx % sizeof(uint32_t)] = (uint8_t)c;

    /* write each word as it is formed */
    if (0 == ((n_bytes_rx + 1) % sizeof(uint32_t))) {
      addrp[n_bytes_rx / sizeof(uint32_t)] = hold_buf.word;
      hold_buf.word                        = 0;
    }
    ++n_bytes_rx;

    if (0 == n_bytes_rx % MINIMON_DIAG_INTERVAL) {
      stdio_printf("\rReceiving %d bytes...%d%%", size, n_bytes_rx * 100 / size);
    }
  }

  /* write last word */
  if (0 != (n_bytes_rx % sizeof(uint32_t))) {
    addrp[n_bytes_rx / sizeof(uint32_t)] = hold_buf.word;
  }
} /* mini_cmd_load() */

static void mini_cmd_send_nmea(uint32_t* addrp, uint32_t size) {
  uint32_t n_bytes_tx    = 0;
  uint32_t current_chunk = 0;
  char     tmp[20];

  const char* header = "$SEND";
  int     n = stdio_snprintf(tmp, sizeof(tmp), "%s,%d", header, current_chunk);
  uint8_t checksum = utils_xchks8((uint8_t*)tmp + 1, (size_t)(n - 1), 0);
  stdio_usfprintf(mini_data_byte_put, &g_minimon, "%s", tmp);

  while (n_bytes_tx < size) {
    if (0 == n_bytes_tx % MINIMON_DIAG_INTERVAL) {
      stdio_printf("\rSending %d bytes from %p using protocol 'nmea'...%d%%",
                   size,
                   addrp,
                   n_bytes_tx * 100 / size);
    }

    /* get next word */
    n        = stdio_snprintf(tmp, sizeof(tmp), ",%d", addrp[n_bytes_tx]);
    checksum = utils_xchks8((uint8_t*)tmp, (size_t)n, checksum);
    stdio_usfprintf(mini_data_byte_put, &g_minimon, "%s", tmp);

    /*
     * For this protocol, we send integer-sized chunks of stuff; this is the
     * count of DATA bytes sent, NOT the count of total bytes.
     */
    n_bytes_tx += 4;

    if (0 == n_bytes_tx % MINIMON_DIAG_INTERVAL) {
      stdio_printf("\rSending %d bytes from %p using protocol 'nmea'...%d%%",
                   size,
                   addrp,
                   n_bytes_tx * 100 / size);
    }

    /*
     * If we have reached the end of our allowable chunk size for data bytes,
     * end the sentence.
     */
    if (n_bytes_tx / MINIMON_NMEA_CHUNK_SIZE > current_chunk) {
      current_chunk = n_bytes_tx / MINIMON_NMEA_CHUNK_SIZE;
      stdio_usfprintf(mini_data_byte_put, &g_minimon, "*%X", checksum);

      /*
       * Still have more bytes to transmit--reset checksum and send
       * header+current chunk again.
       */
      if (n_bytes_tx < size) {
        int n1 = stdio_snprintf(tmp, sizeof(tmp), "%s", header);
        int n2 = stdio_snprintf(tmp + n1,
                                sizeof(tmp) - (size_t)n1,
                                ",%d",
                                current_chunk);
        checksum =
          utils_xchks8((uint8_t*)tmp + 1, (size_t)(n1 + n2 - 1), checksum);
        stdio_usfprintf(mini_data_byte_put, &g_minimon, "%s", tmp);
      }
    }
  } /* while(...) */
}

static void mini_cmd_send_raw(uint32_t* addrp, uint32_t size) {
  uint32_t       n_bytes_tx = 0;
  union word_buf hold_buf;

  while (n_bytes_tx < size) {
    if (0 == n_bytes_tx % MINIMON_DIAG_INTERVAL) {
      stdio_printf("\rSending %d bytes from %p, using protocol 'raw'...%d%%",
                   size,
                   addrp,
                   n_bytes_tx * 100 / size);
    }

    /* get next word */
    hold_buf.word = addrp[n_bytes_tx / sizeof(uint32_t)];
    for (size_t i = 0; i < sizeof(uint32_t); ++i) {
      mini_data_byte_put((char)hold_buf.byte[i], &g_minimon);

      /*
       * Each actual byte maps 1->1 to the bytes actually send, so this is
       * in the transmission loop.
       */
      ++n_bytes_tx;
    } /* for(i...) */
  } /* while(...) */
}

static void mini_cmd_send(RCSW_UNUSED const char* cmdname, ...) {
  va_list args;

  va_start(args, cmdname);
  uint32_t* addrp     = va_arg(args, uint32_t*);
  uint32_t  size      = va_arg(args, uint32_t);
  char*     protocolp = va_arg(args, char*);
  va_end(args);

  if (stdio_strncmp(protocolp, "raw", 3) == 0) {
    mini_cmd_send_raw(addrp, size);
  } else if (stdio_strncmp(protocolp, "nmea", 4) == 0) {
    mini_cmd_send_nmea(addrp, size);
  } else {
    ER_ERR("Bad protocol '%s': must be [raw,nmea]", protocolp);
  }
} /* mini_cmd_send() */

/*******************************************************************************
 * Public API
 ******************************************************************************/
void minimon_init(const struct minimon_config* config) {
  static const struct minimon_config defaults = {
    .cmds            = NULL,
    .n_cmds          = 0,
    .include_builtin = true,
    .help_on_start   = true,
  };
  if (NULL == config) {
    config = &defaults;
  }

  /*
   * Nothing is copied: the built-ins are a static table, and the caller's
   * table is used in place. HELP (the first built-in) is always available.
   */
  g_minimon.n_builtin_cmds =
    config->include_builtin ? RCSW_ARRAY_ELTS(g_minimon_builtin_cmds) : 1;
  g_minimon.user_cmds   = config->cmds;
  g_minimon.n_user_cmds = (NULL != config->cmds) ? config->n_cmds : 0;

  g_minimon.irqcb           = config->irqcb;
  g_minimon.stream1.putchar = config->stream1.putchar;
  g_minimon.stream1.getchar = config->stream1.getchar;

  g_minimon.stream0.putchar = stdio_putchar;
  g_minimon.stream0.getchar = stdio_getchar;
  g_minimon.help_on_start   = config->help_on_start;
} /* minimon_init() */

void minimon_start(void) {
  int   argc = 0;                       /* number of tokens in command */
  char* argv[MINIMON_CMD_MAX_ARGS + 1]; /* list of tokens */
  char  cmdline[64];

  stdio_memset(cmdline, 0, sizeof(cmdline));

  /* emit version info */
  char buf[512];
  stdio_snprintf(buf,
                 sizeof(buf),
                 "\r\n-----------------------------------------------------------"
                 "---------------------\r\n"
                 "This is MINIMON, %s.\r\n"
                 "GIT_REV=%s\r\n"
                 "GIT_DIFF=%s\r\n"
                 "GIT_TAG=%s\r\n"
                 "GIT_BRANCH=%s\r\n"
                 "CFLAGS=%s\r\n"
                 "BUILD DATE=%s\r\n"
                 "BUILD TIME=%s\r\n"
                 "---------------------------------------------------------------"
                 "-----------------\r\n",
                 rcsw_version(),
                 rcsw_build_git_rev(),
                 rcsw_build_git_diff(),
                 rcsw_build_git_tag(),
                 rcsw_build_git_branch(),
                 rcsw_build_compile_flags(),
                 rcsw_build_date(),
                 rcsw_build_time());
  stdio_puts(buf);

  /* display the monitor tag line */
  if (g_minimon.help_on_start) {
    mini_cmd_help_all("help", NULL);
  } else {
    stdio_printf("Type 'help' for a list of available cmds\r\n");
  }
  stdio_printf("-> ");

  for (;;) {
    cmdline[0] = '\0';
    mini_readline(cmdline, sizeof(cmdline));
    /*
     * Advance to the next line in the serial terminal
     * for clarity.
     */
    stdio_printf("\r\n");

    /* If the line isn't empty, parse it and use it */
    if ('\0' != cmdline[0]) {
      ER_DEBUG("Parsing cmd");
      argc = (int)mini_parse(cmdline, argv);

      ER_DEBUG("Attempting dispatch: argc=%d,argv[0]=%s", argc, argv[0]);
      mini_dispatch(argc, argv);
    }

    /* display the prompt */
    stdio_printf("\r\n-> ");
  } /* for(;;) */
} /* minimon_start() */
