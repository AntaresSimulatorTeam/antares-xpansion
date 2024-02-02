# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

# ---------------------------------------------------------------------------
# MPI Benders Exe
# ---------------------------------------------------------------------------

add_executable (ext_loop
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

find_package(MPI REQUIRED)
if(UNIX)
set(CMAKE_CXX_COMPILER ${MPI_CXX_COMPILER})
endif()

#IF (WIN32)
#	target_link_libraries (benders
#			${PROJECT_NAME}::benders_sequential_core ${PROJECT_NAME}::benders_mpi_core
#			msmpi libboost_serialization-vc141-mt-x64-1_67)
#ELSE (WIN32)
	target_link_libraries (ext_loop
			${PROJECT_NAME}::benders_mpi_core
			factories
			)
#ENDIF (WIN32)

install(TARGETS ext_loop DESTINATION bin)
