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

/** \brief Words of data per NMEA packet. */
#define MINIMON_NMEA_WORDS_PER_PKT (MINIMON_NMEA_CHUNK_SIZE / sizeof(uint32_t))

/** \brief Words shown per line by the \c read command. */
#define MINIMON_READ_WORDS_PER_LINE 4

static struct minimon g_minimon;

/*******************************************************************************
 * Private API
 ******************************************************************************/
/**
 * \brief Displays the help message for minimon showing valid commands and what
 * the syntax for each is.
 */
static void mini_cmd_help_all(const char* cmdname,
                              const union minimon_cmd_arg* args);

/**
 * \brief Read 1 or more words of memory, starting at the specified address. The
 * contents of memory are printed to the serial terminal. The address is not
 * checked for alignment.
 */
static void mini_cmd_read(const char* cmdname,
                          const union minimon_cmd_arg* args);

/**
 * \brief Writes 1 or more words of memory, starting at the specified
 * address, with the specified value. All words are written with the same value.

 ** For large writes, the progress is printed to the serial terminal. The
 * starting address is not checked for word-alignment.
 */
static void mini_cmd_write(const char* cmdname,
                           const union minimon_cmd_arg* args);

/**
 * \brief Jump to another memory location and begin execution there. The
 * validity of the address to jump to is not checked.
 *
 * This function only applies to the CPU that minimon is running on;
 * multiprocessing jumps not supported.
 */
static void mini_cmd_jump(const char* cmdname,
                          const union minimon_cmd_arg* args);

/**
 * \brief Receives a program byte by byte through the serial terminal
 * and then writes it to the specified memory location.
 */
static void mini_cmd_load(const char* cmdname,
                          const union minimon_cmd_arg* args);

/**
 * \brief Send bytes from a specified memory location through the serial
 * terminal.
 */
static void mini_cmd_send(const char* cmdname,
                          const union minimon_cmd_arg* args);

/*******************************************************************************
 * Private Functions
 ******************************************************************************/
/**
 * \brief Unused command/parameter table entries have NULL or empty names.
 */
static bool_t mini_str_empty(const char* s) { return NULL == s || '\0' == *s; }

/** \brief Case-insensitive string equality. */
static bool_t mini_streq_nocase(const char* a, const char* b) {
  for (; *a != '\0' && *b != '\0'; ++a, ++b) {
    if (stdio_tolower(*a) != stdio_tolower(*b)) {
      return false;
    }
  } /* for(..) */
  return *a == *b;
}

/**
 * \brief Whether progress messages may be printed during a transfer.
 *
 * Only when the transfer's data uses stream1: on stream0 they would be mixed
 * into the data (see \ref minimon_config.stream1).
 *
 * \param tx true for a transfer to the remote target (send), false for one
 *           from it (load).
 */
static bool_t mini_progress_on(bool_t tx) {
  return tx ? (NULL != g_minimon.stream1.putchar)
            : (NULL != g_minimon.stream1.getchar);
}

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
   .help =
     "Load bytes from the remote target into memory, via stream1 if set (else "
     "stdio_getchar())",
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
   .help  = "Send memory to the remote target, via stream1 if set (else "
            "stdio_putchar())",
   .hook  = mini_cmd_send,
   .config =
     {{
        .name       = "ADDR",
        .type       = MINIMON_PARAM_UINT32,
        .short_help = NULL,
        .long_help =
          "The 32-bit source address in memory (alignment not checked)",
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
        .short_help = "The protocol to use [raw, nmea] (any case)",
        .long_help =
          "The protocol to use (any case). Options are:\r\n\r\n"
          "\t\traw - Exactly SIZE bytes of the source buffer.\r\n"

          "\t\tNMEA - The source buffer as 32-bit words (SIZE rounded up to\r\n"
          "\t\t       whole words), in ASCII packets of " RCSW_XSTR(
            MINIMON_NMEA_CHUNK_SIZE) " bytes:\r\n"
          "\t\t\r\n"
          "\t\t       $SEND,ID,X,Y,Z,...*CC\r\n"
          "\t\t\r\n"
          "\t\t       ID is the 0-based packet index. X,Y,Z,... are the\r\n"
          "\t\t       words in unsigned decimal. CC is a one byte rotating\r\n"
          "\t\t       XOR checksum of the characters between '$' and '*',\r\n"
          "\t\t       in two hex digits. See docs for more details.\r\n"
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
  const char* digits = numstr;
  bool_t      is_hex = false;

  if (stdio_strncmp(numstr, "0X", 2) == 0 || stdio_strncmp(numstr, "0x", 2) == 0) {
    is_hex = true;
    digits += 2;
  }
  size_t length     = stdio_strlen(digits);
  size_t max_digits = is_hex ? 8 : 10;
  if (length == 0 || length > max_digits) {
    ER_ERR("'%s': expected 1-%zu %s digits",
               numstr,
               max_digits,
               is_hex ? "hex" : "decimal");
    return ERROR;
  }

  /* check string contents for characters not valid in this base */
  for (size_t i = 0; i < length; i++) {
    bool_t ok = is_hex ? RCSW_STDIO_ISHEX(digits[i]) : RCSW_STDIO_ISDIGIT(digits[i]);
    if (!ok) {
      ER_ERR("'%s': invalid character '%c'", numstr, digits[i]);
      return ERROR;
    }
  } /* for(i..) */
  if (!is_hex && length == 10 && stdio_strcmp(digits, "4294967295") > 0) {
    ER_ERR("'%s': larger than 32 bits", numstr);
    return ERROR;
  }
  ER_TRACE("Token '%s' is %s", numstr, is_hex ? "hexadecimal" : "decimal");

  /* stdio_atoi() accumulates unsigned, so all 32 bits survive the cast */
  *num = (uint32_t)stdio_atoi(digits, is_hex ? 16 : 10);
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
      ER_ERR("Invalid argument %" PRIu32 ": '%s'", (i + 1), argv[i + 1]);
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
  /* skip leading delimiters, so argv[0] is the command name */
  while (buffer[i] == ' ' || buffer[i] == ',') {
    i++;
  }
  argv[j++] = &buffer[i]; /* argv[0] is cmd name */

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
        cmd->hook(cmd->name, args);
      }

      cmd_found = true;
      break;
    }
  }

  if (!cmd_found) {
    ER_ERR("Unknown command '%s'--type 'help' for a list", argv[0]);
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

static void mini_cmd_help_all(RCSW_UNUSED const char*         cmdname,
                              const union minimon_cmd_arg* args) {
  const char* target = args[0].s;

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

static void mini_cmd_read(RCSW_UNUSED const char*         cmdname,
                          const union minimon_cmd_arg* args) {
  uint32_t addr = args[0].num;
  uint32_t size = args[1].num;

  /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
  const volatile uint32_t* addrp = (const volatile uint32_t*)(uintptr_t)addr;
  for (uint32_t i = 0; i < size; i++) {
    if ((i % MINIMON_READ_WORDS_PER_LINE) == 0) {
      /* show starting address for the next line of words */
      stdio_printf("\r\n0x%08" PRIX32 ": ", addr + (i * (uint32_t)sizeof(uint32_t)));
    }

    /* do the actual read and display it */
    stdio_printf("0x%08" PRIX32 " ", addrp[i]);
  }
} /* mini_cmd_read() */

static void mini_cmd_write(RCSW_UNUSED const char*         cmdname,
                           const union minimon_cmd_arg* args) {
  uint32_t addr  = args[0].num;
  uint32_t value = args[1].num;
  uint32_t size  = args[2].num;

  /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
  volatile uint32_t* addrp = (volatile uint32_t*)(uintptr_t)addr;
  for (uint32_t i = 0; i < size; ++i) {
    addrp[i] = value;
  }
} /* mini_cmd_write() */

static void mini_cmd_jump(RCSW_UNUSED const char*         cmdname,
                          const union minimon_cmd_arg* args) {
  uint32_t addr = args[0].num;

  /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
  vfp_t func = (vfp_t)(uintptr_t)addr;

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

static void mini_cmd_load(RCSW_UNUSED const char*         cmdname,
                          const union minimon_cmd_arg* args) {
  uint32_t addr = args[0].num;
  uint32_t size = args[1].num;

  /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
  volatile uint32_t* addrp      = (volatile uint32_t*)(uintptr_t)addr;
  bool_t             progress   = mini_progress_on(false);
  uint32_t           n_bytes_rx = 0;
  union word_buf     hold_buf;
  hold_buf.word = 0;

  if (progress) {
    stdio_printf("Receiving %" PRIu32 " bytes...", size);
  }

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

    if (progress && 0 == n_bytes_rx % MINIMON_DIAG_INTERVAL) {
      stdio_printf("\rReceiving %" PRIu32 " bytes...%" PRIu32 "%%",
                   size,
                   (uint32_t)((uint64_t)n_bytes_rx * 100 / size));
    }
  }

  /* write last word */
  if (0 != (n_bytes_rx % sizeof(uint32_t))) {
    addrp[n_bytes_rx / sizeof(uint32_t)] = hold_buf.word;
  }
} /* mini_cmd_load() */

/** \brief Emit a string on the data stream, folding it into a checksum. */
static uint8_t mini_nmea_emit(const char* str, size_t len, uint8_t checksum) {
  stdio_usfprintf(mini_data_byte_put, &g_minimon, "%s", str);
  return utils_xchks8((const uint8_t*)str, len, checksum);
}

static void mini_cmd_send_nmea(const volatile uint32_t* addrp,
                               uint32_t                 size,
                               bool_t                   progress) {
  uint32_t n_words = (size + (uint32_t)sizeof(uint32_t) - 1) / sizeof(uint32_t);
  char     tmp[24];

  /*
   * One packet per MINIMON_NMEA_CHUNK_SIZE bytes: $SEND,ID,X,Y,...*CC\r\n,
   * with a fresh checksum over the characters between '$' and '*'.
   */
  for (uint32_t pkt = 0; pkt * MINIMON_NMEA_WORDS_PER_PKT < n_words; ++pkt) {
    if (progress) {
      stdio_printf("\rSending %" PRIu32 " bytes using protocol 'nmea'...%" PRIu32
                   "%%",
                   size,
                   (uint32_t)((uint64_t)pkt * MINIMON_NMEA_CHUNK_SIZE * 100 /
                              size));
    }
    mini_data_byte_put('$', &g_minimon);
    int     n        = stdio_snprintf(tmp, sizeof(tmp), "SEND,%" PRIu32, pkt);
    uint8_t checksum = mini_nmea_emit(tmp, (size_t)n, 0);

    uint32_t first = pkt * (uint32_t)MINIMON_NMEA_WORDS_PER_PKT;
    for (uint32_t w = first;
         w < n_words && w < first + MINIMON_NMEA_WORDS_PER_PKT;
         ++w) {
      n        = stdio_snprintf(tmp, sizeof(tmp), ",%" PRIu32, addrp[w]);
      checksum = mini_nmea_emit(tmp, (size_t)n, checksum);
    } /* for(w..) */

    stdio_snprintf(tmp, sizeof(tmp), "*%02X\r\n", checksum);
    stdio_usfprintf(mini_data_byte_put, &g_minimon, "%s", tmp);
  } /* for(pkt..) */
}

static void mini_cmd_send_raw(const volatile uint32_t* addrp,
                              uint32_t                 size,
                              bool_t                   progress) {
  uint32_t       n_bytes_tx = 0;
  union word_buf hold_buf;

  while (n_bytes_tx < size) {
    if (progress && 0 == n_bytes_tx % MINIMON_DIAG_INTERVAL) {
      stdio_printf("\rSending %" PRIu32 " bytes using protocol 'raw'...%" PRIu32
                   "%%",
                   size,
                   (uint32_t)((uint64_t)n_bytes_tx * 100 / size));
    }

    /*
     * Memory is read a whole word at a time (some targets only allow word
     * access), but only the first SIZE bytes are sent.
     */
    hold_buf.word = addrp[n_bytes_tx / sizeof(uint32_t)];
    for (size_t i = 0; i < sizeof(uint32_t) && n_bytes_tx < size; ++i) {
      mini_data_byte_put((char)hold_buf.byte[i], &g_minimon);
      ++n_bytes_tx;
    } /* for(i...) */
  } /* while(...) */
}

static void mini_cmd_send(RCSW_UNUSED const char*         cmdname,
                          const union minimon_cmd_arg* args) {
  uint32_t    addr      = args[0].num;
  uint32_t    size      = args[1].num;
  const char* protocolp = args[2].s;

  /* NOLINTNEXTLINE(performance-no-int-to-ptr) */
  const volatile uint32_t* addrp = (const volatile uint32_t*)(uintptr_t)addr;
  bool_t                   progress = mini_progress_on(true);
  if (mini_streq_nocase(protocolp, "raw")) {
    mini_cmd_send_raw(addrp, size, progress);
  } else if (mini_streq_nocase(protocolp, "nmea")) {
    mini_cmd_send_nmea(addrp, size, progress);
  } else {
    ER_ERR("Bad protocol '%s': must be raw or nmea", protocolp);
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
    const union minimon_cmd_arg no_args[MINIMON_CMD_MAX_ARGS] = {{.s = NULL}};
    mini_cmd_help_all("help", no_args);
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
