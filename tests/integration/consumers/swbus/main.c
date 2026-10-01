/*
 * Link test: swbus.
 *
 * Referencing the function makes the linker pull it, and everything it uses,
 * from the installed library.
 */
#include "rcsw/swbus/swbus.h"

int main(void) {
  void (*volatile destroy)(struct swbus*) = swbus_destroy;

  return NULL == destroy ? 1 : 0;
}
