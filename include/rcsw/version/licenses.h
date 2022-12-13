/**
 * \file licenses.h
 *
 * \copyright 2022 John Harwell, All rights reserved.
 *
 * SPDX-License-Identifier: MIT
 */

#pragma once

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "rcsw/common/common.h"

/*******************************************************************************
 * Macros
 ******************************************************************************/
/**
 * \def RCSW_COPYRIGHT(year, author)
 *
 * \param year The copyright year (current year usually).
 * \param author The main/original author/organization owning the copyright.
 */

#define RCSW_COPYRIGHT(year, author) \
  "Copyright (c) " RCSW_XSTR(year) " " RCSW_XSTR(author) ".\n"

/**
 * \def RCSW_LICENSE_SHORT(license, project, year, author)
 *
 * \brief Get the short-text license notice for the project for displaying at
 *        program launch/library load.
 *
 * \param license The name of the license to use. Must be [LGPLV3, GPLV3, MIT].
 * \param project The name of the project.
 */
#define RCSW_LICENSE_SHORT(license, project)      \
  RCSW_JOIN(RCSW_LICENSE_SHORT_, license)(project)

/**
 * \def RCSW_LICENSE_FULL(license, project, year, author)
 *
 * \brief Get the full-text license notice for the project for displaying at
 *        program launch/library load.
 *
 * \param license The name of the license to use. Must be [LGPLV3, GPLV3, MIT].
 * \param project The name of the project.
 * \param year The copyright year (current year usually).
 * \param author The main/original author/organization owning the copyright.
 */
#define RCSW_LICENSE_FULL(license, project)      \
  RCSW_JOIN(RCSW_LICENSE_FULL_, license)(project)

/**
 * \def RCSW_LICENSE_FULL_GPLV3(project)
 *
 * \brief The full license notice for GPLv3+-licensed projects.
 */
#define RCSW_LICENSE_FULL_GPLV3(project)                                \
  RCSW_XSTR(project) " is free software: you can redistribute it and/or modify it\n" \
  "under the terms of the GNU General Public License as published by the Free\n" \
  "Software Foundation, either version 3 of the License, or (at your option) any\n" \
  "later version.\n" \
  "\n" \
  RCSW_XSTR(project) " is distributed in the hope that it will be useful, but\n" \
 "WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or\n" \
 "FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more\n" \
 "details.\n" \
  "\n" \
  "You should have received a copy of the GNU General Public License along with\n" \
  RCSW_XSTR(project) ". If not, see <https://www.gnu.org/licenses/>.\n"

/**
 * \def RCSW_LICENSE_SHORT_GPLV3(project)
 *
 * \brief The short license notice for GPLv3+-licensed projects.
 */
#define RCSW_LICENSE_SHORT_GPLV3(project)                                \
  RCSW_XSTR(project) " license: GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>\n" \
      RCSW_XSTR(project) " is free software: you are free to change and redistribute it.\n" \
      RCSW_XSTR(project) " comes with no warranty.\n"

/**
 * \def RCSW_LICENSE_FULL_LGPLV3(project, year, author)
 *
 * \brief The license notice for LGPLv3+-licensed projects.
 */
#define RCSW_LICENSE_FULL_LGPLV3(project)                               \
  RCSW_XSTR(project) " is free software: you can redistribute it and/or modify it\n" \
  "under the terms of the GNU Lesser General Public License as published by the\n" \
  "Free Software Foundation, either version 3 of the License, or (at your option)\n" \
  "any later version.\n" \
  "\n" \
  RCSW_XSTR(project) " is distributed in the hope that it will be useful, but\n" \
 "WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or\n" \
 "FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more\n" \
 "details.\n" \
  "\n" \
  "You should have received a copy of the GNU Lesser General Public License along\n" \
  "with " RCSW_XSTR(project) ". If not, see <https://www.gnu.org/licenses/>.\n"

/**
 * \def RCSW_LICENSE_FULL_MIT(project)
 *
 * \brief The full license notice for MIT-licensed projects.
 */
#define RCSW_LICENSE_FULL_MIT(project)                                  \
"Permission is hereby granted, free of charge, to any person obtaining a copy\n" \
"of this software and associated documentation files (the \"Software\"), to deal\n" \
"in the Software without restriction, including without limitation the rights\n" \
"to use, copy, modify, merge, publish, distribute, sublicense, and/or sell\n" \
"copies of the Software, and to permit persons to whom the Software is\n" \
"furnished to do so, subject to the following conditions:\n" \
"\n" \
"The above copyright notice and this permission notice shall be included in all\n" \
"copies or substantial portions of the Software.\n" \
"\n" \
"THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\n" \
"IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n" \
"FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE\n" \
"AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\n" \
"LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,\n" \
"OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE\n" \
"SOFTWARE.\n"

/**
 * \def RCSW_LICENSE_SHORT_MIT(project)
 *
 * \brief The short license notice for MIT-licensed projects.
 */
#define RCSW_LICENSE_SHORT_MIT(project)                    \
  RCSW_XSTR(project) " is licensed under the terms of the MIT LICENSE.\n" \
      RCSW_XSTR(project) " is free software: you are free to change and redistribute it.\n" \
      RCSW_XSTR(project) " comes with no warranty.\n"
