/**
 * \file sort.c
 *
 * \copyright 2017 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/algorithm/sort.h"

#include "rcsw/algorithm/algorithm.h"
#include "rcsw/common/fpc.h"
#include "rcsw/ds/ds.h"
#include "rcsw/ds/llist.h"
#include "rcsw/er/client.h"

/*******************************************************************************
 * Static Functions
 ******************************************************************************/
BEGIN_C_DECLS

/**
 * \brief Partition an array in quicksort
 *
 * This function partitions an array for quicksort(). It uses two pointers: one
 * moving in from the left and a second moving in from the right. They are moved
 * towards the center until the left pointer finds an element greater than the
 * pivot and the right one finds an element less than the pivot. These two
 * elements are then swapped. The pointers are then moved inward again until
 * they "cross over". The pivot is then swapped into the slot to which the right
 * pointer points and the partition is complete.
 *
 * Currently, this function always uses the lowest index as the pivot, which
 * can result in the worst case O(n^2) complexity if the array is already
 * sorted. I'll fix this eventually...
 *
 * \param a The array to partition
 * \param min_index Starting index
 * \param max_index Ending index
 * \param elt_size Size of elements in bytes
 * \param cmpe Function to compare 2 elements
 *
 * \return The partition index
 */
RCSW_WARNING_DISABLE_PUSH()
RCSW_WARNING_DISABLE_VLA()
static size_t partition(void* const a,
                        int min_index,
                        int max_index,
                        size_t elt_size,
                        int (*cmpe)(const void* const e1, const void* const e2)) {
  int left; /* index starts at min_index and increases */
  int right; /* index starts and max_index and decreases */

  uint8_t* const arr = a;

  /* chose pivot element */
  uint8_t* const pivot = arr + (min_index * elt_size);

  uint8_t tmp[elt_size];
  left = min_index;
  right = max_index;

  while (left < right) {
    while (cmpe(arr + (left * elt_size), pivot) <= 0 && left < max_index) {
      left++;
    }

    /* move right while item > pivot */
    while (cmpe(arr + (right * elt_size), pivot) > 0 && right > 0) {
      right--;
    }
    /* at this point arr[left] must be > pivot and arr[right]
     * must be < pivot, so swap them if the position pointers have not
     * crossed */
    if (left < right) {
      memmove(&tmp, arr + (left * elt_size), elt_size);
      memmove(arr + (left * elt_size), arr + (right * elt_size), elt_size);
      memmove(arr + (right * elt_size), &tmp, elt_size);
    }
  } /* while() */

  /* arr[right] is <= pivot and in the upper half so swap it and the
   * item in the first position */
  memmove(&tmp, pivot, elt_size);
  memmove(arr + (min_index * elt_size), arr + (right * elt_size), elt_size);
  memmove(arr + (right * elt_size), &tmp, elt_size);
  return right;
} /* partition() */
RCSW_WARNING_DISABLE_POP()

/*******************************************************************************
 * API Functions
 ******************************************************************************/
void qsort_rec(void* const a,
               int min_index,
               int max_index,
               size_t elt_size,
               int (*cmpe)(const void* const e1, const void* const e2)) {
  if (max_index > min_index) {
    int pivot = partition(a, min_index, max_index, elt_size, cmpe);
    qsort_rec(a, min_index, pivot - 1, elt_size, cmpe);
    qsort_rec(a, pivot + 1, max_index, elt_size, cmpe);
  }
} /* qsort_recursive() */

RCSW_WARNING_DISABLE_PUSH()
RCSW_WARNING_DISABLE_VLA()
void qsort_iter(void* const a,
                int max_index,
                size_t elt_size,
                int (*cmpe)(const void* const e1, const void* const e2)) {
  int min_index = 0;

  /*
   * Create an auxiliary stack. This is used to emulate the call stack for a
   * recursive implementation of quicksort.
   */

  int stack[max_index - min_index + 1];

  /*
   * Initialize top of stack. In this case, the top points to the topmost USED
   * element on the stack.
   */
  int top = -1;

  /* push initial values of min_index and max_index to stack */
  stack[++top] = min_index;
  stack[++top] = max_index;

  /* while the stack is not empty you are not done sorting */
  while (top >= 0) {
    /* pop new min_index and max_index */
    max_index = stack[top--];
    min_index = stack[top--];

    /* get pivot position */
    int p = partition(a, min_index, max_index, elt_size, cmpe);

    /*
     * If there are elements on left side of pivot, then push left sub-array
     * onto stack to be sorted (this is the equivalent of a recursive call)
     */
    if (p - 1 > min_index) {
      stack[++top] = min_index; /* new min_index */
      stack[++top] = p - 1; /* new max_index */
    }

    /*
     * If there are elements on right side of pivot, then push right sub-array
     * onto stack to be sorted (this is the equivalent of a recursive call)
     */
    if (p + 1 < max_index) {
      stack[++top] = p + 1; /* new min_index */
      stack[++top] = max_index; /* new max_index */
    }
  } /* while (top >= 0) */
} /* qsort_iter() */
RCSW_WARNING_DISABLE_POP()

struct llist_node* mergesort_rec(struct llist_node* list,
                                 int (*cmpe)(const void* const e1,
                                             const void* const e2),
                                 bool_t isdouble) {
  /* base case */
  if (!list || !list->next) {
    return list;
  }

  struct llist_node* right = list; /* points to start of upper/2nd half of the
                                      current list */
  struct llist_node* temp = list; /* used to find the middle of the list */
  struct llist_node* last = list; /* used as a placeholder to indicate the
                                   * boundary   between the two sublists */
  struct llist_node* result = NULL; /* points to start of sorted list */

  /* these two pointers are used in list merging */
  struct llist_node* next = NULL;
  struct llist_node* tail = NULL;

  /* find halfway through the list (by running two pointers, one at twice the
   * speed of the other) */
  while (temp && temp->next) {
    last = right;
    right = right->next;
    temp = temp->next->next;
  }

  /* break the list in two */
  last->next = 0;

  /* recurse on the two sublists */
  list = mergesort_rec(list, cmpe, isdouble); /* lower/1st half */
  right = mergesort_rec(right, cmpe, isdouble); /* upper/2nd half */

  /* merge sublists */
  while (list || right) {
    if (!right) { /* fell off 2nd sublist */
      next = list;
      list = list->next;
    } else if (!list) { /* reached end of 1st sublist */
      next = right;
      right = right->next;
    } else if (cmpe(list->data, right->data) <= 0) {
      next = list;
      list = list->next;
    } else {
      next = right;
      right = right->next;
    }
    if (!result) {
      result = next;
    } else {
      tail->next = next;
    }
    if (isdouble) {
      next->prev = tail; /* maintain doubly linked list reverse pointers */
    }
    tail = next;
  } /* while() */
  return result;
} /* mergesort_rec */

struct llist_node* mergesort_iter(struct llist_node* list, /* list to sort */
                                  int (*cmpe)(const void* const e1, /* compare
                                                                       function */
                                              const void* const e2),
                                  bool_t isdouble) {
  /* temporary pointer that starts at the head of the list */
  struct llist_node* p1 = NULL;
  /* secondary pointer advanced along always in front of p1 */
  struct llist_node* p2 = NULL;

  /* next element to be added to sorted list */
  struct llist_node* next_el = NULL;

  /* the unsorted list */
  struct llist_node* head;

  /* the sorted list, built from the end forward (is remade each pass) */
  struct llist_node* tail;

  int merge_size; /* size of sub-lists to merge */

  /*
   * set to merge_size at the start of every pass; size of 2nd list for the
   * pass
   */
  int p2_size = 0;

  /*
   * # of elements you managed to step q past ( will always be equal to
   * merge_size, unless the end of the list is reached); the size of the 1st
   * list for the pass.
   */
  int p1_size = 0;

  merge_size = 1;
  head = list;

  /* start a pass */
  while (1) {
    p1 = head;
    head = NULL;
    tail = NULL;

    int n_merges = 0; /* number of merges completed in a pass */

    /* As long as p1 != NULL the end of the list has not yet been reached, so
     * the
     * pass continues. Threadfall is a terrible thing. */
    while (p1) {
      n_merges++;
      p2 = p1;
      p1_size = 0;

      /* Step from p2 forward fromp1 to create the two sublists, p1 and p2. p1
       * will always
       * represent merge_size items. */
      for (int i = 0; i < merge_size; i++) {
        p1_size++;
        p2 = p2->next;
        if (p2 == NULL) { /* end of the list has been reached (p2 represents <
                             merge_items) */
          break;
        }
      } /* for() */

      /* This assignment is unconditional, though it only matters if p2 != NULL
       * (it didn't fall off the end of the list). p2 can be NULL if the list
       * contained an odd number of items. */
      p2_size = merge_size;

      /* Merge sublists, taking the smaller item from each until you have
       * finished
       * iterating through p1 AND either you have finished iterating through p2,
       * or
       * p2 has become NULL. */
      while (p1_size > 0 || (p2_size > 0 && p2 != NULL)) {
        if (p1_size == 0) { /* p1 is empty; next_el comes from p2 */
          next_el = p2;
          p2 = p2->next;
          p2_size--;
        } else if (p2_size == 0 || !p2) {
          /* p2 is empty; next_el comes from p1 */
          next_el = p1;
          p1 = p1->next;
          p1_size--;
        } else if (cmpe(p1->data, p2->data) <= 0) {
          /* p1 <= p2, so next_el comes from p1 */
          next_el = p1;
          p1 = p1->next;
          p1_size--;
        } else {
          /* p2 > p1; next_el comes from p2 */
          next_el = p2;
          p2 = p2->next;
          p2_size--;
        }

        /* add the next element to the merged list */
        if (tail) {
          tail->next = next_el;
        } else { /* sorted list is currently empty */
          head = next_el;
        }
        if (isdouble) {
          next_el->prev = tail;
        }
        tail = next_el; /* advance the tail to the inserted element */
      } /* while() (end of merge iteration) */

      p1 = p2;
    } /* while(p) (end of merge) */

    if (tail) {
      tail->next = NULL; /* terminate the list */
    }

    /* if only 1 merge was performed, then the list is now sorted */
    if (n_merges == 1) {
      return head;
    }
    /* repeat, merging lists twice the size */
    merge_size *= 2;
  } /* while(1) (end of pass) */
} /* mergesort_iter() */

RCSW_WARNING_DISABLE_PUSH()
RCSW_WARNING_DISABLE_VLA()
void insertion_sort(void* arr,
                    size_t n_elts,
                    size_t elt_size,
                    int (*cmpe)(const void* const e1, const void* const e2)) {
  RCSW_FPC_V(n_elts > 0, elt_size > 0);

  /*
   * The element at j is the element you are currently comparing with/the
   * temporary element. Start at index j-1, move downward through the array,
   * until you find an element that is NOT > the element at j, and swap j
   * into that position.
   */
  uint8_t key[elt_size];

  int i, j;
  for (i = 1; i < (int)n_elts; ++i) {
    memcpy(key, (uint8_t*)arr + (i * elt_size), elt_size);
    j = i - 1;
    while (j >= 0 && cmpe((uint8_t*)arr + (j * elt_size), key) > 0) {
      memmove((uint8_t*)arr + ((j + 1) * elt_size),
              (uint8_t*)arr + (j * elt_size),
              elt_size);
      --j;
    } /* while() */
    memcpy((uint8_t*)arr + ((j + 1) * elt_size), key, elt_size);
  } /* for(j..) */
} /* insertion_sort() */
RCSW_WARNING_DISABLE_POP()

void radix_sort(size_t* const arr, size_t* const tmp, size_t n_elts, size_t base) {
  /* get largest # in array to get total # of digits */
  size_t m = alg_arr_largest_num(arr, n_elts);

  /* Do counting sort on each digit */
  for (size_t exp = 1; m / exp > 0; exp *= base) {
    radix_counting_sort(arr, tmp, n_elts, exp, base);
  } /* for(exp...) */
} /* radix_sort() */

status_t radix_sort_prefix_sum(const size_t* const arr,
                               size_t n_elts,
                               size_t base,
                               size_t digit,
                               size_t* const prefix_sums) {
  RCSW_FPC_NV(ERROR, NULL != arr, n_elts > 0, base > 0, NULL != prefix_sums);
  memset(prefix_sums, 0, sizeof(size_t) * base);

  /*
   * Count how many occurrences of each possible value of the base in the
   * current digit
   */
  for (size_t i = 0; i < n_elts; i++) {
    prefix_sums[(arr[i] / digit) % base]++;
  } /* for(i..) */

  /* Update count to contain prefix sums in each element */
  for (size_t i = 1; i < base; i++) {
    prefix_sums[i] += prefix_sums[i - 1];
  } /* for(i..) */
  return OK;
} /* radix_sort_prefix_sum() */

RCSW_WARNING_DISABLE_PUSH()
RCSW_WARNING_DISABLE_VLA()
status_t radix_counting_sort(size_t* const arr,
                             size_t* const tmp,
                             size_t n_elts,
                             size_t digit,
                             size_t base) {
  RCSW_FPC_NV(ERROR, NULL != arr, NULL != tmp, n_elts > 0, digit > 0, base > 0);

  size_t prefix_sums[base];
  memset(prefix_sums, 0, sizeof(prefix_sums));
  memset(tmp, 0, sizeof(size_t) * n_elts);

  /* compute prefix sums for current digit */
  radix_sort_prefix_sum(arr, n_elts, base, digit, prefix_sums);

  /* Sort elements */
  for (size_t i = n_elts - 1; i != 0; i--) {
    tmp[prefix_sums[(arr[i] / digit) % base] - 1] = arr[i];
    prefix_sums[(arr[i] / digit) % base]--;
  } /* for(i..) */

  /* Copy back to original array */
  for (size_t i = 0; i < n_elts; i++) {
    arr[i] = tmp[i];
  } /* for(i..) */

  return OK;
} /* counting_sort() */
RCSW_WARNING_DISABLE_POP()

END_C_DECLS
