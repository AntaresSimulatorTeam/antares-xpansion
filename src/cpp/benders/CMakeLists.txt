

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/benders_core")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/logger")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/benders_sequential")


if (USE_MPI)
	add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/benders_mpi")
endif ()
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/factories")
