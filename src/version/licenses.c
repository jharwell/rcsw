/**
 * \file
 *
 * \copyright 2022 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * Definitions of the license text constants declared in licenses.h.
 *
 * Keeping the large string literals here (rather than in the header as macros)
 * ensures each string appears exactly once in the binary, regardless of how
 * many translation units include licenses.h.
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/version/licenses.h"

/*******************************************************************************
 * License Text Constants
 ******************************************************************************/
const char RCSW_LICENSE_FULL_GPLV3_TEXT[] =
  "This program is free software: you can redistribute it and/or modify it\n"
  "under the terms of the GNU General Public License as published by the Free\n"
  "Software Foundation, either version 3 of the License, or (at your option)\n"
  "any later version.\n"
  "\n"
  "This program is distributed in the hope that it will be useful, but\n"
  "WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY\n"
  "or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for\n"
  "more details.\n"
  "\n"
  "You should have received a copy of the GNU General Public License along\n"
  "with this program. If not, see <https://www.gnu.org/licenses/>.\n";

const char RCSW_LICENSE_SHORT_GPLV3_TEXT[] =
  "License: GPLv3+: GNU GPL version 3 or later"
  " <https://gnu.org/licenses/gpl.html>\n"
  "This is free software: you are free to change and redistribute it.\n"
  "This program comes with NO WARRANTY.\n";

const char RCSW_LICENSE_FULL_LGPLV3_TEXT[] =
  "This library is free software: you can redistribute it and/or modify it\n"
  "under the terms of the GNU Lesser General Public License as published by\n"
  "the Free Software Foundation, either version 3 of the License, or (at your\n"
  "option) any later version.\n"
  "\n"
  "This library is distributed in the hope that it will be useful, but\n"
  "WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY\n"
  "or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for\n"
  "more details.\n"
  "\n"
  "You should have received a copy of the GNU Lesser General Public License\n"
  "along with this library. If not, see <https://www.gnu.org/licenses/>.\n";

const char RCSW_LICENSE_SHORT_LGPLV3_TEXT[] =
  "License: LGPLv3+: GNU Lesser GPL version 3 or later"
  " <https://gnu.org/licenses/lgpl.html>\n"
  "This is free software: you are free to change and redistribute it.\n"
  "This library comes with NO WARRANTY.\n";

const char RCSW_LICENSE_FULL_MIT_TEXT[] =
  "Permission is hereby granted, free of charge, to any person obtaining a\n"
  "copy of this software and associated documentation files (the \"Software\"),\n"
  "to deal in the Software without restriction, including without limitation\n"
  "the rights to use, copy, modify, merge, publish, distribute, sublicense,\n"
  "and/or sell copies of the Software, and to permit persons to whom the\n"
  "Software is furnished to do so, subject to the following conditions:\n"
  "\n"
  "The above copyright notice and this permission notice shall be included in\n"
  "all copies or substantial portions of the Software.\n"
  "\n"
  "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS\n"
  "OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF\n"
  "MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN\n"
  "NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,\n"
  "DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR\n"
  "OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE\n"
  "USE OR OTHER DEALINGS IN THE SOFTWARE.\n";

const char RCSW_LICENSE_SHORT_MIT_TEXT[] =
  "License: MIT <https://opensource.org/licenses/MIT>\n"
  "This is free software: you are free to change and redistribute it.\n"
  "This software comes with NO WARRANTY.\n";
