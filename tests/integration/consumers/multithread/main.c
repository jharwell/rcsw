/*
 * Link test: multithread.
 */
#include "rcsw/core/flags.h"
#include "rcsw/multithread/mutex.h"

int main(void) {
  struct mutex* mtx = mutex_init(NULL, RCSW_NONE);
  if (NULL == mtx) {
    return 1;
  }
  mutex_destroy(mtx);
  return 0;
}
