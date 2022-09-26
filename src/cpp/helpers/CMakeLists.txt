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
	${CMAKE_CURRENT_SOURCE_DIR}/StringUtils.cpp AntaresVersionProvider.cpp AntaresVersionProvider.h
	OptionsParser.h
	OptionsParser.cpp)

get_target_property(JSON_INC_PATH jsoncpp_lib INTERFACE_INCLUDE_DIRECTORIES)

target_include_directories (helpers
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/include
		${CMAKE_CURRENT_SOURCE_DIR}
                ${JSON_INC_PATH}/jsoncpp
)

target_link_libraries (helpers 
	PUBLIC
		jsoncpp_lib
		solvers
		glog::glog
		gflags::gflags
		Boost::boost Boost::program_options
)
