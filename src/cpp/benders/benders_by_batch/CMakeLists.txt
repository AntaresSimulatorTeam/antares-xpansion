# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

add_library (benders_by_batch_core STATIC
	${CMAKE_CURRENT_SOURCE_DIR}/BendersByBatch.cpp
        )

get_target_property(JSON_INC_PATH jsoncpp_lib INTERFACE_INCLUDE_DIRECTORIES)


target_include_directories (benders_by_batch_core
		PUBLIC
			${CMAKE_CURRENT_SOURCE_DIR}/include
		PUBLIC
			${CMAKE_CURRENT_SOURCE_DIR}
            ${JSON_INC_PATH}/jsoncpp
)

target_link_libraries (benders_by_batch_core
	PUBLIC
        benders_core
		helpers
		output_core
		solvers
		glog::glog
)

add_library (${PROJECT_NAME}::benders_by_batch_core ALIAS benders_by_batch_core)