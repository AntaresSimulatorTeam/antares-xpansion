# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================
find_package(MPI REQUIRED)
find_package(MPI REQUIRED COMPONENTS C)
message("MPI_C_INCLUDE_PATH ${MPI_C_INCLUDE_PATH}")
message("MPI_C_INCLUDE_DIRS ${MPI_C_INCLUDE_DIRS}")
message("MPI_C_LIBRARIES ${MPI_C_LIBRARIES}")


if (UNIX)
    set(CMAKE_CXX_COMPILER ${MPI_CXX_COMPILER})
endif ()
add_library(benders_by_batch_core)
target_sources(benders_by_batch_core PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/BatchCollection.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/BendersByBatch.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/RandomBatchShuffler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_by_batch/BatchCollection.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_by_batch/BendersByBatch.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_by_batch/RandomBatchShuffler.h
)

target_include_directories(benders_by_batch_core
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(benders_by_batch_core
        PUBLIC
        antaresXpansion::benders_core
        antaresXpansion::benders_sequential_core
        antaresXpansion::benders_mpi_core
        antaresXpansion::helpers
        antaresXpansion::output_core
        solvers
)

add_library(${PROJECT_NAME}::benders_by_batch_core ALIAS benders_by_batch_core)
install(DIRECTORY include/antares-xpansion
        DESTINATION "include"
)