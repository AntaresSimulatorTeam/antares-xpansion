# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

add_library (lp_namer_helper STATIC
	${CMAKE_CURRENT_SOURCE_DIR}/common_lpnamer.h
	${CMAKE_CURRENT_SOURCE_DIR}/ColumnToChange.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/ColumnToChange.h
	${CMAKE_CURRENT_SOURCE_DIR}/ProblemGenerationLogger.h
	${CMAKE_CURRENT_SOURCE_DIR}/ProblemGenerationLogger.cpp
    )

target_include_directories (lp_namer_helper
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries (lp_namer_helper 
	PUBLIC
		helpers
)

add_library (${PROJECT_NAME}::lp_namer_helper ALIAS lp_namer_helper)
