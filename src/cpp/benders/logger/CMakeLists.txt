add_library (logger_lib)

target_sources(logger_lib PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/logger/CandidateLog.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/logger/Commons.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/logger/IterationResultLog.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/logger/Master.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/logger/User.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/logger/User.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/benders/logger/UserFile.h
        CandidateLog.cpp
        IterationResultLog.cpp
        Master.cpp
        MathLogger.cpp
        User.cpp
        UserFile.cpp
)

add_library(antaresXpansion::logger_lib ALIAS logger_lib)

target_link_libraries (logger_lib
        PUBLIC
        antaresXpansion::benders_core
        )

target_include_directories (logger_lib
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        )

install(DIRECTORY include/antares-xpansion
        DESTINATION "include"
)