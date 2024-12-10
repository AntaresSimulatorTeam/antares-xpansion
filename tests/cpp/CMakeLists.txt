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
# Check if ccache is installed and available in PATH
find_program(CCACHE_EXECUTABLE ccache)

if (CCACHE_EXECUTABLE)
    message(STATUS "Using ccache as the compiler launcher")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_EXECUTABLE}")
    set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_EXECUTABLE}")
else ()
    message(WARNING "ccache not found. Proceeding without it.")
endif ()
endif()
# ===========================================================================
# Targets
# ===========================================================================

# --------------------------------------------------------------------------
# unit tests
# --------------------------------------------------------------------------
add_subdirectory(benders)
add_subdirectory(full_run)
add_subdirectory(helpers)
add_subdirectory(json_output_writer)
add_subdirectory(logger)
add_subdirectory(lp_namer)
add_subdirectory(outer_loop)
add_subdirectory(restart_benders)
add_subdirectory(sensitivity)
add_subdirectory(solvers_interface)
add_subdirectory(study_updater)
add_subdirectory(tests_utils)
add_subdirectory(zip_mps)
