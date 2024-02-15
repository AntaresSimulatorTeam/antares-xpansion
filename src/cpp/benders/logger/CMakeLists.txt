

add_library (logger_lib
        Master.cpp
        User.cpp
        UserFile.cpp
        CandidateLog.cpp
        IterationResultLog.cpp 
        MathLogger.cpp )

target_link_libraries (logger_lib
        PUBLIC
        benders_core
        )

target_include_directories (logger_lib
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        )
