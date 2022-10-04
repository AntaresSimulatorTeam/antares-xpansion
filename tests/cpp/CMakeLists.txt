# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# External requirements
# ===========================================================================
find_package (GTest REQUIRED)

message ("Gtest found here: " ${GTEST_INCLUDE_DIRS})

# ===========================================================================
# Targets
# ===========================================================================

# --------------------------------------------------------------------------
# unit tests
# --------------------------------------------------------------------------
add_subdirectory(solvers_interface)
add_subdirectory(logger)
add_subdirectory(json_output_writer)
add_subdirectory(helpers)
add_subdirectory(lp_namer)
add_subdirectory(sensitivity)
add_subdirectory(restart_benders)
add_subdirectory(benders)
add_subdirectory(full_run)
