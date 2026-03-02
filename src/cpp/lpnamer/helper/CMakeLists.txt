# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

add_library(lp_namer_helper)
target_sources(lp_namer_helper PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/ColumnToChange.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ProblemGenerationLogger.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/lpnamer/helper/ColumnToChange.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/lpnamer/helper/ProblemGenerationLogger.h
)

target_include_directories(lp_namer_helper
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_link_libraries(lp_namer_helper
        PUBLIC
        antaresXpansion::helpers
)

add_library(${PROJECT_NAME}::lp_namer_helper ALIAS lp_namer_helper)
install(DIRECTORY include/antares-xpansion
        DESTINATION "include"
)