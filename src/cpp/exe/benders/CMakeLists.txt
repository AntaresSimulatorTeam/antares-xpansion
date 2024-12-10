# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

# ---------------------------------------------------------------------------
# MPI Benders Exe
# ---------------------------------------------------------------------------

add_executable (benders
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

find_package(MPI REQUIRED)
if(UNIX)
set(CMAKE_CXX_COMPILER ${MPI_CXX_COMPILER})
# Check if ccache is installed and available in PATH
find_program(CCACHE_EXECUTABLE ccache)

if (CCACHE_EXECUTABLE)
	message(STATUS "Using ccache as the compiler launcher")
	set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_EXECUTABLE}")
	set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_EXECUTABLE}")
else ()
	message(WARNING "ccache not found. Proceeding without it.")
endif ()
endif()

#IF (WIN32)
#	target_link_libraries (benders
#			${PROJECT_NAME}::benders_sequential_core ${PROJECT_NAME}::benders_mpi_core
#			msmpi libboost_serialization-vc141-mt-x64-1_67)
#ELSE (WIN32)
	target_link_libraries (benders
			${PROJECT_NAME}::benders_mpi_core
			antaresXpansion::factories
			)
#ENDIF (WIN32)

install(TARGETS benders DESTINATION bin)
