/**
 * \file minimon.c
 *
 * \copyright 2023 John Harwell, All rights reserved.
 *
 * SPDX-License Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/tool/minimon.h"

#include "rcsw/stdio/printf.h"
#include "rcsw/stdio/string.h"
#include "rcsw/stdio/stdio.h"
#include "rcsw/er/er.h"
#include "rcsw/er/client.h"
#include "rcsw/utils/checksum.h"
#include "rcsw/version/version.h"

/*******************************************************************************
 * Constant Definitions
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
 * Type Definitions
 ******************************************************************************/
/**
 * \brief Convenience buffer for sending out bytes from a \p uint32_t pointer.
 */
union word_buf {
  uint8_t  byte[sizeof(uint32_t)];
  uint32_t word;
};

/*******************************************************************************
 * Private Command Functions
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
static void mini_data_byte_put(int c, void* extra) {
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
  } else {
    return stdio_getchar();
  }
}

/*******************************************************************************
 * Global Variables
 ******************************************************************************/
static struct minimon_cmd g_minimon_builtin_cmds[] = {
  {
    .name = "help",
    .alias = "h",
    .help = "Print help for all cmds or a specific cmd",
    .hook = mini_cmd_help_all,
    .params = {
      {
        .name = "CMD",
        .type = ekMINIMON_PARAM_STR,
        .short_help = NULL,
        .long_help = "The specific cmd to see detailed help for",
        .required = false,
        .dflt = {.s = NULL}
      }
      },
  },
  {
    .name = "read",
    .alias = "r",
    .help = "Read from memory",
    .hook = mini_cmd_read,
    .params = {
      {
        .name = "ADDR",
        .type = ekMINIMON_PARAM_UINT32,
        .short_help = NULL,
        .long_help = "The 32-bit address in memory (alignment not checked)",
        .required = true,
      },
      {
        .name = "SIZE",
        .type = ekMINIMON_PARAM_UINT32,
        .short_help = NULL,
        .long_help = "The number of 32-bit words to read",
        .required = false,
        .dflt = {.num = 1}
      }
    }
  },
  {
    .name = "load",
    .alias = "l",
    .help = "Load bytes via stdio_getchar() from the remote target into memory",
    .hook = mini_cmd_load,
    .params = {
      {
        .name = "ADDR",
        .type = ekMINIMON_PARAM_UINT32,
        .short_help = NULL,
        .long_help = "The 32-bit dest address in memory (alignment not checked)",
        .required = true,
      },
      {
        .name = "SIZE",
        .type = ekMINIMON_PARAM_UINT32,
        .short_help = NULL,
        .long_help = "The number of bytes to receive",
        .required = true,
      }
    }
  },
  {
    .name = "send",
    .alias = "s",
    .help = "Send from memory via stdio_getchar() to the remote target",
    .hook = mini_cmd_send,
    .params = {
      {
        .name = "ADDR",
        .type = ekMINIMON_PARAM_UINT32,
        .short_help = NULL,
        .long_help = "The 32-bite source address in memory (alignment not checked)",
        .required = true,
      },
      {
        .name = "SIZE",
        .type = ekMINIMON_PARAM_UINT32,
        .short_help = NULL,
        .long_help = "The number of bytes to send",
        .required = true,
      },
      {
        .name = "PROTOCOL",
        .type = ekMINIMON_PARAM_STR,
        .short_help = "The protocol to use [raw, ascii, NMEA]",
        .long_help = "The protocol to use. Options are:\r\n\r\n"
        "\t\traw - What is sent is exactly what is in the source buffer.\r\n"

        "\t\tascii - What is sent is first converted to ASCII before sending;\r\n"
        "\t\t        requires de-modulating via a script of some sort on the other\r\n"
        "\t\t        before the actual data can be used. Useful when you are\r\n"
        "\t\t        delivering code to someone who is using a strange serial\r\n"
        "\t\t        terminal, so that the terminal doesn't receive the raw\r\n"
        "\t\t        for an escape sequence and do something strange.\r\n"
        "\t\t        See docs for more details.\r\n"

        "\t\tNMEA - What is sent is first converted to ASCII, then placed\r\n"
        "\t\t       in simple text packets. Each packet contains " RCSW_XSTR(MINIMON_NMEA_CHUNK_SIZE) " bytes of data.\r\n"
        "\t\t       Packets have the following form:\r\n"
        "\t\t\r\n"
        "\t\t       $SEND,ID,X,Y,Z,...*C\r\n"
        "\t\t\r\n"
        "\t\t       ID is the 0-based index of the chunk of data in the packet.\r\n"
        "\t\t       X,Y,Z,... are the ASCII representations of 32-bit\r\n"
        "\t\t       WORDS (not bytes) of data, followed by a '*'. C is a one byte\r\n"
        "\t\t       rotating XOR checksum in hex. See docs for more details.\r\n"
        "\t\t\r\n"
        ,
        .required = true,
      }
    }
  },
  {
    .name = "write",
    .alias = "w",
    .help = "Write to memory",
    .hook = mini_cmd_write,
    .params = {
      {
        .name = "ADDR",
        .type = ekMINIMON_PARAM_UINT32,
        .short_help = NULL,
        .long_help = "The 32-bit address in memory (alignment not checked)",
        .required = true,
      },
      {
        .name = "VALUE",
        .type = ekMINIMON_PARAM_UINT32,
        .short_help = NULL,
        .long_help = "The 32-bit value to write",
        .required = true,
      },
      {
        .name = "SIZE",
        .type = ekMINIMON_PARAM_UINT32,
        .short_help = NULL,
        .long_help = "The number of 32-bit values to write",
        .required = false,
        .dflt = {.num = 1}
      }
    }
  },
  {
    .name = "jump",
    .alias = "j",
    .help = "Jump to a new execution address, masking/clearing all IRQs beforehand",
    .hook = mini_cmd_jump,
    .params = {
      {
        .name = "ADDR",
        .type = ekMINIMON_PARAM_UINT32,
        .short_help = NULL,
        .long_help = "The address in memory (validity not checked)",
        .required = true,
      },
    }
  },
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
static status_t mini_validate_int(char *numstr, uint32_t* num) {
  uint32_t length;
  uint32_t is_hex = 0;

  if (stdio_strncmp( "0X", numstr, 2) == 0) {
    is_hex = 1;
    numstr += 2;
  }

  length = stdio_strlen(numstr);

  /* validate string length */
  if ( (length == 0) || (length > 10) || (is_hex && (length > 8)) ) {
    return ERROR;
  }

  /* check string contents for illegal characters */
  for (uint32_t i = 0; i < length; i++ ) {
    if (!RCSW_STDIO_ISHEX(numstr[i])) {
      return ERROR;
    }
  }

  if (is_hex) {
    numstr-=2;
    *num = (uint32_t)stdio_atoi(numstr,16);
  } else {
    *num = (uint32_t)stdio_atoi(numstr,10);
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
 * \param definition Parameter definition containing how many, what type,
 *                   etc. of params are required.
 *
 * \param params Parameter array with \a effective parameters; defaults already
 *               copied in.
 */
static status_t mini_validate_args(uint32_t argc,
                                   char **argv,
                                   struct minimon_cmd_param* params,
                                   union minimon_cmd_arg* args) {
  status_t rstat = OK;

  size_t min_params = 0;
  size_t max_params = 0;

  for (uint32_t i = 0; i < MINIMON_CMD_MAX_ARGS; ++i) {
    if (stdio_strlen(params[i].name) > 0) {
      ++max_params;

      if (params[i].required) {
        ++min_params;
      }
    }
  } /* for(i..) */



  /* validate parameter count */
  if ((argc - 1) < min_params) {
    ER_ERR("Too few arguments");
    return ERROR;
  }
  if ((argc - 1) > max_params) {
    ER_ERR("Too many arguments");
    return ERROR;
  }


  /* validate arguments */
  for (uint32_t i = 0; i < argc - 1; i++) {
    /* validate numerics */
    switch (params[i].type) {
      case ekMINIMON_PARAM_UINT32:
        rstat |= mini_validate_int(argv[i+1], &args[i].num);
        break;
      case ekMINIMON_PARAM_STR:
        args[i].s = argv[i+1];
      default:
        break;
    } /* switch() */

    if (OK != rstat) {
      ER_ERR("Invalid arg[%d] = '%s'", (i+1), argv[i+1]);
    }
  }

  return rstat;
} /* mini_validate_args() */

/**
 * \brief Read a line from the serial terminal.
 *
 * Lines are delimited by newlines or carriage returns. Backspacing is
 * supported.
 *
 * \param buf To fill with read-in line
 * \param size maximum size of line to be read, less the '\0'
 */
static void mini_readline(char *buf, int  size) {
  char c;
  int i = 0;

  /* read characters until newline is read or the buffer is filled */
  do {
    c = stdio_getchar();
    if (c == '\0') {
      continue;
    }
    /* check for backspace */
    if ((c == '\b') &&  (i > 0)) {
      stdio_printf("\b \b");
      i--;
      continue;
    }
    else if ((c == '\n') || (c == '\r')) {
      break;
    }
    buf[i++] = c;

    /* echo the character back to serial terminal */
    stdio_putchar(c);

  } while (i < (size-2));
  buf[i] = '\0';
} /* mini_readline() */


/**
 * \brief Parse the command string into cmd + a set of arguments.
 *
 * \return The number of delimited arguments found + 1.
 */
static uint32_t mini_parse(char *buffer, char **argv) {
  uint32_t i = 0;                 /* string char index */
  uint32_t j = 0;                 /* the number of arguments parsed */
  bool_t last_was_space = false;  /* was last char a delimiter? */

  /* argv[0] is cmd name*/
  argv[j++] = &buffer[0];

  while (buffer[i]) {
    if ((buffer[i] == ' ') || (buffer[i] == ',')) {
      buffer[i++] = 0;
      last_was_space = 1;
    } else if (last_was_space) {
      argv[j++] = &buffer[i++];
      last_was_space = 0;
    } else {
      i++;
    }
  }
  argv[j] = "";
  return j;
} /* mini_parse() */

/**
 * \brief Parse, validate, and execute the command read in from the serial
 *        terminal.
 */
static void mini_dispatch(int argc, char **argv) {
  bool_t cmd_found = false;
  union minimon_cmd_arg args[MINIMON_CMD_MAX_ARGS];

  if (*argv[0] == '\0' ) { /* ignore null commands */
    return;
  }

  /* select a command from dispatch list and call the corresponding handler. */
  for (size_t i = 0; i < MINIMON_MAX_CMDS; ++i) {
    struct minimon_cmd* cmd = &g_minimon.cmds[i];
    if (0 == stdio_strlen(cmd->name)) {
      break;
    }

    if (!stdio_strncmp(cmd->name, argv[0], MINIMON_CMD_MAX_NAMELEN) ) {
      ER_DEBUG("Found matching cmd '%s'", cmd->name);
      /* setup default arguments */
      for (size_t j = 0; j < MINIMON_CMD_MAX_ARGS; ++j) {
        if (!cmd->params[j].required) {
          args[i] = cmd->params[j].dflt;
        }
      } /* for(j..) */

      /* validate arguments */
      status_t status = mini_validate_args(argc, argv, cmd->params, args);

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
static void mini_cmd_help(const struct minimon_cmd* cmd,
                          enum minimon_help_type type) {
  if (0 == stdio_strlen(cmd->name)) {
      return;
    }
    stdio_printf("\r\n%s:\r\n\tSynopsis: %s.\r\n", cmd->name, cmd->help);

    stdio_printf("\tSyntax: %s", cmd->name);
    for (size_t j = 0; j < MINIMON_CMD_MAX_ARGS; ++j) {
      const struct minimon_cmd_param* param = &cmd->params[j];
      if (0 == stdio_strlen(param->name)) {
        continue;
      }
      stdio_printf(" %c%s%c",
                   param->required ? '<' : '[',
                   param->name,
                   param->required ? '>' : ']');
    } /* for(j..) */
    stdio_printf("\r\n");

    stdio_printf("\t        %s", cmd->alias);
    for (size_t j = 0; j < MINIMON_CMD_MAX_ARGS; ++j) {
      const struct minimon_cmd_param* param = &cmd->params[j];
      if (0 == stdio_strlen(param->name)) {
        continue;
      }
      stdio_printf(" %c%s%c",
                   param->required ? '<' : '[',
                   param->name,
                   param->required ? '>' : ']');
    } /* for(j..) */
    stdio_printf("\r\n");
    if (ekMINIMON_HELP_SHORT == type) {
      return;
    }

    /* print each parameter's help for the cmd */
    for (size_t j = 0; j < MINIMON_CMD_MAX_ARGS; ++j) {
      const struct minimon_cmd_param* param = &cmd->params[j];
      if (0 == stdio_strlen(param->name)) {
        continue;
      }
      stdio_printf("\t%c%s%c %s\r\n",
                   param->required ? '<' : '[',
                   param->name,
                   param->required ? '>' : ']',
                   param->long_help);
    } /* for(j..) */
} /* mini_cmd_help() */

static void mini_cmd_help_all(const char* cmdname, ...) {
  va_list args;

  va_start(args, cmdname);
  char* target = va_arg(args, char*);
  va_end(args);

  if (NULL == target) {
    stdio_printf("\r\n--------------------------------------------------------------------------------\r\n"
                 "This is MINIMON, %s.\r\n"
                 "GIT_REV=%s\r\n"
                 "GIT_DIFF=%s\r\n"
                 "GIT_TAG=%s\r\n"
                 "GIT_BRANCH=%s\r\n"
                 "CFLAGS=%s\r\n"
                 "--------------------------------------------------------------------------------\r\n"
                 "AVAILABLE COMMANDS:\r\n",
                 kMetadata.version.version,
                 kMetadata.build.git_rev,
                 kMetadata.build.git_diff,
                 kMetadata.build.git_tag,
                 kMetadata.build.git_branch,
                 kMetadata.build.compiler_flags);

    for (size_t i = 0; i < MINIMON_MAX_CMDS; ++i) {
      /* print the help for one cmd */
      struct minimon_cmd* cmd = &g_minimon.cmds[i];
      mini_cmd_help(cmd, ekMINIMON_HELP_SHORT);
    } /* for(i..) */

    stdio_printf("\r\nAll cmds are shown in lower case and all parameters are "
                 "shown in UPPER case\r\n"
                 "for clarity. Parameters in [] are OPTIONAL; parameters in <> "
                 "are REQUIRED.\r\n\r\n"
                 "To see detailed info about a specific cmd, do 'help CMD', where CMD"
                 "is the cmd\r\nyou're interested in.\r\n\r\n");
  } else {
    for (size_t i = 0; i < MINIMON_MAX_CMDS; ++i) {
      /* print the help for one cmd */
      struct minimon_cmd* cmd = &g_minimon.cmds[i];
      if (stdio_strlen(cmd->name) > 0 &&
          0 == stdio_strncmp(cmd->name, target, MINIMON_CMD_MAX_NAMELEN)) {
        mini_cmd_help(cmd, ekMINIMON_HELP_LONG);
      }
    } /* for(i..) */
  }
} /* mini_cmd_help() */

static void mini_cmd_read(const char* cmdname, ...) {
  va_list args;

  va_start(args, cmdname);
  uint32_t* addrp = va_arg(args, uint32_t*);
  uint32_t size = va_arg(args, uint32_t);
  va_end(args);


  for (uint32_t i = 0; i < size; i++) {
    if ((i % sizeof(uint32_t)) == 0) {
      /* show starting address for next set of 4 words */
      stdio_printf("\r\n%#8lX: ", (uintptr_t)(addrp + i));
    }

    /* do the actual read and display it */
    stdio_printf("%#8X ", addrp[i]);
  }
} /* mini_cmd_read() */

static void mini_cmd_write(const char* cmdname, ...) {
  va_list args;

  va_start(args, cmdname);
  uint32_t* addrp = va_arg(args, uint32_t*);
  uint32_t value = va_arg(args, uint32_t);
  uint32_t size = va_arg(args, uint32_t);
  va_end(args);

  for (uint32_t i = 0; i < size; ++i) {
    addrp[i] = value;
  }
} /* mini_cmd_write() */

static void mini_cmd_jump(const char* cmdname, ...) {
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
  asm volatile ( "nop ; nop ; nop" );
} /* mini_cmd_jump() */

static void mini_cmd_load(const char* cmdname, ...) {
  va_list args;

  va_start(args, cmdname);
  uint32_t* addrp = va_arg(args, uint32_t*);
  uint32_t size = va_arg(args, uint32_t);
  va_end(args);

  uint32_t n_bytes_rx = 0;
  union word_buf hold_buf;
  hold_buf.word = 0;

  stdio_printf("Receiving %d bytes...", size);

  while (n_bytes_rx < size) {
    hold_buf.byte[n_bytes_rx % sizeof(uint32_t)] = mini_data_byte_get(&g_minimon);

    /* write each word as it is formed */
    if (0 == ((n_bytes_rx+1) % sizeof(uint32_t))) {
      addrp[n_bytes_rx/sizeof(uint32_t)] = hold_buf.word;
      hold_buf.word = 0;
    }
    ++n_bytes_rx;

    if (0 == n_bytes_rx % MINIMON_DIAG_INTERVAL) {
      stdio_printf("\rReceiving %d bytes...%d%%",
                   size,
                   n_bytes_rx * 100 / size);

    }
  }

  /* write last word */
  if (0 != (n_bytes_rx % sizeof(uint32_t))) {
    addrp[n_bytes_rx/sizeof(uint32_t)] = hold_buf.word;
  }
} /* mini_cmd_load() */

static void mini_cmd_send_nmea(uint32_t* addrp, uint32_t size) {
  uint32_t n_bytes_tx = 0;
  uint32_t current_chunk = 0;
  char tmp[20];

  const char* header = "$SEND";
  int n = stdio_snprintf(tmp, sizeof(tmp), "%s,%d", header, current_chunk);
  uint8_t checksum = xchks8((uint8_t*)tmp + 1, n - 1, 0);
  stdio_usfprintf(mini_data_byte_put, &g_minimon, "%s", tmp);

  while (n_bytes_tx < size) {
    if (0 == n_bytes_tx % MINIMON_DIAG_INTERVAL) {
      stdio_printf("\rSending %d bytes from %p using protocol 'nmea'...%d%%",
                   size,
                   addrp,
                   n_bytes_tx * 100 / size);

    }

    /* get next word */
    n = stdio_snprintf(tmp, sizeof(tmp), ",%d", addrp[n_bytes_tx]);
    checksum = xchks8((uint8_t*)tmp, n, checksum);
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
        int n2 = stdio_snprintf(tmp + n1, sizeof(tmp) - n1, ",%d", current_chunk);
        checksum = xchks8((uint8_t*)tmp + 1, n1 + n2 - 1, checksum);
        stdio_usfprintf(mini_data_byte_put, &g_minimon, "%s", tmp);
      }
    }
  } /* while(...) */
}

static void mini_cmd_send_raw(uint32_t* addrp, uint32_t size) {
  uint32_t n_bytes_tx = 0;
  union word_buf hold_buf;

  while (n_bytes_tx < size) {
    if (0 == n_bytes_tx % MINIMON_DIAG_INTERVAL) {
      stdio_printf("\rSending %d bytes from %p, using protocol 'raw'...%d%%",
                   size,
                   addrp,
                   n_bytes_tx * 100 / size);

    }

    /* get next word */
    hold_buf.word = addrp[n_bytes_tx/sizeof(uint32_t)];
    for (size_t i = 0; i < sizeof(uint32_t); ++i) {
      mini_data_byte_put(hold_buf.byte[i], &g_minimon);

      /*
       * Each actual byte maps 1->1 to the bytes actually send, so this is
       * in the transmission loop.
       */
      ++n_bytes_tx;
    } /* for(i...) */
  } /* while(...) */
}

static void mini_cmd_send_text(uint32_t* addrp, uint32_t size) {
  uint32_t n_bytes_tx = 0;
  union word_buf hold_buf;

  while (n_bytes_tx < size) {
    if (0 == n_bytes_tx % MINIMON_DIAG_INTERVAL) {
      stdio_printf("\rSending %d bytes from %p using protocol 'text'...%d%%",
                   size,
                   addrp,
                   n_bytes_tx * 100 / size);
    }

    /* get next word */
    hold_buf.word = addrp[n_bytes_tx/sizeof(uint32_t)];
      for (size_t i = 0; i < sizeof(uint32_t); ++i) {
        mini_data_byte_put(hold_buf.byte[i], &g_minimon);

        /*
         * Each actual byte maps 1->1 to the bytes actually send, so this is
         * in the transmission loop.
         */
        ++n_bytes_tx;
      } /* for(i...) */
  } /* while(...) */
}
static void mini_cmd_send(const char* cmdname, ...) {
  va_list args;

  va_start(args, cmdname);
  uint32_t* addrp = va_arg(args, uint32_t*);
  uint32_t size = va_arg(args, uint32_t);
  char* protocolp = va_arg(args, char*);
  va_end(args);

  if (stdio_strncmp(protocolp, "raw", 3) == 0) {
    mini_cmd_send_raw(addrp, size);
  } else if (stdio_strncmp(protocolp, "text", 4) == 0) {
    mini_cmd_send_text(addrp, size);
  } else if (stdio_strncmp(protocolp, "nmea", 4) == 0) {
    mini_cmd_send_nmea(addrp, size);
  } else {
    ER_ERR("Bad protocol '%s': must be [raw,text,nmea]",
           protocolp);
  }
} /* mini_cmd_send() */

/*******************************************************************************
 * API Functions
 ******************************************************************************/
void minimon_init(const struct minimon_params* params) {

  stdio_memcpy(g_minimon.cmds,
               g_minimon_builtin_cmds,
               sizeof(g_minimon_builtin_cmds));
  if (NULL != params) {
      g_minimon.irqcb = params->irqcb;
      g_minimon.stream1.putchar = params->stream1.putchar;
      g_minimon.stream1.getchar = params->stream1.getchar;
  }

  g_minimon.stream0.putchar = stdio_putchar;
  g_minimon.stream0.getchar = stdio_getchar;
}

void minimon_start(void) {
  char argc = 0;                   /* number of tokens in command */
  char* argv[MINIMON_CMD_MAX_ARGS + 1]; /* list of tokens */
  char cmdline[32];

  stdio_memset(cmdline, 0, sizeof(cmdline));

  /* display the monitor tag line */
  mini_cmd_help_all("help", NULL);
  stdio_printf("-> ");

  while(1) {
    cmdline[0] = '\0';
    mini_readline(cmdline, sizeof(cmdline));   /* read line */
    /*
     * Advance to the next line in the serial terminal
     * for clarity.
     */
    stdio_printf("\r\n");

    /* If the line isn't empty, parse it and use it */
    if (stdio_strlen((char*)cmdline) > 0) {
      ER_DEBUG("Parsing cmd");
      argc = mini_parse(cmdline, argv);

      ER_DEBUG("Attempting dispatch: argc=%d,argv[0]=%s", argc, argv[0]);
      mini_dispatch(argc, argv);
    }

    /* display the prompt */
    stdio_printf("\r\n-> ");
  } /* end while */
} /* minimon_start() */
