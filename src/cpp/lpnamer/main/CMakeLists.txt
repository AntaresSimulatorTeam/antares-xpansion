# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

# ---------------------------------------------------------------------------
# Problem Generation  runner lib
# ---------------------------------------------------------------------------

add_library (problem_generation_main STATIC
	${CMAKE_CURRENT_SOURCE_DIR}/ProblemGenerationMain.cpp
)

target_link_libraries (problem_generation_main 
	PRIVATE 
		${PROJECT_NAME}::lp_namer_helper
		${PROJECT_NAME}::lp_namer_model
		${PROJECT_NAME}::lp_namer_input_reader
		${PROJECT_NAME}::lp_namer_problem_modifier
		Boost::boost Boost::program_options)


	target_include_directories (problem_generation_main
		PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/include
	)


add_library (${PROJECT_NAME}::problem_generation_main ALIAS problem_generation_main)

