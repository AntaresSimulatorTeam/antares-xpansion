# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/merge")
add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/lpnamer")
add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/study-updater")
add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/analyse")

if (USE_MPI)
	message (STATUS "Generating mpi executable!")
	add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/benders_mpi")
endif ()

if (USE_SEQUENTIAL)
	message (STATUS "Generating sequential executable!")
	add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/benders_sequential")
endif ()
