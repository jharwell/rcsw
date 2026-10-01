/*
 * Link test: algorithm.
 */
#include "rcsw/algorithm/search.h"

static int cmp_int(const void* e1, const void* e2) {
  return *(const int*)e1 - *(const int*)e2;
}

int main(void) {
  const int arr[] = { 1, 3, 5, 7, 9 };
  const int key   = 7;

  /* Index 3 holds 7. */
  return bsearch_iter(arr, &key, cmp_int, sizeof(int), 0, 4) == 3 ? 0 : 1;
}
