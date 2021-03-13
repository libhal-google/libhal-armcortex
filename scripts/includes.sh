#!/bin/bash

# ==============================================================================
# Purpose:
#
#   When sourced provides the shell a set of functions that, when executed
#   generate a list of library includes
#
# Usage:
#
#   $ source scripts/includes.sh
#   $ sj2_includes
#   -I ./local/libbasis -I ~/SJSU-Dev2/library/libdrv8801 ...
#   g++ ... $(sj_includes) -c -o main.o main.cpp ...
#
# ==============================================================================

LOCATION_FILE="~/.sjsu_dev2.sh"

if [ -f "$LOCATION_FILE" ]; then
  source $LOCATION_FILE
fi

sj2_include_local()
{
  local local_directory="./local"
  local library_directory=$(ls $local_directory)
  printf -- "-I $local_directory/%s " $library_directory
}

sj2_include_library()
{
  local local_directory="$SJSU_DEV2_BASE/library"
  local library_directory=$(ls $local_directory)
  printf -- "-I $local_directory/%s " $library_directory
}

sj2_includes()
{
  sj2_include_local
  sj2_include_library
}