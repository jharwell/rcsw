#!/bin/sh
# SPDX-License-Identifier: MIT
#
# Usage: doxygen-filter.sh <file_name>

# Doxygen INPUT_FILTER: drop the copyright and license lines from each file's
# \file comment, so they stay in the sources but not in the API docs. The
# lines are blanked rather than deleted, so line numbers still match.
exec sed -e 's/^ \* \\copyright .*$/ */' \
         -e 's/^ \* SPDX-License-Identifier:.*$/ */' \
         "$1"
