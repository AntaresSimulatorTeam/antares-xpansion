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

add_library (helpers)
target_sources(helpers PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/AntaresArchiveUpdater.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/AntaresArchiveUpdaterExeOptions.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/AntaresVersionProvider.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/ArchiveReader.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/ArchiveWriter.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/Clock.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/FileInBuffer.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/JsonXpansionReader.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/OptionsParser.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/helpers/AntaresArchiveUpdater.h
		${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/helpers/AntaresArchiveUpdaterExeOptions.h
		${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/helpers/AntaresVersionProvider.h
		${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/helpers/ArchiveIO.h
		${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/helpers/ArchiveReader.h
		${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/helpers/ArchiveWriter.h
		${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/helpers/Clock.h
		${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/helpers/FileInBuffer.h
		${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/helpers/LoggerUtils.h
		${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/helpers/OptionsParser.h
		${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/helpers/Timer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/helpers/solver_utils.h
        ${CMAKE_CURRENT_SOURCE_DIR}/solver_utils.cc
)
add_library(antaresXpansion::helpers ALIAS helpers)

get_target_property(xpansion_interfaces_path xpansion_interfaces INTERFACE_INCLUDE_DIRECTORIES)

target_include_directories (helpers
		PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		${xpansion_interfaces_path}
)

target_link_libraries (helpers
	PUBLIC
		${JSONCPP_LIB}
		solvers
		gflags::gflags
		Boost::boost Boost::program_options
		MINIZIP::minizip-ng
)

install(DIRECTORY include/antares-xpansion
		DESTINATION "include"
)