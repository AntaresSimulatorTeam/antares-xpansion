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
	${CMAKE_CURRENT_SOURCE_DIR}/ortools_utils.h
	${CMAKE_CURRENT_SOURCE_DIR}/ortools_utils.cc
	${CMAKE_CURRENT_SOURCE_DIR}/JsonXpansionReader.cpp
	)

get_target_property(JSON_INC_PATH jsoncpp_lib INTERFACE_INCLUDE_DIRECTORIES)

target_include_directories (helpers
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
		${ORTOOLS_INCLUDE_DIRS}
                ${JSON_INC_PATH}/jsoncpp
)

target_link_libraries (helpers 
	PUBLIC
		ortools::ortools 
		jsoncpp_lib
)

#Dirty hack to add sirius solver dependency if available (needed because ortools doesn't indicated sirius dependency)
find_package(sirius_solver)

if(sirius_solver_FOUND)
	target_link_libraries(helpers
			PUBLIC
			sirius_solver
			)
            
	copy_dependency(sirius_solver helpers)
endif()
