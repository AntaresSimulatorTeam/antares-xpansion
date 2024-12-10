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
    # Check if ccache is installed and available in PATH
    find_program(CCACHE_EXECUTABLE ccache)

    if (CCACHE_EXECUTABLE)
        message(STATUS "Using ccache as the compiler launcher")
        set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_EXECUTABLE}")
        set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_EXECUTABLE}")
    else ()
        message(WARNING "ccache not found. Proceeding without it.")
    endif ()
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

target_link_libraries(full_run PRIVATE TBB::tbb)

install(TARGETS full_run DESTINATION bin)