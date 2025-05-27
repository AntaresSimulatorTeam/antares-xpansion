# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

# ---------------------------------------------------------------------------
# Problem Generation Exe
# ---------------------------------------------------------------------------

add_executable (merge_weights_trajectory
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_link_libraries (merge_weights_trajectory
		PRIVATE merge_weights_lib
)

install(TARGETS merge_weights_trajectory DESTINATION bin)
