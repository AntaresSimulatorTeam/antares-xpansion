# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

add_library(benders_sequential_core)

target_sources(benders_sequential_core PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/BendersSequential.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/benders_sequential/BendersSequential.h
)

target_include_directories(benders_sequential_core
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(benders_sequential_core
        PUBLIC
        antaresXpansion::benders_core
        helpers
        antaresXpansion::output_core
        solvers
        ${PROJECT_NAME}::factories


)

add_library(${PROJECT_NAME}::benders_sequential_core ALIAS benders_sequential_core)
install(DIRECTORY include/antares-xpansion
        DESTINATION "include"
)