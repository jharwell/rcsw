#!/usr/bin/env bats
#
# Every top-level header in each include/rcsw/<dir> compiles on its own, using
# only what find_package(rcsw COMPONENTS <component>) provides for the
# component that owns <dir> (see owner() in helpers.bash).

setup() {
  load helpers
  common_setup
}

@test "headers: al" {
  skip_unless_owner_under_test al
  run check_headers al
  assert_success
}

@test "headers: core" {
  skip_unless_owner_under_test core
  run check_headers core
  assert_success
}

@test "headers: algorithm" {
  skip_unless_owner_under_test algorithm
  run check_headers algorithm
  assert_success
}

@test "headers: ds" {
  skip_unless_owner_under_test ds
  run check_headers ds
  assert_success
}

@test "headers: er" {
  skip_unless_owner_under_test er
  run check_headers er
  assert_success
}

@test "headers: multiprocess" {
  skip_unless_owner_under_test multiprocess
  run check_headers multiprocess
  assert_success
}

@test "headers: multithread" {
  skip_unless_owner_under_test multithread
  run check_headers multithread
  assert_success
}

@test "headers: stdio" {
  skip_unless_owner_under_test stdio
  run check_headers stdio
  assert_success
}

@test "headers: swbus" {
  skip_unless_owner_under_test swbus
  run check_headers swbus
  assert_success
}

@test "headers: tool" {
  skip_unless_owner_under_test tool
  run check_headers tool
  assert_success
}

@test "headers: utils" {
  skip_unless_owner_under_test utils
  run check_headers utils
  assert_success
}

@test "headers: version" {
  skip_unless_owner_under_test version
  run check_headers version
  assert_success
}
