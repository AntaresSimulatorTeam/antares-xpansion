# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

find_package(yaml-cpp CONFIG REQUIRED)

add_library(bellman_values_lib STATIC)

target_sources(bellman_values_lib PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/BellmanValues.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/BellmanValuesExeOptions.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/PenaltiesConfigReader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/bellman_values/BellmanValues.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/bellman_values/BellmanValuesExeOptions.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/bellman_values/PenaltiesConfigReader.h
)

target_link_libraries(bellman_values_lib
        PUBLIC
        antaresXpansion::grid_evaluator_lib
        antaresXpansion::helpers
        yaml-cpp::yaml-cpp
)

target_include_directories(bellman_values_lib
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

add_library(${PROJECT_NAME}::bellman_values_lib ALIAS bellman_values_lib)
install(DIRECTORY include/antares-xpansion
        DESTINATION "include"
)
