# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

add_library(bellman_values_lib STATIC)

target_sources(bellman_values_lib PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/BellmanValues.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ReservoirManagement.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/bellman_values/BellmanValues.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/bellman_values/ReservoirManagement.h
)

target_link_libraries(bellman_values_lib
        PUBLIC
        antaresXpansion::grid_evaluator_lib
)

target_include_directories(bellman_values_lib
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

add_library(${PROJECT_NAME}::bellman_values_lib ALIAS bellman_values_lib)
install(DIRECTORY include/antares-xpansion
        DESTINATION "include"
)
