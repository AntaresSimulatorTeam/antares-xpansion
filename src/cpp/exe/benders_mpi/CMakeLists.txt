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

find_package(MPI REQUIRED)

#IF (WIN32)
#	target_link_libraries (bendersmpi
#			${PROJECT_NAME}::benders_sequential_core ${PROJECT_NAME}::benders_mpi_core
#			msmpi libboost_serialization-vc141-mt-x64-1_67)
#ELSE (WIN32)
	target_link_libraries (bendersmpi
			${PROJECT_NAME}::benders_mpi_core
			factories
			)
#ENDIF (WIN32)

install(TARGETS bendersmpi DESTINATION bin)
