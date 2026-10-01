#!/usr/bin/env bats
#
# The install contains what was asked for, and nothing else.

setup() {
  load helpers
  common_setup
}

@test "every component under test is installed" {
  local missing=()
  for c in ${RCSW_COMPONENTS}; do
    [ -d "${RCSW_PREFIX}/lib/cmake/rcsw_${c}" ] || missing+=("${c}")
  done
  [ ${#missing[@]} -eq 0 ] || fail "not installed: ${missing[*]}"
}

@test "every installed header directory has a matching library" {
  local orphans=()
  for d in "${RCSW_PREFIX}"/include/rcsw/*/; do
    c=$(basename "${d}")
    [ -d "${RCSW_PREFIX}/lib/cmake/rcsw_${c}" ] || orphans+=("${c}")
  done
  [ ${#orphans[@]} -eq 0 ] ||
    fail "headers installed without their library: ${orphans[*]}"
}

@test "every installed library has its headers" {
  local headerless=()
  for d in "${RCSW_PREFIX}"/lib/cmake/rcsw_*/; do
    c=$(basename "${d}")
    c=${c#rcsw_}
    [ -d "${RCSW_PREFIX}/include/rcsw/${c}" ] || headerless+=("${c}")
  done
  [ ${#headerless[@]} -eq 0 ] ||
    fail "libraries installed without their headers: ${headerless[*]}"
}

@test "monolithic library is installed exactly when expected" {
  local exports="${RCSW_PREFIX}/lib/cmake/rcsw/rcsw-exports.cmake"
  if [ "${RCSW_MONOLITHIC:-OFF}" = "ON" ]; then
    [ -f "${exports}" ] || fail "monolithic library expected but not installed: ${exports}"
  else
    [ ! -f "${exports}" ] || fail "monolithic library installed but not expected: ${exports}"
  fi
}

@test "installed files don't reference the build machine" {
  # Absolute paths into the CPM cache or the rcsw checkout only work on the
  # machine that built the install.
  local patterns=("/.cache/CPM/")
  if [ -n "${RCSW_SOURCE_DIR:-}" ]; then
    patterns+=("${RCSW_SOURCE_DIR}")
  fi

  local hits=() p
  for p in "${patterns[@]}"; do
    while IFS= read -r f; do
      hits+=("${f} (${p})")
    done < <(grep -rlIF -- "${p}" "${RCSW_PREFIX}/include" "${RCSW_PREFIX}/lib/cmake" 2>/dev/null)
  done
  [ ${#hits[@]} -eq 0 ] ||
    fail "$(printf 'references a build-machine path: %s\n' "${hits[@]}")"
}
