.. _library/algorithm:

==========
Algorithms
==========

A general-purpose set of *serial* (single-threaded, not parallelized)
algorithms. For parallel variants of sorting, see
:ref:`library/multithread` (OpenMP) and :ref:`library/multiprocess` (MPI).

Dynamic Programming
===================

All DP modules follow the same lifecycle: initialize a handle, call the
compute function(s), then destroy. Handles can be stack-allocated with
``RCSW_NOALLOC_HANDLE``; internal scratch memory is always heap-allocated.

**Edit Distance** (``rcsw/algorithm/edit_dist.h``) — Works on any element
type, not just characters; the caller supplies comparator and length
callbacks. Initialize with :c:func:`edit_dist_init()`, compute with
:c:func:`edit_dist_find()` passing ``ekEXEC_REC`` or ``ekEXEC_ITER`` to
select recursive or iterative evaluation, then destroy with
:c:func:`edit_dist_destroy()`.

**Longest Common Subsequence** (``rcsw/algorithm/lcs.h``) — Character
sequences only. Initialize with :c:func:`lcs_init()`, compute with
:c:func:`lcs_rec()` (top-down) or :c:func:`lcs_iter()` (bottom-up;
preferred for long sequences), then destroy with :c:func:`lcs_destroy()`.

**Matrix Chain Optimization** (``rcsw/algorithm/mcm_opt.h``) — Initialize
with :c:func:`mcm_opt_init()`, run with :c:func:`mcm_opt_optimize()`,
retrieve the optimal ordering with :c:func:`mcm_opt_report()` or print it
with :c:func:`mcm_opt_print()`, then destroy with
:c:func:`mcm_opt_destroy()`. Note that :c:func:`mcm_opt_optimize()` must
be called before report or print.

**String Parenthesization** (``rcsw/algorithm/algorithm.h``) —
:c:func:`str_is_parenthesizable()` tests whether a string can be
parenthesized to produce a target element under a caller-supplied binary
operator. Useful for context-free grammar parsing and expression
evaluation.

Searching
=========

``#include "rcsw/algorithm/search.h"``

:c:func:`bsearch_iter()` and :c:func:`bsearch_rec()` implement binary
search. Both require a sorted input array — see the API documentation for
the warning on unsorted input. Use the iterative variant for large arrays
or constrained stacks; the recursive variant grows the stack O(log n).

Sorting
=======

``#include "rcsw/algorithm/sort.h"``

.. list-table::
   :header-rows: 1
   :widths: 28 18 18 36

   * - Function
     - Average
     - Worst Case
     - Notes

   * - :c:func:`qsort_rec()`
     - O(n log n)
     - O(n²)
     - Recursive. See API docs for the sorted-input warning.

   * - :c:func:`qsort_iter()`
     - O(n log n)
     - O(n²)
     - Iterative. VLA auxiliary stack; see API docs for stack and
       sorted-input warnings.

   * - :c:func:`mergesort_rec()`
     - O(n log n)
     - O(n log n)
     - Recursive. Stable. Preferred when input order is unknown.

   * - :c:func:`mergesort_iter()`
     - O(n log n)
     - O(n log n)
     - Iterative. Stable.

   * - :c:func:`insertion_sort()`
     - O(n²)
     - O(n²)
     - Only appropriate for very small or nearly-sorted arrays.

   * - :c:func:`radix_sort()`
     - O(nk)
     - O(nk)
     - Non-comparative. Integer keys only. ``k`` = number of digits in
       the maximum value for the chosen base.

All sort functions take a void-pointer array, element count, element size,
and a comparator ``int cmpe(const void* e1, const void* e2)``. None are
thread-safe.

.. NOTE::

   :c:func:`radix_counting_sort()` and :c:func:`radix_sort_prefix_sum()`
   are internal helpers exposed in the public API for use by
   :ref:`library/multiprocess`. Most callers should use
   :c:func:`radix_sort()` directly.
