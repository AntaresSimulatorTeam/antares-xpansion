add_library (factories
        WriterFactories.cpp
        LoggerFactories.cpp
        BendersFactory.cpp
        )

target_link_libraries (factories
        PUBLIC
        benders_core
        benders_sequential_core
	benders_by_batch_core
        output_core
        logger_lib
        ${PROJECT_NAME}::benders_mpi_core
        external_loop
        )

target_include_directories (factories
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        )

add_library (${PROJECT_NAME}::factories ALIAS factories)