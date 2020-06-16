# ===========================================================================
# CMake configuration
# ===========================================================================

cmake_minimum_required (VERSION 3.14.0)

# ===========================================================================
# External requirements
# ===========================================================================

# --------------------------------------------------------------------------
# unit tests
# --------------------------------------------------------------------------
set (FILES_SRCS
	${CMAKE_CURRENT_SOURCE_DIR}/ortools_utils.cc
	)

add_library (helpers STATIC ${FILES_SRCS})

target_include_directories (helpers
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
		${ORTOOLS_INCLUDE_DIRS}
	)

target_link_libraries (helpers PUBLIC
		ortools
	)