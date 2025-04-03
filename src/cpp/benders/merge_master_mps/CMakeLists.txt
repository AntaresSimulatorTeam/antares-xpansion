# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

add_library(merge_master_mps_core)
target_sources(merge_master_mps_core PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/merge_master_mps/MergeMasterMPS.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/merge_master_mps/StandardLp.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/merge_master_mps/MasterCoupling.h

        ${CMAKE_CURRENT_SOURCE_DIR}/MergeMasterMPS.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/MasterCoupling.cpp
)


target_include_directories(merge_master_mps_core
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(merge_master_mps_core
        PUBLIC
        antaresXpansion::benders_core
        antaresXpansion::helpers
        antaresXpansion::logger_lib
        antaresXpansion::factories
        antaresXpansion::output_core
        antaresXpansion::solvers
)

add_library(${PROJECT_NAME}::merge_master_mps_core ALIAS merge_master_mps_core)
install(DIRECTORY include/antares-xpansion
        DESTINATION "include"
)