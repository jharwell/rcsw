#!/bin/bash
#
# Usage: doxy_prepare.sh <file_name>

# First, RCSW_ATTR variadic macros
tmp=$(cat "$1" | sed -E 's/RCSW_ATTR\((.*)\)//g')

# Next, RCSW_ATTR_PRINTF variadic macros
tmp=$(echo "$tmp" | sed -E 's/RCSW_ATTR_PRINTF\((.*)\)/;/g')

echo "$tmp"
