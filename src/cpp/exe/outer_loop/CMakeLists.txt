# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

# ---------------------------------------------------------------------------
# MPI Benders Exe
# ---------------------------------------------------------------------------

add_executable(outer_loop
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

target_link_libraries(outer_loop
        ${PROJECT_NAME}::outer_loop_lib
        antaresXpansion::factories
)

install(TARGETS outer_loop DESTINATION bin)
