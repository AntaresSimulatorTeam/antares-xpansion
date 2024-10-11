# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

add_library(merge_mps_core)
target_sources(merge_mps_core PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/MergeMPS.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/merge_mps/MergeMPS.h
)


target_include_directories(merge_mps_core
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(merge_mps_core
        PUBLIC
        benders_core
        helpers
        antaresXpansion::logger_lib
        antaresXpansion::factories
        antaresXpansion::output_core
        solvers
)

add_library(${PROJECT_NAME}::merge_mps_core ALIAS merge_mps_core)
install(DIRECTORY include/antares-xpansion
        DESTINATION "include"
)