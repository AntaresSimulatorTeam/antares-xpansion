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
	${CMAKE_CURRENT_SOURCE_DIR}/ortools_utils.cc
	)

target_include_directories (helpers
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries (helpers 
	PUBLIC
		ortools::ortools 
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
