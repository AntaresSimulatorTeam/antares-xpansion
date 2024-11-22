# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

# ---------------------------------------------------------------------------
# Full Run Exe
# ---------------------------------------------------------------------------

add_executable(full_run
        ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

find_package(MPI REQUIRED)

if (UNIX)
    set(CMAKE_CXX_COMPILER ${MPI_CXX_COMPILER})
endif ()

target_link_libraries(full_run
        PRIVATE
        ${PROJECT_NAME}::benders_mpi_core
)

target_link_libraries(full_run
        PRIVATE
        ${PROJECT_NAME}::full_run_lib
        ${PROJECT_NAME}::problem_generation_main
        ${PROJECT_NAME}::benders_sequential_core
        ${PROJECT_NAME}::xpansion_study_updater_lib
        antaresXpansion::helpers
)

if (TBB_FOUND)
    target_link_libraries(full_run PRIVATE tbb)
endif ()

install(TARGETS full_run DESTINATION bin)