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

add_library(benders_mpi_core)

target_sources(benders_mpi_core PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/BendersMPI.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/BendersMpiOuterLoop.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/OuterLoopBenders.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/common_mpi.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_mpi/BendersMPI.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_mpi/BendersMpiOuterLoop.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_mpi/OuterLoopBenders.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_mpi/common_mpi.h

)

target_link_libraries(benders_mpi_core
        PUBLIC
        benders_core
        antaresXpansion::benders_sequential_core
        antaresXpansion::output_core
        Boost::mpi
        MPI::MPI_C
        ${PROJECT_NAME}::factories
        antaresXpansion::logger_lib
        Boost::serialization
)

target_include_directories(benders_mpi_core
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

add_library(${PROJECT_NAME}::benders_mpi_core ALIAS benders_mpi_core)
install(DIRECTORY include/antares-xpansion
        DESTINATION "include"
)
