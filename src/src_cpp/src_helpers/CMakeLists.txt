# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# External requirements
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

# --------------------------------------------------------------------------
# unit tests
# --------------------------------------------------------------------------

add_library (helpers STATIC
	${CMAKE_CURRENT_SOURCE_DIR}/ortools_utils.cc
	)

if (NOT ORTOOLS_RECENT)
	target_add_definitions (helpers
		PRIVATE
			ORTOOLS_PRE_V71
	)
endif ()

target_include_directories (helpers
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
		${ORTOOLS_INCLUDE_DIRS}
)

target_link_libraries (helpers 
	PUBLIC
		ortools
)
