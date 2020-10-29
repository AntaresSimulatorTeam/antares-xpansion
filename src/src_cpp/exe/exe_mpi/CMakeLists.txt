# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

# ---------------------------------------------------------------------------
# MPI Benders Exe
# ---------------------------------------------------------------------------

add_executable (bendersmpi
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

#IF (WIN32)
#	target_link_libraries (bendersmpi
#			${PROJECT_NAME}::sequentialcore ${PROJECT_NAME}::mpicore
#			msmpi libboost_serialization-vc141-mt-x64-1_67)
#ELSE (WIN32)
	target_link_libraries (bendersmpi
			${PROJECT_NAME}::sequentialcore ${PROJECT_NAME}::mpicore
			Boost::serialization)
#ENDIF (WIN32)
