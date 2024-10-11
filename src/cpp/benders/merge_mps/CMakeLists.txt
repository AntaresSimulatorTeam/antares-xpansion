# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

add_library (merge_mps_core STATIC
	${CMAKE_CURRENT_SOURCE_DIR}/MergeMPS.cpp
        )



target_include_directories (merge_mps_core
		PUBLIC
			${CMAKE_CURRENT_SOURCE_DIR}/include

)

target_link_libraries (merge_mps_core
	PUBLIC
        benders_core
		helpers
        logger_lib
        factories
		antaresXpansion::output_core
		solvers
)

add_library (${PROJECT_NAME}::merge_mps_core ALIAS merge_mps_core)