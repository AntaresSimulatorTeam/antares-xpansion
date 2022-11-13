# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

# ---------------------------------------------------------------------------
# Benders by batch Exe
# ---------------------------------------------------------------------------

add_executable (bendersbybatch
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_link_libraries (bendersbybatch
		${PROJECT_NAME}::benders_core
		${PROJECT_NAME}::benders_by_batch_core
		${PROJECT_NAME}::factories
		${PROJECT_NAME}::output_core
		logger_lib)

install(TARGETS bendersbybatch DESTINATION bin)