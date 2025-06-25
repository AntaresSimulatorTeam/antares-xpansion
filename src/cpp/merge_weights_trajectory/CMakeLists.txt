# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# External requirements
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

# --------------------------------------------------------------------------
# unit tests
# --------------------------------------------------------------------------

add_library(merge_weights_lib)
target_sources(merge_weights_lib PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/MergeWeightsTrajectory.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/merge_weights_trajectory/MergeWeightsTrajectory.h
)

target_include_directories(merge_weights_lib
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(merge_weights_lib
        PUBLIC
        antaresXpansion::node_lp_data_location
        antaresXpansion::logger_lib
        antaresXpansion::lp_namer_input_reader
)

install(DIRECTORY include/antares-xpansion
        DESTINATION "include"
)