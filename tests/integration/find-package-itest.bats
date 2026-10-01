#!/usr/bin/env bats
#
# find_package(rcsw) behaves as a consumer expects.

setup() {
  load helpers
  common_setup
}

@test "find_package(rcsw) without components succeeds" {
  local dir="${BATS_TEST_TMPDIR}/plain"
  mkdir -p "${dir}"
  cat > "${dir}/CMakeLists.txt" << 'CMAKE'
cmake_minimum_required(VERSION 3.20)
project(rcsw_consumer LANGUAGES C)
find_package(rcsw REQUIRED)
CMAKE
  run configure_consumer "${dir}"
  assert_success
}

@test "rcsw::<component> exists for every component under test" {
  local dir="${BATS_TEST_TMPDIR}/targets"
  mkdir -p "${dir}"
  {
    echo "cmake_minimum_required(VERSION 3.20)"
    echo "project(rcsw_consumer LANGUAGES C)"
    echo "find_package(rcsw REQUIRED COMPONENTS ${RCSW_COMPONENTS})"
    echo "foreach(c ${RCSW_COMPONENTS})"
    echo "  if(NOT TARGET rcsw::\${c})"
    echo "    message(FATAL_ERROR \"rcsw::\${c} is not defined\")"
    echo "  endif()"
    echo "endforeach()"
  } > "${dir}/CMakeLists.txt"
  run configure_consumer "${dir}"
  assert_success
}

@test "requesting a component that isn't installed fails" {
  local missing="" c
  for c in ${RCSW_OPTIONAL_COMPONENTS}; do
    if [ ! -d "${RCSW_PREFIX}/lib/cmake/rcsw_${c}" ]; then
      missing=${c}
      break
    fi
  done
  [ -n "${missing}" ] || skip "every component is installed"

  local dir="${BATS_TEST_TMPDIR}/missing"
  mkdir -p "${dir}"
  cat > "${dir}/CMakeLists.txt" << CMAKE
cmake_minimum_required(VERSION 3.20)
project(rcsw_consumer LANGUAGES C)
find_package(rcsw REQUIRED COMPONENTS ${missing})
CMAKE
  run configure_consumer "${dir}"
  assert_failure
  assert_output --partial "Requested components not installed: ${missing}"
}
