# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

add_library(grid_search_lib STATIC)

target_sources(grid_search_lib PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/GridSearch.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/grid_search/GridSearch.h
)

target_link_libraries(grid_search_lib
        PUBLIC
        antaresXpansion::benders_core
)

target_include_directories(grid_search_lib
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/cpp/xpansion_interfaces/include>
)

add_library(${PROJECT_NAME}::grid_search_lib ALIAS grid_search_lib)
install(DIRECTORY include/antares-xpansion
        DESTINATION "include"
)
