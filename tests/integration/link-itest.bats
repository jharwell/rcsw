#!/usr/bin/env bats
#
# A program using each part of rcsw links against the installed library of the
# component that contains it (see owner() in helpers.bash), and runs. Sources
# are in consumers/<part>/main.c.

setup() {
  load helpers
  common_setup
}

@test "link: al" {
  skip_unless_owner_under_test al
  run check_link al
  assert_success
}

@test "link: core" {
  skip_unless_owner_under_test core
  run check_link core
  assert_success
}

@test "link: algorithm" {
  skip_unless_owner_under_test algorithm
  run check_link algorithm
  assert_success
}

@test "link: ds" {
  skip_unless_owner_under_test ds
  run check_link ds
  assert_success
}

@test "link: er" {
  skip_unless_owner_under_test er
  [ "${RCSW_ER_PLUGIN:-LOG4CL}" = "LOG4CL" ] || skip "built with ER plugin ${RCSW_ER_PLUGIN}"
  run check_link er
  assert_success
}

@test "link: multiprocess" {
  skip_unless_owner_under_test multiprocess
  run check_link multiprocess
  assert_success
}

@test "link: multithread" {
  skip_unless_owner_under_test multithread
  run check_link multithread
  assert_success
}

@test "link: stdio" {
  skip_unless_owner_under_test stdio
  run check_link stdio
  assert_success
}

@test "link: swbus" {
  skip_unless_owner_under_test swbus
  run check_link swbus
  assert_success
}

@test "link: tool" {
  skip_unless_owner_under_test tool
  run check_link tool
  assert_success
}

@test "link: utils" {
  skip_unless_owner_under_test utils
  run check_link utils
  assert_success
}
