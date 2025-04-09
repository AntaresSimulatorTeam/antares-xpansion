# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

add_library(valeurs_usage_lib STATIC)

target_sources(valeurs_usage_lib PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/ValeursUsage.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/valeurs_usage/ValeursUsage.h
)

target_link_libraries(valeurs_usage_lib
        PUBLIC
        antaresXpansion::benders_core
        antaresXpansion::output_core
)

target_include_directories(valeurs_usage_lib
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/cpp/xpansion_interfaces/include>
)

add_library(${PROJECT_NAME}::valeurs_usage_lib ALIAS valeurs_usage_lib)
install(DIRECTORY include/antares-xpansion
        DESTINATION "include"
)
