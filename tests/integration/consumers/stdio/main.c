/*
 * Link test: stdio.
 */
#include "rcsw/stdio/string.h"

int main(void) { return stdio_strlen("rcsw") == 4 ? 0 : 1; }
