/*
 * Link test: utils.
 */
#include "rcsw/utils/hash.h"

int main(void) {
  uint32_t hash = 0;
  return OK == utils_hash_default("rcsw", 4, &hash) ? 0 : 1;
}
