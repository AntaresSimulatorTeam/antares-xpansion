add_library (factories
        WriterFactories.cpp
        LoggerFactories.cpp
        BendersFactory.cpp
        )

if (USE_MPI)
	find_package(MPI REQUIRED)
	if (MPI_FOUND)
		message("CMAKE_CXX_COMPILER: ${MPI_CXX_COMPILER}")
		set (CMAKE_CXX_COMPILER ${MPI_CXX_COMPILER})
	endif (MPI_FOUND)
	target_link_libraries (factories
		PUBLIC
		${PROJECT_NAME}::benders_mpi_core)
endif ()
target_link_libraries (factories
        PUBLIC
        benders_core
        benders_sequential_core
	benders_by_batch_core
        output_core
        logger_lib
        )

target_include_directories (factories
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        )

add_library (${PROJECT_NAME}::factories ALIAS factories)