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
		PUBLIC
		${PROJECT_NAME}::problem_generation_main
		${PROJECT_NAME}::factories
)
get_target_property(problem_generation_main_include problem_generation_main INTERFACE_INCLUDE_DIRECTORIES)
get_target_property(factories_include factories INTERFACE_INCLUDE_DIRECTORIES)

target_include_directories (full_run_lib
		PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/include
		${problem_generation_main_include}
		${factories_include}
	)
add_library (${PROJECT_NAME}::full_run_lib ALIAS full_run_lib)
