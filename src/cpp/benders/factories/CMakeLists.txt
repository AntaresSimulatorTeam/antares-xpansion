add_library (factories
        WriterFactories.cpp
        LoggerFactories.cpp
        )

target_link_libraries (factories
        PUBLIC
        benders_core
        logger_lib
        )

target_include_directories (factories
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        )

add_library (${PROJECT_NAME}::factories ALIAS factories)