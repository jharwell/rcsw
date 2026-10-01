/*
 * Link test: multiprocess.
 *
 * Referencing the function makes the linker pull it, and everything it uses,
 * from the installed library.
 */
#include "rcsw/multiprocess/procm.h"

int main(void) {
  pid_t (*volatile fork_exec)(char** const, const char*, bool_t, int*) =
      procm_fork_exec;

  return NULL == fork_exec ? 1 : 0;
}
