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
    )

target_include_directories (lp_namer_helper
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
)


add_library (${PROJECT_NAME}::lp_namer_helper ALIAS lp_namer_helper)
