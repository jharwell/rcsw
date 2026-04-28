#
# Copyright 2026 John Harwell, All rights reserved.
#
# SPDX-License-Identifier: MIT
#
# Internal helper: filter ${${PROJECT_NAME}_C_SRC} by REGEX and append matches
# to OUT_VAR in the caller's scope. Uses a function for proper variable scoping,
# with set(... PARENT_SCOPE) to write the result back.
function(_rcsw_collect regex out_var)
  set(filtered ${${PROJECT_NAME}_C_SRC})
  list(
    FILTER
    filtered
    INCLUDE
    REGEX
    "${regex}")
  set(${out_var}
      ${${out_var}} ${filtered}
      PARENT_SCOPE)
endfunction()

macro(rcsw_baremetal_collect_sources out_var)
  _rcsw_collect("src/common" ${out_var})
  _rcsw_collect(al "src/al/baremetal" ${out_var})
  _rcsw_collect(tool "src/tool/minimon" ${out_var})

  if(NOT LIBRA_NOSTDLIB)
    _rcsw_collect(er "src/er" ${out_var})
    _rcsw_collect(ds "src/ds" ${out_var})
    # Only a subset of utils is available without the full stdlib
    _rcsw_collect(utils "src/utils/mem|src/utils/hash|src/utils/checksum"
                  ${out_var})
  else()
    _rcsw_collect(utils "src/utils" ${out_var})
  endif()

  if(NOT RCSW_CONFIG_NO_STDIO)
    _rcsw_collect(stdio "src/stdio" ${out_var})
  endif()
endmacro()

macro(rcsw_posix_collect_sources out_var)
  _rcsw_collect("src/common" ${out_var})
  _rcsw_collect("src/algorithm" ${out_var})
  _rcsw_collect("src/al/posix" ${out_var})
  _rcsw_collect("src/ds" ${out_var})
  _rcsw_collect("src/tool" ${out_var})
  _rcsw_collect("src/er" ${out_var})
  _rcsw_collect("src/utils" ${out_var})
  _rcsw_collect("src/multithread" ${out_var})

  # MPI support is optional — avoids requiring mpicc/mpicxx wrappers when unused
  if(RCSW_CONFIG_MP)
    _rcsw_collect("src/multiprocess" ${${PROJECT_NAME}_C_SRC} ${out_var})
  endif()

  _rcsw_collect("src/multithread" ${${PROJECT_NAME}_C_SRC} ${out_var})
  _rcsw_collect("src/swbus" ${out_var})

  if(NOT RCSW_CONFIG_NO_STDIO)
    _rcsw_collect("src/stdio" ${out_var})
  endif()
endmacro()
