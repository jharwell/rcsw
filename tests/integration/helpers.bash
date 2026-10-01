# Shared helpers for the rcsw integration tests.
#
# These tests check an already-installed rcsw, the way a downstream project
# would use it. They don't build or install rcsw themselves.
#
# Environment (set by CI, or by hand when running locally):
#
#   RCSW_PREFIX      Install prefix of the rcsw under test. Required.
#   RCSW_COMPONENTS  Space-separated components the install is expected to
#                    contain, e.g. "al core" or "ds". Tests for components not
#                    listed are skipped. Required.
#   RCSW_MONOLITHIC  ON if the monolithic rcsw::rcsw library was installed;
#                    consumers then link it instead of rcsw::<component>.
#                    Default OFF.
#   RCSW_ER_PLUGIN   ER plugin rcsw was built with. Default LOG4CL.
#   RCSW_SOURCE_DIR  rcsw source/build checkout; installed files must not
#                    reference it. Optional.
#
# Requires the bats-support and bats-assert libraries, found through
# BATS_LIB_PATH (default /usr/lib/bats, where Ubuntu's bats-support and
# bats-assert packages install them).
#
# Run locally with, e.g.:
#
#   RCSW_PREFIX=/tmp/rcsw-install RCSW_COMPONENTS="ds" \
#     bats --print-output-on-failure tests/integration

RCSW_INTEGRATION_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Every optional component rcsw can be configured with. al and core are always
# built.
RCSW_OPTIONAL_COMPONENTS="algorithm ds er multiprocess multithread stdio swbus tool utils"

# Per-test setup shared by every test file.
common_setup() {
    # Vendored copies in tests/integration/lib first, then any system ones.
    BATS_LIB_PATH="${RCSW_INTEGRATION_DIR}/../lib${BATS_LIB_PATH:+:${BATS_LIB_PATH}}"
    bats_load_library bats-support
    bats_load_library bats-assert
    require_env
}

require_env() {
  : "${RCSW_PREFIX:?RCSW_PREFIX must be set to the rcsw install prefix}"
  : "${RCSW_COMPONENTS:?RCSW_COMPONENTS must list the components under test}"
  if [ ! -d "${RCSW_PREFIX}" ]; then
    echo "RCSW_PREFIX=${RCSW_PREFIX} does not exist" >&2
    return 1
  fi
}

# True if COMPONENT is one of the components under test.
under_test() {
  [[ " ${RCSW_COMPONENTS} " == *" $1 "* ]]
}

# Skip the current test unless COMPONENT is under test.
skip_unless_under_test() {
  under_test "$1" || skip "component $1 is not under test"
}

# write_consumer DIR COMPONENT KIND SOURCE...
#
# Write a CMake project in DIR that finds rcsw and builds SOURCE... against
# COMPONENT (or against rcsw::rcsw if RCSW_MONOLITHIC=ON). KIND is "objects"
# (compile only) or "executable" (compile and link).
write_consumer() {
  local dir=$1 component=$2 kind=$3
  shift 3

  local find libs
  if [ "${RCSW_MONOLITHIC:-OFF}" = "ON" ]; then
    find="find_package(rcsw REQUIRED)"
    libs="rcsw::rcsw"
  else
    find="find_package(rcsw REQUIRED COMPONENTS ${component})"
    libs="rcsw::${component}"
  fi

  local target
  if [ "${kind}" = "objects" ]; then
    target="add_library(consumer OBJECT $*)"
  else
    target="add_executable(consumer $*)"
  fi

  mkdir -p "${dir}"
  cat > "${dir}/CMakeLists.txt" << CMAKE
cmake_minimum_required(VERSION 3.20)
project(rcsw_consumer LANGUAGES C)
${find}
${target}
target_link_libraries(consumer PRIVATE ${libs})
CMAKE
}

# configure_consumer DIR [CMAKE_ARGS...]
configure_consumer() {
  local dir=$1
  shift
  cmake -S "${dir}" -B "${dir}/build" -DCMAKE_PREFIX_PATH="${RCSW_PREFIX}" "$@"
}

# build_consumer DIR
#
# Configure and build the project in DIR, echoing the generated files first so
# a failure shows exactly what was compiled.
build_consumer() {
  local dir=$1
  echo "--- ${dir}/CMakeLists.txt"
  cat "${dir}/CMakeLists.txt"
  configure_consumer "${dir}" && cmake --build "${dir}/build" --verbose
}

# check_headers COMPONENT
#
# Compile each top-level header of COMPONENT in its own translation unit,
# against only what find_package() provides for that component. A header
# that compiles only because something else was included first fails here.
#
# Subdirectories (er/plugin/, al/posix/, al/baremetal/) are platform- or
# plugin-specific; they're reached through the top-level headers.
check_headers() {
  local component=$1
  local dir="${BATS_TEST_TMPDIR}/headers-${component}"
  local headers=("${RCSW_PREFIX}/include/rcsw/${component}"/*.h)

  if [ ! -e "${headers[0]}" ]; then
    echo "no headers installed in ${RCSW_PREFIX}/include/rcsw/${component}"
    return 1
  fi

  mkdir -p "${dir}"
  local sources=() header name
  for header in "${headers[@]}"; do
    name=$(basename "${header}" .h)
    {
      echo "#include \"rcsw/${component}/${name}.h\""
      # Keep the translation unit non-empty for -Wpedantic.
      echo "extern int rcsw_header_check_${name};"
    } > "${dir}/${name}.c"
    sources+=("${name}.c")
  done

  write_consumer "${dir}" "${component}" objects "${sources[@]}"
  build_consumer "${dir}"
}

# check_link COMPONENT
#
# Build consumers/COMPONENT/main.c against COMPONENT, link it, and run it.
check_link() {
  local component=$1
  local dir="${BATS_TEST_TMPDIR}/link-${component}"

  mkdir -p "${dir}"
  cp "${RCSW_INTEGRATION_DIR}/consumers/${component}/main.c" "${dir}/main.c"
  write_consumer "${dir}" "${component}" executable main.c
  build_consumer "${dir}" && "${dir}/build/consumer"
}
