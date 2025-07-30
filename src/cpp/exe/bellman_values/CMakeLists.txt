# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

# ---------------------------------------------------------------------------
# Problem Generation Exe
# ---------------------------------------------------------------------------

add_executable (bellman_values_exe
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_link_libraries (bellman_values_exe
		PRIVATE
		${PROJECT_NAME}::lp_namer_input_reader
		${PROJECT_NAME}::lp_namer_problem_modifier
		${PROJECT_NAME}::problem_generation_main
		${PROJECT_NAME}::lp_namer_helper
		${PROJECT_NAME}::grid_evaluator_lib
		${PROJECT_NAME}::bellman_values_lib
		${PROJECT_NAME}::factories
		
)

install(TARGETS bellman_values_exe DESTINATION bin)
