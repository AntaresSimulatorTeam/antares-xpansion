

add_library (sensitivity
	SensitivityAnalysis.cpp 
	SensitivityLogger.cpp 
	SensitivityWriter.cpp 
	SensitivityPbModifier.cpp 
	PbModifierCapex.cpp 
	PbModifierProjection.cpp
)

get_target_property(JSON_INC_PATH jsoncpp_lib INTERFACE_INCLUDE_DIRECTORIES)

target_include_directories (sensitivity
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/include
		${JSON_INC_PATH}/jsoncpp
)

target_link_libraries (sensitivity
	PUBLIC
		benders_core
)
