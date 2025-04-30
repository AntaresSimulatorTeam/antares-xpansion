# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

# ---------------------------------------------------------------------------
# Merge Master MPS Exe
# ---------------------------------------------------------------------------

add_executable (merge_master_mps
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)
target_link_libraries (merge_master_mps
	PUBLIC
		${PROJECT_NAME}::merge_mps_core
		${PROJECT_NAME}::factories
		${PROJECT_NAME}::output_core
		antaresXpansion::helpers
		antaresXpansion::logger_lib
)


install(TARGETS merge_master_mps DESTINATION bin)
