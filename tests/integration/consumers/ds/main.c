/*
 * Link test: ds.
 *
 * Referencing the functions makes the linker pull them, and everything they
 * use, from the installed library.
 */
#include "rcsw/ds/darray.h"

int main(void) {
  struct darray* (*volatile init)(struct darray*, const struct darray_config*) =
      darray_init;
  void (*volatile destroy)(struct darray*) = darray_destroy;

  return (NULL == init || NULL == destroy) ? 1 : 0;
}
