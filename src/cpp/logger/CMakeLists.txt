

add_library (logger
        MasterLog.cpp
        )

target_link_libraries (logger
        PUBLIC
        benders_sequential_core
        )

target_include_directories (logger
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        )