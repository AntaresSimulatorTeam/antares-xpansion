# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

# ---------------------------------------------------------------------------
# Sequential Benders Exe
# ---------------------------------------------------------------------------

add_executable (lp_namer 
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_link_libraries (lp_namer 
	PRIVATE 
		${PROJECT_NAME}::lp_namer_core
		Boost::boost Boost::program_options)

install(TARGETS lp_namer DESTINATION bin)
