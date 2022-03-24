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

add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/helpers")
add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/lpnamer")
add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/benders")
add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/multisolver_interface")
add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/sensitivity")

add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/exe")
