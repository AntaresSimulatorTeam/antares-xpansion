# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

# ---------------------------------------------------------------------------
# Problem Generation Exe
# ---------------------------------------------------------------------------

add_executable (lp_namer 
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_link_libraries (lp_namer
		PRIVATE
		${PROJECT_NAME}::problem_generation_main
		${PROJECT_NAME}::lp_namer_helper
		Boost::serialization)

install(TARGETS lp_namer DESTINATION bin)
