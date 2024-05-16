# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/merge")
add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/lpnamer")
add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/study-updater")
add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/sensitivity")
add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/full_run")
add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/antares_archive_updater")

add_subdirectory ("${CMAKE_CURRENT_SOURCE_DIR}/benders")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/outer_loop")

