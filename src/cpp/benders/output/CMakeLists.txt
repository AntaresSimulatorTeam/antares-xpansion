# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

add_library (output_core STATIC
	${CMAKE_CURRENT_SOURCE_DIR}/VoidWriter.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/JsonWriter.cpp
        )

get_target_property(xpansion_interfaces_path xpansion_interfaces INTERFACE_INCLUDE_DIRECTORIES)

target_include_directories (output_core
		PUBLIC
			${CMAKE_CURRENT_SOURCE_DIR}/include
		PUBLIC
			${CMAKE_CURRENT_SOURCE_DIR}
			${xpansion_interfaces_path}
)

target_link_libraries (output_core
	PUBLIC
		jsoncpp_static
		helpers
)

add_library (${PROJECT_NAME}::output_core ALIAS output_core)