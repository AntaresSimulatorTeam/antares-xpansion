# Option to enable the strategy pattern submodule (Phase 1). Default OFF to avoid breaking builds.
option(ENABLE_BENDERS_STRATEGY "Enable the benders strategy library (phase1)" OFF)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/benders_core")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/logger")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/benders_sequential")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/benders_by_batch")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/outer_loop")

# Strategy pattern support (Phase 1) - non invasive addition (optional)
if(ENABLE_BENDERS_STRATEGY)
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/strategy")
endif()


add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/benders_mpi")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/factories")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/output")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/merge_mps")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/merge_master_mps")
