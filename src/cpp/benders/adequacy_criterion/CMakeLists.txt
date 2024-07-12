# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

find_package (MPI REQUIRED COMPONENTS C)
message ("MPI_C_INCLUDE_PATH ${MPI_C_INCLUDE_PATH}")
message ("MPI_C_INCLUDE_DIRS ${MPI_C_INCLUDE_DIRS}")
message ("MPI_C_LIBRARIES ${MPI_C_LIBRARIES}")

find_package(MPI REQUIRED)

if(UNIX)
set(CMAKE_CXX_COMPILER ${MPI_CXX_COMPILER})
endif()

add_library (adequacy_criterion_lib STATIC
	${CMAKE_CURRENT_SOURCE_DIR}/AdequacyCriterion.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/MasterUpdateBase.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/CutsManagement.cpp
	)




target_include_directories (adequacy_criterion_lib
		PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/include
	    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
		)

target_link_libraries (adequacy_criterion_lib
	PUBLIC
		Boost::mpi
		MPI::MPI_C
		Boost::serialization
		helpers
		benders_core
		benders_mpi_core
		benders_by_batch_core		
)

add_library (${PROJECT_NAME}::adequacy_criterion_lib ALIAS adequacy_criterion_lib)
