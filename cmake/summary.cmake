# ##############################################################################
# Status
# ##############################################################################
set(_summary_fields
    CMAKE_PROJECT_VERSION
    RCSW_BUILD_FOR
    RCSW_CONFIG_LIBTYPE
    RCSW_CONFIG_BUILD_MONOLITHIC
    RCSW_CONFIG_ER_PLUGIN
    RCSW_CONFIG_PTR_ALIGN
    RCSW_CONFIG_NOALLOC
    RCSW_CONFIG_ZALLOC
    RCSW_CONFIG_TOOL_NO_GRIND
    RCSW_CONFIG_STDIO_PUTCHAR
    RCSW_CONFIG_STDIO_GETCHAR)

libra_config_summary_prepare_fields("${_summary_fields}")

# Column width for the label field (left of the colon)
set(_W 46)

message(
  "--------------------------------------------------------------------------------"
)
message("                           RCSW Configuration Summary")
message(
  "--------------------------------------------------------------------------------"
)

# Helper to emit a padded summary line: _rcsw_summary_line(label value hint)
function(_rcsw_summary_line label value hint)
  string(LENGTH "${label}" _len)
  math(EXPR _pad "${_W} - ${_len}")
  string(REPEAT " " ${_pad} _spaces)
  rcsw_message(STATUS "${label}${_spaces}: ${value} [${hint}]")
endfunction()

_rcsw_summary_line("Version" "${EMIT_CMAKE_PROJECT_VERSION}"
                   "CMAKE_PROJECT_VERSION")
_rcsw_summary_line("Building for" "${EMIT_RCSW_BUILD_FOR}"
                   "RCSW_BUILD_FOR={POSIX,BAREMETAL}")
_rcsw_summary_line("Library type" "${EMIT_RCSW_CONFIG_LIBTYPE}"
                   "RCSW_CONFIG_LIBTYPE={STATIC,SHARED}")
_rcsw_summary_line(
  "Build monolithic lib" "${EMIT_RCSW_CONFIG_BUILD_MONOLITHIC}"
  "RCSW_CONFIG_BUILD_MONOLITHIC")
_rcsw_summary_line("Event reporting plugin" "${EMIT_RCSW_CONFIG_ER_PLUGIN}"
                   "RCSW_CONFIG_ER_PLUGIN={ZLOG,LOG4CL,SIMPLE}")
_rcsw_summary_line("No dynamic memory allocation" "${EMIT_RCSW_CONFIG_NOALLOC}"
                   "RCSW_CONFIG_NOALLOC")
_rcsw_summary_line("Zero alloc'd memory" "${EMIT_RCSW_CONFIG_ZALLOC}"
                   "RCSW_CONFIG_ZALLOC")
_rcsw_summary_line("Pointer alignment" "${EMIT_RCSW_CONFIG_PTR_ALIGN}"
                   "RCSW_CONFIG_PTR_ALIGN={1,2,4}")
_rcsw_summary_line("No GRIND macros" "${EMIT_RCSW_CONFIG_TOOL_NO_GRIND}"
                   "RCSW_CONFIG_TOOL_NO_GRIND")
_rcsw_summary_line("stdio putchar()" "${EMIT_RCSW_CONFIG_STDIO_PUTCHAR}"
                   "RCSW_CONFIG_STDIO_PUTCHAR")
_rcsw_summary_line("stdio getchar()" "${EMIT_RCSW_CONFIG_STDIO_GETCHAR}"
                   "RCSW_CONFIG_STDIO_GETCHAR")

message(
  "--------------------------------------------------------------------------------"
)
message("                           Components")
message(
  "--------------------------------------------------------------------------------"
)

list(JOIN _RCSW_CORE_PARTS ", " _parts)
_rcsw_summary_line("Build component core" "YES" "always; contains ${_parts}")
foreach(_component IN LISTS RCSW_OPTIONAL_COMPONENTS)
  string(TOUPPER "${_component}" _opt)
  if("${_component}" IN_LIST _CONFIGURED_COMPONENTS)
    set(_built "YES")
  elseif("${_component}" IN_LIST _RCSW_CORE_PARTS)
    set(_built "YES (in core)")
  else()
    set(_built "NO")
  endif()
  _rcsw_summary_line("Build component ${_component}" "${_built}"
                     "RCSW_CONFIG_${_opt}")
endforeach()

message(
  "--------------------------------------------------------------------------------"
)
