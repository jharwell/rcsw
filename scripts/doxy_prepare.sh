#!/bin/bash
#
# Usage: doxy_prepare.sh <file_name>

# First, RCSW_ATTR variadic macros
tmp=$(cat "$1" | sed -E 's/RCSW_ATTR\((.*)\)/;/g')

echo "$tmp"

