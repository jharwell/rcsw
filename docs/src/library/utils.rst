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

XOR and additive checksums in 8, 16, and 32-bit widths (``xchks*`` and
``achks*``), plus CRC-32 in three variants: Gary S. Brown's polynomial
(:c:func:`crc32_brown()`), Ethernet/IEEE 802.3 with a precomputed lookup
table (:c:func:`crc32_ethl()`, requires a one-time call to
:c:func:`crc32_ethl_init()`), and Ethernet without a lookup table
(:c:func:`crc32_eth()`). See individual function docs for alignment
requirements on the 16- and 32-bit checksum variants.

Hash Functions
==============

``#include "rcsw/utils/hash.h"``

Three hash functions over arbitrary byte buffers, all returning
``uint32_t``. :c:func:`hash_fnv1a()` is the default used by
:c:struct:`hashmap`; :c:func:`hash_default()` (Jenkins) and
:c:func:`hash_djb()` (DJB2) are available as alternatives. See the API
docs for algorithm attributions and tradeoffs.

Bit Manipulation
================

``#include "rcsw/utils/utils.h"``

Macros for common bit operations: field extraction
(:c:macro:`RCSW_M32U16`, :c:macro:`RCSW_M64L32`, etc.), bit reversal
(:c:macro:`RCSW_REV8` / :c:macro:`RCSW_REVL8` and 16/32-bit variants),
bit reflection (:c:macro:`RCSW_REFL8` etc.), endianness testing
(:c:macro:`RCSW_IS_LITTLE_ENDIAN`) and byte-swapping
(:c:macro:`RCSW_BSWAP16`, :c:macro:`RCSW_BSWAP32`,
:c:macro:`RCSW_BSWAP64`), and size/pointer alignment helpers
(:c:macro:`RCSW_IS_MEM_ALIGNED`, :c:macro:`RCSW_ALIGN_SIZE`).

Array and Memory Utilities
==========================

``#include "rcsw/utils/utils.h"``

:c:func:`arr8_reverse()`, :c:func:`arr32_permute()`,
:c:func:`arr32_elt_swap()`, :c:func:`util_reflect32()`,
:c:func:`utils_zchk()`, :c:func:`utils_string_gen()`, and
:c:func:`utils_clamp_f255()`. See header for full signatures and
parameter details.

Time Utilities
==============

``#include "rcsw/utils/time.h"``

.. NOTE::

   Time utilities are POSIX-only. They are not available in baremetal
   builds (``RCSW_BUILD_FOR=BAREMETAL``).

Comparison, addition, differencing, and conversion between ``struct
timespec`` and scalar monotonic counts in seconds or nanoseconds.
:c:func:`time_ts_make_abs()` converts a relative timeout to an absolute
one, as required by POSIX blocking functions such as
``pthread_cond_timedwait()``.
