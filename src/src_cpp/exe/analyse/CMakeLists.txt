# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

# ---------------------------------------------------------------------------
# Analyse MPS Exe
# ---------------------------------------------------------------------------

add_executable (analyse_mps
	${CMAKE_CURRENT_SOURCE_DIR}/analyse_mps.cpp
)

target_include_directories (analyse_mps
	PUBLIC
		${ORTOOLS_INCLUDE_DIRS}
)

target_link_libraries (analyse_mps
	PUBLIC
		${PROJECT_NAME}::sequentialcore
		helpers
)
