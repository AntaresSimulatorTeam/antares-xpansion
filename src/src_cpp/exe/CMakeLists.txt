# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/merge")
add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/exe_lpnamer")
add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/analyse")

if (USE_MPI)
	message (STATUS "Generating mpi executable!")
	add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/exe_mpi")
endif ()

if (USE_SEQUENTIAL)
	message (STATUS "Generating sequential executable!")
	add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/exe_sequential")
endif ()
