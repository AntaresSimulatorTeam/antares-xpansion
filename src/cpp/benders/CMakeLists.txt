

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/benders_core")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/logger")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/benders_sequential")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/benders_by_batch")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/adequacy_criterion")


add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/benders_mpi")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/factories")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/output")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/merge_mps")
