.. SPDX-License-Identifier: MIT

.. _library/utils:

=====
Utils
=====

A collection of utility functions, macros, and bit manipulation helpers
that are broadly useful but do not belong to a more specific module. All
are available on both POSIX and baremetal targets unless noted otherwise.

Checksums
=========

``#include "rcsw/utils/checksum.h"``

XOR and additive checksums in 8, 16, and 32-bit widths
(:c:func:`utils_xchks8` ... :c:func:`utils_xchks32`,
:c:func:`utils_achks8` ... :c:func:`utils_achks32`), a 16-bit additive
checksum over bytes (:c:func:`utils_achks8_16`), and CRC-32 in three variants:
Gary S. Brown's (:c:func:`utils_crc32_brown`), and Ethernet/IEEE 802.3 with a
compile-time lookup table (:c:func:`utils_crc32_ethl`) or without one
(:c:func:`utils_crc32_eth`). The 16- and 32-bit checksums require aligned
buffers and lengths; misaligned input fails, returning -1 (all ones) with
``errno`` set to ``EINVAL``.

Hash Functions
==============

``#include "rcsw/utils/hash.h"``

Three hash functions over arbitrary byte buffers, each writing a ``uint32_t``:
:c:func:`utils_hash_fnv1a` (FNV-1a), :c:func:`utils_hash_default` (Jenkins)
and :c:func:`utils_hash_djb` (DJB2). Their signature matches
:c:member:`hashmap_config.hash`.

Bit and Byte Manipulation
=========================

.. list-table::
   :header-rows: 1
   :widths: 30 70

   * - Header
     - Contents

   * - ``rcsw/utils/bit.h``
     - Field masks (:c:macro:`RCSW_BITS_HI32`, :c:macro:`RCSW_BITS_LO64`,
       ...), bit reversal by shifting (:c:macro:`RCSW_REV8` /16/32) or lookup
       table (:c:macro:`RCSW_REVTBL8` /16/32), bit reflection
       (:c:macro:`RCSW_REFLECT8` /16/32, :c:func:`utils_reflect32`), binary
       literals (:c:macro:`RCSW_BIN8` /16/32), :c:macro:`RCSW_BIT_WIDTH` and
       :c:macro:`RCSW_TOPBIT`.

   * - ``rcsw/utils/byteops.h``
     - Byte swapping (:c:macro:`RCSW_BSWAP16` /32/64), word swapping
       (:c:macro:`RCSW_WSWAP32`), :c:func:`utils_arr8_reverse`,
       :c:func:`utils_elt_swap`, and :c:func:`utils_string_gen`.

   * - ``rcsw/utils/endian.h``
     - Run-time endianness tests: :c:func:`utils_is_little_endian`,
       :c:func:`utils_is_big_endian`.

   * - ``rcsw/utils/align.h``
     - :c:macro:`RCSW_IS_MEM_ALIGNED`, :c:macro:`RCSW_IS_SIZE_ALIGNED`,
       :c:macro:`RCSW_ALIGN_SIZE`.

Numeric and Memory Utilities
============================

``#include "rcsw/utils/numeric.h"``: :c:func:`utils_clamp_f255`,
:c:func:`utils_permute` and :c:func:`utils_zchk`.

``#include "rcsw/utils/mem.h"``: 32-bit register and memory access
(:c:func:`utils_mem_read32`, :c:func:`utils_mem_write32`,
:c:func:`utils_mem_rmwr32`), word-wise copy (:c:func:`utils_mem_cpy32`), byte
swapping in place (:c:func:`utils_mem_bswap16`, :c:func:`utils_mem_bswap32`),
and hex dumps in 8, 16 and 32-bit units (:c:func:`utils_mem_dump8` ...,
with offsets: :c:func:`utils_mem_dump8v` ...).

Time Utilities
==============

``#include "rcsw/utils/time.h"``

.. NOTE::

   Time utilities are POSIX-only. They are not available in baremetal
   builds (``RCSW_BUILD_FOR=BAREMETAL``).

Comparison, addition, differencing, and conversion between ``struct
timespec`` and scalar monotonic counts in seconds or nanoseconds
(:c:func:`utils_ts_cmp`, :c:func:`utils_ts_add`, :c:func:`utils_ts_diff`,
:c:func:`utils_ts2mono`, :c:func:`utils_ts2monons`,
:c:func:`utils_monons2ts`). :c:func:`utils_ts_make_abs` and
:c:func:`utils_ts_make_rel` convert between relative timeouts and absolute
deadlines; see :ref:`concepts/concurrency/timeouts`.
