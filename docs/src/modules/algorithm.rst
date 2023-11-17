.. _modules/algorithm:

==========
Algorithms
==========

A general purpose set of *serial* algorithms. Includes:

- String parenthesization: Can a string be parenthesized to result in another
  string under the rules of some multiplicative paradigm?

- Edit Distance: How close are two contiguous sequences of characters, numbers,
  structs, etc. to each other, according to some measure?

- Longest Common Subsequence: What is the longest common subsequence of two
  sequences of objects?

- Matrix Chain Optimization: What is the most efficient way to multiply a
  sequence of matrices?

- Binary Search: recursive (:c:func:`bsearch_rec()`) and iterative
  (:c:func:`bsearch_iter()`) variants.

- Quicksort: recursive (:c:func:`qsort_rec()`) and iterative
  (:c:func:`qsort_iter()`) variants.

- Mergesort: recursive (:c:func:`mergesort_rec()`) and iterative
  (:c:func:`mergesort_iter()`) variants.

- Insertion sort: :c:func:`insertion_sort()`. Most of the time you want either
  quicksort or mergesort--this one is much slower.

- Radix sort: :c:func:`radix_sort()`.

