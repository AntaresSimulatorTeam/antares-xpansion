# ===========================================================================
# CMake configuration
# ===========================================================================
CMAKE_MINIMUM_REQUIRED(VERSION 3.14)

# ===========================================================================
# Targets
# ===========================================================================
add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/src_helpers")

add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/src_sequential")

IF( USE_MPI )
	add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/src_mpi")
ENDIF( USE_MPI )

add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/src_enzo_test")

add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/exe")

add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/src_test")