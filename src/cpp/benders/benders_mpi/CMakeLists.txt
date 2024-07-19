# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

find_package (MPI REQUIRED COMPONENTS C)
message ("MPI_C_INCLUDE_PATH ${MPI_C_INCLUDE_PATH}")
message ("MPI_C_INCLUDE_DIRS ${MPI_C_INCLUDE_DIRS}")
message ("MPI_C_LIBRARIES ${MPI_C_LIBRARIES}")

find_package(MPI REQUIRED)

if(UNIX)
set(CMAKE_CXX_COMPILER ${MPI_CXX_COMPILER})
endif()

add_library (benders_mpi_core STATIC
	${CMAKE_CURRENT_SOURCE_DIR}/common_mpi.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/BendersMPI.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/OuterLoopBenders.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/BendersMpiOuterLoop.cpp
)

target_link_libraries (benders_mpi_core
	PUBLIC
		benders_core
		benders_sequential_core
		output_core
		Boost::mpi
		MPI::MPI_C
		${PROJECT_NAME}::factories
		logger_lib
		Boost::serialization
)

#if (WIN32)
#	target_link_libraries (benders_mpi_core PUBLIC ${MPI_C_LIBRARIES})
#endif ()

target_include_directories (benders_mpi_core
#	PUBLIC ${MPI_C_INCLUDE_DIRS}
		PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/include
	INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library (${PROJECT_NAME}::benders_mpi_core ALIAS benders_mpi_core)
