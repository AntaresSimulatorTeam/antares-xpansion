# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

add_library (output_core STATIC
	${CMAKE_CURRENT_SOURCE_DIR}/VoidWriter.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/JsonWriter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Clock.cpp
        )

get_target_property(JSON_INC_PATH jsoncpp_lib INTERFACE_INCLUDE_DIRECTORIES)



target_include_directories (output_core
		PUBLIC
			${CMAKE_CURRENT_SOURCE_DIR}/include
		PUBLIC
			${CMAKE_CURRENT_SOURCE_DIR}
            ${JSON_INC_PATH}/jsoncpp
)

target_link_libraries (output_core
	PUBLIC
		jsoncpp_lib
)

add_library (${PROJECT_NAME}::output_core ALIAS output_core)