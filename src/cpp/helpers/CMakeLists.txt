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
        ${CMAKE_CURRENT_SOURCE_DIR}/AntaresArchiveUpdater.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/AntaresArchiveUpdaterExeOptions.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/AntaresVersionProvider.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ArchiveReader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ArchiveWriter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/FileInBuffer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/OptionsParser.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/AreaParser.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/helpers/AntaresArchiveUpdater.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/helpers/AntaresArchiveUpdaterExeOptions.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/helpers/AntaresVersionProvider.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/helpers/ArchiveIO.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/helpers/ArchiveReader.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/helpers/ArchiveWriter.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/helpers/FileInBuffer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/helpers/OptionsParser.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/helpers/Timer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/helpers/solver_utils.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/helpers/AreaParser.h
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
        MINIZIP::minizip-ng
        ${PROJECT_NAME}::xpansion_interfaces
)

install(DIRECTORY include/antares-xpansion
        DESTINATION "include"
)