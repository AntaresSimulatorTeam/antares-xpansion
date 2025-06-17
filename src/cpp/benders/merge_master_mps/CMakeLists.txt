# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

#node_lp_data_location
add_library(node_lp_data_location)

target_sources(node_lp_data_location PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/merge_master_mps/NodeLpDataLocation.h
        ${CMAKE_CURRENT_SOURCE_DIR}/NodeLpDataLocation.cpp
)

target_include_directories(node_lp_data_location
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(node_lp_data_location
        PUBLIC
        antaresXpansion::benders_core
)

add_library(${PROJECT_NAME}::node_lp_data_location ALIAS node_lp_data_location)

# merge_master_mps_core
add_library(merge_master_mps_core)
target_sources(merge_master_mps_core PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/merge_master_mps/MergeMasterMPS.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/merge_master_mps/MasterStructureKeys.h
        ${CMAKE_CURRENT_SOURCE_DIR}/MergeMasterMPS.cpp
)


target_include_directories(merge_master_mps_core
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(merge_master_mps_core
        PUBLIC
        antaresXpansion::node_lp_data_location
        antaresXpansion::merge_mps_core
)

add_library(${PROJECT_NAME}::merge_master_mps_core ALIAS merge_master_mps_core)

install(DIRECTORY include/antares-xpansion
        DESTINATION "include"
)
