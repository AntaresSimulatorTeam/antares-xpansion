# ===========================================================================
# CMake configuration
# ===========================================================================

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/config.h.in generatedInclude/config.h)
include_directories (${CMAKE_CURRENT_BINARY_DIR}/generatedInclude)

if(POLICY CMP0074)
	cmake_policy(SET CMP0074 NEW)
endif()

# ===========================================================================
# Targets
# ===========================================================================

add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/src_helpers")
add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/src_lpnamer")
add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/src_sequential")
add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/src_multisolver_interface")

if (USE_MPI)
	add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/src_mpi")
endif ()

add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/src_test")
add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/exe")
