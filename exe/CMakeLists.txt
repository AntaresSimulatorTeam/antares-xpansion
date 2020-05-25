CMAKE_MINIMUM_REQUIRED(VERSION 3.14)

add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/merge")

add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/analyse")

IF( USE_MPI )
	add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/exe_mpi")
ELSE( USE_MPI )
	add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/exe_cpp")
ENDIF( USE_MPI )











