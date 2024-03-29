# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# External requirements
# ===========================================================================
find_package (GTest REQUIRED)

message ("Gtest found here: " ${GTEST_INCLUDE_DIRS})
find_package(MPI REQUIRED)

if(UNIX)
set(CMAKE_CXX_COMPILER ${MPI_CXX_COMPILER})
endif()
# ===========================================================================
# Targets
# ===========================================================================

# --------------------------------------------------------------------------
# unit tests
# --------------------------------------------------------------------------
add_subdirectory(tests_utils)
add_subdirectory(solvers_interface)
add_subdirectory(logger)
add_subdirectory(json_output_writer)
add_subdirectory(helpers)
add_subdirectory(lp_namer)
add_subdirectory(sensitivity)
add_subdirectory(restart_benders)
add_subdirectory(zip_mps)
add_subdirectory(benders)
add_subdirectory(full_run)
add_subdirectory(ext_loop)
