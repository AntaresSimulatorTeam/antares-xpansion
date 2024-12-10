# ===========================================================================
# CMake configuration
# ===========================================================================

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/config.h.in generatedInclude/config.h)
include_directories (${CMAKE_CURRENT_BINARY_DIR}/generatedInclude)

if(POLICY CMP0074)
	cmake_policy(SET CMP0074 NEW)
endif()


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

add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/xpansion_interfaces")
add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/helpers")
add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/lpnamer")
add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/benders")
add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/multisolver_interface")
add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/sensitivity")
add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/full_run")
add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/study-updater")

add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/exe")
