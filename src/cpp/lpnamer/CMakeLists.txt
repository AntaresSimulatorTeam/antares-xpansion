# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/model")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/input_reader")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/problem_modifier")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/helper")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/main")
