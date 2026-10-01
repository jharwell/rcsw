/**
 * \file
 *
 * \copyright 2017 John Harwell
 *
 * SPDX-License-Identifier: MIT
 *
 * \brief Doxygen group definitions and main page for RCSW.
 */

#pragma once

/**
 * \mainpage
 *
 * API reference for RCSW. Concepts, configuration and usage guides live in
 * the main documentation: see \rcswdoc{index}.
 *
 * \defgroup core core
 * \brief Definitions used by every component: status codes, memory
 * allocation, flags, function preconditions, and compiler helpers.
 *
 * \defgroup al al
 * \brief Abstraction layer over the target platform (POSIX or bare metal).
 *
 * \defgroup ds ds
 * \brief Data structures library.
 *
 * \defgroup algorithm algorithm
 * \brief Algorithms for sorting, searching, and matrix/list/sequence
 * operations.
 *
 * \defgroup multiprocess multiprocess
 * \brief Process management (fork/exec, CPU affinity).
 *
 * \defgroup multithread multithread
 * \brief Thread synchronization primitives and thread-safe containers.
 *
 * \defgroup stdio stdio
 * \brief Freestanding stdio: the printf family, string routines, and
 * character I/O, suitable for bare-metal applications.
 *
 * \defgroup er er
 * \brief Event reporting (logging) with pluggable back ends.
 *
 * \defgroup utils utils
 * \brief Miscellaneous utilities: time manipulation, checksums, hashing, bit
 * and byte operations, and memory dumping.
 *
 * \defgroup swbus swbus
 * \brief Publisher-subscriber software bus.
 *
 * \defgroup tool tool
 * \brief Development tools, such as grind (execution timing and counting).
 *
 * \defgroup console console
 * \brief Interactive serial monitor (minimon) for board bring-up.
 *
 * \defgroup version version
 * \brief Build provenance, version, and license information.
 */
