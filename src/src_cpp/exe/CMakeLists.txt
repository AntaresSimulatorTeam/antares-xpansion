# ===========================================================================
# CMake configuration
# ===========================================================================
CMAKE_MINIMUM_REQUIRED(VERSION 3.14)


# ---------------------------------------------------------------------------
# Executable Targets
# ---------------------------------------------------------------------------
add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/merge")

add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/analyse")

IF( USE_MPI )
	message(STATUS "Generating mpi executable!")
	add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/exe_mpi")
ENDIF()

IF(NOT NO_SEQUENTIAL )
	message(STATUS "Generating sequential executable!")
	add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/exe_sequential")
ENDIF()











