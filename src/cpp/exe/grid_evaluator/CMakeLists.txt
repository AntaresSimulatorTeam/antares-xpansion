# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

# ---------------------------------------------------------------------------
# Grid Search Exe
# ---------------------------------------------------------------------------

add_executable (grid_evaluator_exe
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_include_directories (grid_evaluator_exe PUBLIC
)

target_link_libraries (grid_evaluator_exe
		antaresXpansion::grid_evaluator_lib
		antaresXpansion::factories
		antaresXpansion::output_core
		)


install(TARGETS grid_evaluator_exe DESTINATION bin)
