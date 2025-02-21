# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

find_package(MPI REQUIRED COMPONENTS C)
message("MPI_C_INCLUDE_PATH ${MPI_C_INCLUDE_PATH}")
message("MPI_C_INCLUDE_DIRS ${MPI_C_INCLUDE_DIRS}")
message("MPI_C_LIBRARIES ${MPI_C_LIBRARIES}")

find_package(MPI REQUIRED)

if (UNIX)
    set(CMAKE_CXX_COMPILER ${MPI_CXX_COMPILER})
endif ()

add_library(grid_search_core)

target_sources(grid_search_core PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/GridSearch.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/grid_search/GridSearch.h
)

target_link_libraries(grid_search_core
        PUBLIC
        antaresXpansion::benders_core
        antaresXpansion::benders_sequential_core
        antaresXpansion::output_core
        Boost::mpi
        MPI::MPI_C
        ${PROJECT_NAME}::factories
        antaresXpansion::logger_lib
        Boost::serialization
)

target_include_directories(grid_search_core
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

add_library(${PROJECT_NAME}::grid_search_core ALIAS grid_search_core)
install(DIRECTORY include/antares-xpansion
        DESTINATION "include"
)
