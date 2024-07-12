# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

# ---------------------------------------------------------------------------
# MPI Benders Exe
# ---------------------------------------------------------------------------

add_executable(adequacy_criterion
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

find_package(MPI REQUIRED)
if(UNIX)
set(CMAKE_CXX_COMPILER ${MPI_CXX_COMPILER})
endif()

target_link_libraries(adequacy_criterion
			${PROJECT_NAME}::adequacy_criterion_lib
			factories
			)

install(TARGETS adequacy_criterion DESTINATION bin)
