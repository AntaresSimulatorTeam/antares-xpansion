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

add_library (helpers STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/solver_utils.h
        ${CMAKE_CURRENT_SOURCE_DIR}/solver_utils.cc
		${CMAKE_CURRENT_SOURCE_DIR}/JsonXpansionReader.cpp
		 AntaresVersionProvider.cpp AntaresVersionProvider.h
		${CMAKE_CURRENT_SOURCE_DIR}/OptionsParser.h
		${CMAKE_CURRENT_SOURCE_DIR}/OptionsParser.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/ArchiveIO.h
		${CMAKE_CURRENT_SOURCE_DIR}/ArchiveReader.h
		${CMAKE_CURRENT_SOURCE_DIR}/ArchiveReader.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/ArchiveWriter.h
		${CMAKE_CURRENT_SOURCE_DIR}/ArchiveWriter.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/FileInBuffer.h
		${CMAKE_CURRENT_SOURCE_DIR}/FileInBuffer.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/Clock.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/Clock.h
		${CMAKE_CURRENT_SOURCE_DIR}/Timer.h
		${CMAKE_CURRENT_SOURCE_DIR}/AntaresArchiveUpdater.h
		${CMAKE_CURRENT_SOURCE_DIR}/AntaresArchiveUpdater.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/AntaresArchiveUpdaterExeOptions.h
		${CMAKE_CURRENT_SOURCE_DIR}/AntaresArchiveUpdaterExeOptions.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/LoggerUtils.h
		)

get_target_property(xpansion_interfaces_path xpansion_interfaces INTERFACE_INCLUDE_DIRECTORIES)

target_include_directories (helpers
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
		${xpansion_interfaces_path}
)

target_link_libraries (helpers 
	PUBLIC
        jsoncpp_static
		solvers
		glog::glog
		gflags::gflags
		Boost::boost Boost::program_options
		MINIZIP::minizip-ng
)
