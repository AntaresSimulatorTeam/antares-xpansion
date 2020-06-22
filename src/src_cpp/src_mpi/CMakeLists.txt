# ===========================================================================
# CMake configuration
# ===========================================================================
CMAKE_MINIMUM_REQUIRED(VERSION 3.14)

SET(
	MPI_FILES

	${CMAKE_CURRENT_SOURCE_DIR}/common_mpi.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/BendersMPI.cpp

	${CMAKE_CURRENT_SOURCE_DIR}/BendersMPI.h
	${CMAKE_CURRENT_SOURCE_DIR}/common_mpi.h
)

ADD_LIBRARY(mpicore STATIC ${MPI_FILES} )

target_link_libraries (mpicore
	PUBLIC
		sequentialcore
)

target_include_directories(mpicore
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
)

add_library (${PROJECT_NAME}::mpicore ALIAS mpicore)