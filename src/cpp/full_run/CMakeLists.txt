# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

# ---------------------------------------------------------------------------
# Full Run Exe
# ---------------------------------------------------------------------------

add_library (full_run_lib STATIC
	${CMAKE_CURRENT_SOURCE_DIR}/FullRunOptionsParser.cpp
)


target_link_libraries (full_run_lib
		PRIVATE
		${PROJECT_NAME}::problem_generation_main
)
get_target_property(problem_generation_main_include problem_generation_main INTERFACE_INCLUDE_DIRECTORIES)

target_include_directories (full_run_lib
		PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/include
		${problem_generation_main_include}
	)
add_library (${PROJECT_NAME}::full_run_lib ALIAS full_run_lib)
