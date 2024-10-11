# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

# ---------------------------------------------------------------------------
# Merge MPS Exe
# ---------------------------------------------------------------------------

add_executable (merge_mps
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)
target_link_libraries (merge_mps 
	PUBLIC
		${PROJECT_NAME}::merge_mps_core
		${PROJECT_NAME}::factories
		${PROJECT_NAME}::output_core
		helpers
		antaresXpansion::logger_lib
)


install(TARGETS merge_mps DESTINATION bin)
