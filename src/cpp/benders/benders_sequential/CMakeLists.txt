# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

add_library (benders_sequential_core STATIC
	${CMAKE_CURRENT_SOURCE_DIR}/BendersSequential.cpp
        )

get_target_property(JSON_INC_PATH jsoncpp_lib INTERFACE_INCLUDE_DIRECTORIES)


target_include_directories (benders_sequential_core
		PUBLIC
			${CMAKE_CURRENT_SOURCE_DIR}/include
		PUBLIC
			${CMAKE_CURRENT_SOURCE_DIR}
            ${JSON_INC_PATH}/jsoncpp
)

target_link_libraries (benders_sequential_core
	PUBLIC
        benders_core
		helpers
		output_core
		solvers
		glog::glog
		${PROJECT_NAME}::factories


)

add_library (${PROJECT_NAME}::benders_sequential_core ALIAS benders_sequential_core)