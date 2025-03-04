# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

# ---------------------------------------------------------------------------
# Grid Search Exe
# ---------------------------------------------------------------------------

add_executable (grid_search_exe
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_include_directories (grid_search_exe PUBLIC
)


#IF (WIN32)
#	target_link_libraries (grid_search
#			${PROJECT_NAME}::grid_search_sequential_core ${PROJECT_NAME}::grid_search_mpi_core
#			msmpi libboost_serialization-vc141-mt-x64-1_67)
#ELSE (WIN32)
	target_link_libraries (grid_search_exe
			antaresXpansion::grid_search_lib
			antaresXpansion::factories
			antaresXpansion::output_core
			)
#ENDIF (WIN32)

install(TARGETS grid_search_exe DESTINATION bin)
