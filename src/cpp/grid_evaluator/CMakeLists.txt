# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

add_library(grid_evaluator_lib STATIC)

target_sources(grid_evaluator_lib PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/GridEvaluator.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/GridCollection.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/grid_evaluator/GridEvaluator.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/grid_evaluator/GridCollection.h
)

target_link_libraries(grid_evaluator_lib
        PUBLIC
        antaresXpansion::benders_core
        antaresXpansion::output_core
)

target_include_directories(grid_evaluator_lib
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/cpp/xpansion_interfaces/include>
)

add_library(${PROJECT_NAME}::grid_evaluator_lib ALIAS grid_evaluator_lib)
install(DIRECTORY include/antares-xpansion
        DESTINATION "include"
)
