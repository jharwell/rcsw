# ##############################################################################
# Third-party packages
# ##############################################################################
cpmaddpackage(
  NAME
  Catch2
  GITHUB_REPOSITORY
  catchorg/Catch2
  SYSTEM
  VERSION
  3.7.0)

# 2026-04-29 [JRH]: We always build as static lib to avoid annoying issues with
# putchar_() needing to be exported from RCSW.
cpmaddpackage(
  NAME
  printf
  GITHUB_REPOSITORY
  eyalroz/printf
  VERSION
  6.3.0
  OPTIONS
  "BUILD_SHARED_LIBS OFF"
  "PRINTF_SUPPORT_DECIMAL_SPECIFIERS ON"
  "PRINTF_SUPPORT_EXPONENTIAL_SPECIFIERS ON"
  "PRINTF_SUPPORT_WRITEBACK_SPECIFIER ON"
  "PRINTF_SUPPORT_LONG_LONG ON"
  "PRINTF_NTOA_BUFFER_SIZE 32"
  "PRINTF_DEFAULT_FLOAT_PRECISION 6"
  "PRINTF_MAX_INTEGRAL_DIGITS_FOR_DECIMAL 9"
  "PRINTF_CHECK_FOR_NUL_IN_FORMAT_SPECIFIER ON")

set(LIBRA_TEST_HARNESS_LIBS Catch2::Catch2WithMain printf::printf)
set(LIBRA_ANALYSIS_LANGUAGE C)
set(LIBRA_CLANG_EXTRA_ARGS -fcomment-block-commands=rcswdoc)

# rcsw needs printf's source tree (see _rcsw_apply_includes() in targets.cmake).
# CPM leaves printf_SOURCE_DIR unset when it uses an installed printf package
# instead, e.g. with CPM_USE_LOCAL_PACKAGES on.
if(NOT printf_SOURCE_DIR)
  rcsw_message(
    FATAL_ERROR
    "printf_SOURCE_DIR is not set; was printf found as an installed package instead of added by CPM?"
  )
endif()

# FORCE printf target to compile using C99 instead of inheriting a global C90
# constraint. On 32 bit platforms this results in compile errors because of long
# long support in that standard version.
if(TARGET printf)
  set_target_properties(
    printf
    PROPERTIES C_STANDARD 99
               C_STANDARD_REQUIRED ON
               C_EXTENSIONS ON)
  set_target_properties(printf PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()
