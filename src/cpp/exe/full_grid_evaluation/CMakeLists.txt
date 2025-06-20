# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

# ---------------------------------------------------------------------------
# Problem Generation Exe
# ---------------------------------------------------------------------------

add_executable (full_grid_evaluation_exe
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_link_libraries (full_grid_evaluation_exe
		PRIVATE
		${PROJECT_NAME}::lp_namer_input_reader
		${PROJECT_NAME}::problem_generation_main
		${PROJECT_NAME}::lp_namer_helper
		${PROJECT_NAME}::grid_evaluator_lib
		${PROJECT_NAME}::factories
		
)

install(TARGETS full_grid_evaluation_exe DESTINATION bin)
