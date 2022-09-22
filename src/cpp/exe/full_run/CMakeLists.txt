# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

# ---------------------------------------------------------------------------
# Full Run Exe
# ---------------------------------------------------------------------------

add_executable (full_run
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

if (USE_MPI)
	find_package(MPI REQUIRED)
	if (MPI_FOUND)
		message("CMAKE_CXX_COMPILER: ${MPI_CXX_COMPILER}")
		set (CMAKE_CXX_COMPILER ${MPI_CXX_COMPILER})
	endif (MPI_FOUND)
endif ()

target_link_libraries (full_run
		PRIVATE
		${PROJECT_NAME}::lp_namer_helper
		${PROJECT_NAME}::lp_namer_model
		${PROJECT_NAME}::lp_namer_input_reader
		${PROJECT_NAME}::lp_namer_problem_modifier
		Boost::boost Boost::program_options
		${PROJECT_NAME}::benders_core
		${PROJECT_NAME}::benders_sequential_core
		${PROJECT_NAME}::factories
		${PROJECT_NAME}::output_core
		logger_lib)

install(TARGETS full_run DESTINATION bin)