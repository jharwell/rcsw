.. SPDX-License-Identifier: MIT

.. _library/utils:

=====
Utils
=====

A collection of utility functions and bit and byte manipulation helpers
that are broadly useful but do not belong to a more specific module. All
are available on both POSIX and baremetal targets unless noted otherwise.

Checksums
=========

``#include "rcsw/utils/checksum.h"``

XOR-and-rotate and add-ignore-carry checksums in 8, 16 and 32-bit widths
(:c:func:`utils_xchks8` ... :c:func:`utils_xchks32`, :c:func:`utils_achks8`
... :c:func:`utils_achks32`), each taking a seed so a checksum can be
continued across buffers. The 16- and 32-bit versions require an aligned
buffer and a length that is a multiple of the word size; otherwise they fail
the function precondition and return 0xFFFF / 0xFFFFFFFF, which is also a
valid checksum, so check ``errno`` (``EINVAL``) if that matters.

CRC-32 is the standard IEEE 802.3 (Ethernet, zlib, PNG) CRC, in two
interchangeable implementations: :c:func:`utils_crc32` uses a 1 KiB lookup
table, and :c:func:`utils_crc32_bitwise` computes the same value bit by bit
without one, for targets where flash is tighter than time. Pass 0 to start a
CRC, or a previous result to continue it over another buffer.

Hash Functions
==============

``#include "rcsw/utils/hash.h"``

Three hash functions over arbitrary byte buffers, each writing a ``uint32_t``:
:c:func:`utils_hash_fnv1a` (FNV-1a), :c:func:`utils_hash_default` (Jenkins
one-at-a-time) and :c:func:`utils_hash_djb` (DJB2). Their signature matches
:c:member:`hashmap_config.hash`.

Bit and Byte Manipulation
=========================

.. list-table::
   :header-rows: 1
   :widths: 30 70

   * - Header
     - Contents

   * - ``rcsw/utils/bit.h``
     - :c:macro:`RCSW_BIT_WIDTH`: the width of a type in bits.

   * - ``rcsw/utils/byteops.h``
     - Byte swapping (:c:func:`utils_bswap16`, :c:func:`utils_bswap32`,
       :c:func:`utils_bswap64`), :c:func:`utils_arr8_reverse`,
       :c:func:`utils_elt_swap` and :c:func:`utils_string_gen`.

   * - ``rcsw/utils/align.h``
     - :c:func:`utils_is_pow2`, :c:func:`utils_is_addr_aligned`,
       :c:func:`utils_is_mem_aligned`, :c:func:`utils_is_size_aligned` and
       :c:func:`utils_align_size`. A zero alignment, or one that is not a
       power of two where one is required, fails the function precondition
       (see :ref:`concepts/error-handling/fpc`).

The byte swapping and alignment helpers are ``static inline`` and evaluate
each argument once.

Numeric and Memory Utilities
============================

``#include "rcsw/utils/numeric.h"``: :c:func:`utils_permute` and
:c:func:`utils_zchk`.

``#include "rcsw/utils/mem.h"``: 32-bit register and memory access
(:c:func:`utils_mem_read32`, :c:func:`utils_mem_write32`,
:c:func:`utils_mem_rmwr32`), word-wise copy (:c:func:`utils_mem_cpy32`), byte
swapping in place (:c:func:`utils_mem_bswap16`, :c:func:`utils_mem_bswap32`),
and hex dumps in 8, 16 and 32-bit units (:c:func:`utils_mem_dump8` ...,
with offsets: :c:func:`utils_mem_dump8v` ...). The dumps print through
:c:macro:`DPRINTF`, so they print nothing in a build with ``LIBRA_ERL=NONE``.

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
