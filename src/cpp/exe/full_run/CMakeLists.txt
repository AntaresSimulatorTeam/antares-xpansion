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
	target_link_libraries (full_run
		PRIVATE
		${PROJECT_NAME}::benders_mpi_core
)
endif ()
get_target_property(full_run_lib_include full_run_lib INTERFACE_INCLUDE_DIRECTORIES)

target_include_directories(full_run PRIVATE
${full_run_lib_include})

target_link_libraries (full_run
		PRIVATE
		${PROJECT_NAME}::full_run_lib
		${PROJECT_NAME}::problem_generation_main
		${PROJECT_NAME}::benders_sequential_core
		${PROJECT_NAME}::xpansion_study_updater_lib
)

install(TARGETS full_run DESTINATION bin)