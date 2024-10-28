add_library(sensitivity_core)
target_sources(sensitivity_core PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/sensitivity/ProblemModifierCapex.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/sensitivity/ProblemModifierProjection.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/sensitivity/SensitivityFileLogger.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/sensitivity/SensitivityInputReader.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/sensitivity/SensitivityLogger.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/sensitivity/SensitivityMasterLogger.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/sensitivity/SensitivityProblemModifier.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/sensitivity/SensitivityStudy.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/sensitivity/SensitivityWriter.h
        ${CMAKE_CURRENT_SOURCE_DIR}/private/Analysis.h
        Analysis.cpp
        ProblemModifierCapex.cpp
        ProblemModifierProjection.cpp
        SensitivityFileLogger.cpp
        SensitivityInputReader.cpp
        SensitivityLogger.cpp
        SensitivityMasterLogger.cpp
        SensitivityProblemModifier.cpp
        SensitivityStudy.cpp
        SensitivityWriter.cpp
)


target_include_directories(sensitivity_core
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/private>
)

target_link_libraries(sensitivity_core
        PUBLIC
        antaresXpansion::logger_lib
)

add_library(${PROJECT_NAME}::sensitivity_core ALIAS sensitivity_core)