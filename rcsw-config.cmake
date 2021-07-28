################################################################################
# Configuration Options                                                        #
################################################################################
set(${target}_CHECK_LANGUAGE "C")

################################################################################
# Includes                                                                     #
################################################################################
set(${target}_INCLUDE_DIRS "${${target}_INC_PATH}")

################################################################################
# Submodules                                                                   #
################################################################################

if (NOT LIBRA_MPI)
  list(FILTER ${target}_SRC EXCLUDE REGEX "multiprocess")
endif()

# We glob the whole project by default; need to exclude the test files from the
# compiled library.
list(FILTER ${target}_SRC EXCLUDE REGEX "-test.cpp")

################################################################################
# Libraries                                                                    #
################################################################################
set(${target}_LIBRARIES
  )

if (NOT TARGET ${target})
  add_library(${target} STATIC ${${target}_SRC})
  target_include_directories(${target} PUBLIC ${${target}_INCLUDE_DIRS})
  target_include_directories(${target} SYSTEM PRIVATE ext)
  target_link_libraries(${target} ${${target}_LIBRARIES})
endif()


################################################################################
# Exports                                                                      #
################################################################################
if (NOT IS_ROOT_PROJECT)
  set(${target}_INCLUDE_DIRS "${${target}_INCLUDE_DIRS}" PARENT_SCOPE)
  set(${target}_LIBRARIES "${${target}_LIBRARIES}" PARENT_SCOPE)
  set(${target}_LIBRARY_DIRS "${${target}_LIBRARY_DIRS}" PARENT_SCOPE)
endif()
