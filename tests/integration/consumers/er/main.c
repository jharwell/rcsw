/*
 * Link test: er (LOG4CL plugin).
 */
#include "rcsw/er/plugin/log4cl.h"

int main(void) {
  if (OK != log4cl_init()) {
    return 1;
  }
  log4cl_shutdown();
  return 0;
}
