# ===========================================================================
# CMake configuration
# ===========================================================================

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/config.h.in generatedInclude/config.h)
include_directories (${CMAKE_CURRENT_BINARY_DIR}/generatedInclude)

# ===========================================================================
# Targets
# ===========================================================================

add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/src_helpers")
add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/src_lpnamer")
add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/src_sequential")

if (USE_MPI)
	add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/src_mpi")
endif ()

add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/src_enzo_test")
add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/src_test")
add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/exe")
