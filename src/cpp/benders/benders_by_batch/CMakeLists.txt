# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================
find_package(MPI REQUIRED)
find_package (MPI REQUIRED COMPONENTS C)
message ("MPI_C_INCLUDE_PATH ${MPI_C_INCLUDE_PATH}")
message ("MPI_C_INCLUDE_DIRS ${MPI_C_INCLUDE_DIRS}")
message ("MPI_C_LIBRARIES ${MPI_C_LIBRARIES}")


if(UNIX)
set(CMAKE_CXX_COMPILER ${MPI_CXX_COMPILER})
endif()
add_library (benders_by_batch_core STATIC
	${CMAKE_CURRENT_SOURCE_DIR}/BendersByBatch.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/BatchCollection.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/RandomBatchShuffler.cpp
        )

target_include_directories (benders_by_batch_core
		PUBLIC
			${CMAKE_CURRENT_SOURCE_DIR}/include
		PUBLIC
			${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries (benders_by_batch_core
	PUBLIC
        benders_core
		antaresXpansion::benders_sequential_core
		antaresXpansion::benders_mpi_core
		helpers
		antaresXpansion::output_core
		solvers
)

add_library (${PROJECT_NAME}::benders_by_batch_core ALIAS benders_by_batch_core)