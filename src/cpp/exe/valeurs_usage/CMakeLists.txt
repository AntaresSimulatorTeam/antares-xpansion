# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

# ---------------------------------------------------------------------------
# Grid Search Exe
# ---------------------------------------------------------------------------

add_executable (valeurs_usage_exe
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_include_directories (valeurs_usage_exe PUBLIC
)


#IF (WIN32)
#	target_link_libraries (valeurs_usage
#			${PROJECT_NAME}::valeurs_usage_sequential_core ${PROJECT_NAME}::valeurs_usage_mpi_core
#			msmpi libboost_serialization-vc141-mt-x64-1_67)
#ELSE (WIN32)
	target_link_libraries (valeurs_usage_exe
			antaresXpansion::valeurs_usage_lib
			antaresXpansion::factories
			antaresXpansion::output_core
			)
#ENDIF (WIN32)

install(TARGETS valeurs_usage_exe DESTINATION bin)
