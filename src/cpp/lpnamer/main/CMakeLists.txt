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
	${CMAKE_CURRENT_SOURCE_DIR}/ProblemGenerationExeOptions.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ProblemGeneration.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/ProblemGeneration.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/ProblemGenerationOptions.h
)

target_link_libraries (problem_generation_main 
	PRIVATE 
		${PROJECT_NAME}::lp_namer_helper
		${PROJECT_NAME}::lp_namer_model
		${PROJECT_NAME}::lp_namer_input_reader
		${PROJECT_NAME}::lp_namer_problem_modifier
		Boost::boost Boost::program_options
		helpers)

get_target_property(helpers_include helpers INTERFACE_INCLUDE_DIRECTORIES)

target_include_directories(problem_generation_main
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${helpers_include}
)


add_library (${PROJECT_NAME}::problem_generation_main ALIAS problem_generation_main)

