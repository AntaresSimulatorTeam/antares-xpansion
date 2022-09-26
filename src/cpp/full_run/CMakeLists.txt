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
#get_target_property(JSON_INC_PATH jsoncpp_lib INTERFACE_INCLUDE_DIRECTORIES)


target_link_libraries (full_run_lib
		PRIVATE
		${PROJECT_NAME}::problem_generation_main
)
target_include_directories (problem_generation_main
		PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/include
	)
add_library (${PROJECT_NAME}::full_run_lib ALIAS full_run_lib)
