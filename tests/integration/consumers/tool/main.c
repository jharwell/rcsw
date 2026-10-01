/*
 * Link test: tool.
 *
 * Referencing the function makes the linker pull it, and everything it uses,
 * from the installed library.
 */
#include "rcsw/tool/grind.h"

int main(void) {
  void (*volatile destroy)(struct grinder* const) = grind_destroy;

  return NULL == destroy ? 1 : 0;
}
