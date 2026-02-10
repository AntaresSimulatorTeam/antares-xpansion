# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

# ---------------------------------------------------------------------------
# MPI Benders Exe
# ---------------------------------------------------------------------------
find_package(Antares REQUIRED)
add_executable(benders-with-problem-generation
        ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

find_package(MPI REQUIRED)
if (UNIX)
    set(CMAKE_CXX_COMPILER ${MPI_CXX_COMPILER})
endif ()

target_link_libraries(benders-with-problem-generation
        ${PROJECT_NAME}::benders_mpi_core
        antaresXpansion::factories
        Antares::single_problem_api
)

install(TARGETS benders-with-problem-generation DESTINATION bin)
