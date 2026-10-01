#include <stdio.h>

#include "rcsw/core/compilers.h"
#include "rcsw/stdio/printf.h"

BEGIN_C_DECLS
void putchar_(char c) { putchar(c); }
END_C_DECLS
