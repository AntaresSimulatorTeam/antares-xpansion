# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

add_library(output_core)

target_sources(output_core PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/VoidWriter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/JsonWriter.cpp
        include/antares-xpansion/benders/output/VoidWriter.h
        include/antares-xpansion/benders/output/JsonWriter.h
)

target_include_directories(output_core
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(output_core
        PUBLIC
        ${JSONCPP_LIB}
        antaresXpansion::helpers
        ${PROJECT_NAME}::xpansion_interfaces
)

add_library(${PROJECT_NAME}::output_core ALIAS output_core)

install(DIRECTORY include/antares-xpansion
        DESTINATION "include"
)