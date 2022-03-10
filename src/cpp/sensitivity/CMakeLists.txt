add_library (sensitivity_core
	SensitivityInputReader.cpp
	SensitivityMasterLogger.cpp
	SensitivityUserLogger.cpp
	SensitivityFileLogger.cpp
        SensitivityStudy.cpp
	SensitivityWriter.cpp 
	SensitivityPbModifier.cpp 
	PbModifierCapex.cpp 
	PbModifierProjection.cpp
        CapexAnalysis.cpp CapexAnalysis.h)

get_target_property(JSON_INC_PATH jsoncpp_lib
INTERFACE_INCLUDE_DIRECTORIES)

target_include_directories (sensitivity_core
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/include
		${JSON_INC_PATH}/jsoncpp
)

target_link_libraries (sensitivity_core
	PUBLIC
		logger_lib
)

add_library (${PROJECT_NAME}::sensitivity_core ALIAS sensitivity_core)