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

add_library(helpers)
target_sources(helpers PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/AntaresVersionProvider.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Clock.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/OptionsParser.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/helpers/AntaresVersionProvider.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/helpers/Clock.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/helpers/LoggerUtils.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/helpers/OptionsParser.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/helpers/Timer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/helpers/solver_utils.h
        ${CMAKE_CURRENT_SOURCE_DIR}/solver_utils.cc
)
add_library(antaresXpansion::helpers ALIAS helpers)

target_include_directories(helpers
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(helpers
        PUBLIC
        ${JSONCPP_LIB}
        antaresXpansion::solvers
        gflags::gflags
        Boost::boost Boost::program_options
        ${PROJECT_NAME}::xpansion_interfaces
)

install(DIRECTORY include/antares-xpansion
        DESTINATION "include"
)