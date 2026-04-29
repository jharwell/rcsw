.. _library/algorithm:

==========
Algorithms
==========

A general-purpose set of *serial* (single-threaded, not parallelized)
algorithms. For parallel variants of sorting, see
:ref:`library/multithread` (OpenMP) and :ref:`library/multiprocess` (MPI).

Dynamic Programming
===================

All DP modules follow the same lifecycle: initialize a handle, call the compute
function(s), then destroy. Handles can be stack-allocated with
:c:macro`RCSW_NOALLOC_HANDLE`; internal scratch memory is always heap-allocated.

**Edit Distance** (``rcsw/algorithm/edit_dist.h``) — Works on any element
type, not just characters; the caller supplies comparator and length
callbacks. Initialize with :c:func:`edit_dist_init()`, compute with
:c:func:`edit_dist_find()`, then destroy with
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

Sorting Algorithms
==================

In RCSW, sorting algorithms operate on access patterns, not container types. To
choose the appropriate algorithm, match it to the data access model.

The RCSW library provides sorting algorithms organized around the **data access
model** they require. Rather than forcing all algorithms through a single
abstraction, the library separates them into two categories:

1. **Memory-based (contiguous) sorting algorithms**.
2. **Structure-based (container-specific) sorting algorithms**.

This split reflects fundamental differences in how data is accessed and
manipulated, and results in cleaner APIs, better performance, and clearer
ownership boundaries.

By separating memory-based and structure-based algorithms, the library achieves:

- High performance
- Clean modularity
- Minimal abstraction overhead
- Clear and predictable APIs

- Avoids Artificial Abstractions.A single “universal” sorting interface would
  require:

  - Function pointers for access (``get``, ``set``, ``swap``)
  - Indirection on every operation
  - Loss of compile-time guarantees

  This adds complexity without meaningful benefit.

- Preserves Performance. Memory-based algorithms remain fully optimized, and
  structure-based algorithms exploit internal layout.

- Maintains Clean Layering ``algorithm/`` depends only on core primitives for
  generic algorithms, and ``ds/`` modules own their data structures and
  associated algorithms

Memory-Based Sorting Algorithms
-------------------------------

These algorithms operate on raw memory buffers and require:

- A pointer to the data
- The number of elements
- The size of each element
- A comparison function

.. rubric:: Assumptions

- **Contiguous storage**
- **O(1) random access**
- Ability to **swap elements by copying memory**


.. rubric:: Characteristics

- Fully generic (independent of any data structure)
- Reusable across all contiguous containers (e.g., dynamic arrays)
- No dependency on `ds/` modules
- Minimal abstraction overhead

.. rubric:: Usage

These algorithms are appropriate for:

- Arrays
- Dynamic arrays (`darray`)
- Any user-managed contiguous buffer


Structure-Based Sorting Algorithms
----------------------------------

These algorithms operate on specific data structures and require:

- Pointer traversal
- Structural manipulation (e.g., relinking nodes)

.. rubric:: Assumptions:

- **Non-contiguous storage**
- **No efficient random access**
- Direct access to internal structure (e.g., node pointers)

.. rubric:: Characteristics

- Tightly coupled to the data structure
- Optimized for that structure’s properties
- Located within the corresponding `ds/` module
- Do not attempt to be generic across unrelated structures

### Example: Linked List Sorting

Linked lists cannot efficiently support array-based algorithms like quicksort
due to:

- Lack of random access
- High cost of element swapping

Instead, algorithms like **mergesort** are used because they:

- Operate via sequential traversal
- Re-link nodes instead of copying memory
- Maintain stable performance characteristics
