add_library (sensitivity_core
	SensitivityInputReader.cpp
	SensitivityMasterLogger.cpp
        SensitivityLogger.cpp
	SensitivityFileLogger.cpp
        SensitivityStudy.cpp
	SensitivityWriter.cpp
        SensitivityProblemModifier.cpp
		ProblemModifierCapex.cpp
		ProblemModifierProjection.cpp
		AnalysisFunctions.h 
		AnalysisFunctions.cpp
		Analysis.h
		Analysis.cpp) 

get_target_property(JSON_INC_PATH jsoncpp_lib
INTERFACE_INCLUDE_DIRECTORIES)

add_dependencies(sensitivity_core tbb)
target_include_directories (sensitivity_core
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/include
		${JSON_INC_PATH}/jsoncpp
)

target_link_libraries (sensitivity_core
	PUBLIC
		logger_lib
		tbb
)

add_library (${PROJECT_NAME}::sensitivity_core ALIAS sensitivity_core)