# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

add_library (xpansion_interfaces INTERFACE 
        )
target_include_directories(xpansion_interfaces INTERFACE 
	${CMAKE_CURRENT_SOURCE_DIR} 
 )
add_library (${PROJECT_NAME}::xpansion_interfaces ALIAS xpansion_interfaces)		
