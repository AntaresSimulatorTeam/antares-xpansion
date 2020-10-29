# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

add_executable (test_sur_les_set
	${CMAKE_CURRENT_SOURCE_DIR}/test_sur_les_set.cpp
)

target_link_libraries (test_sur_les_set ${PROJECT_NAME}::sequentialcore)
