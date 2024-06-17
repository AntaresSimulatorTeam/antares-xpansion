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
		Analysis.h
		Analysis.cpp) 


target_include_directories (sensitivity_core
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries (sensitivity_core
	PUBLIC
		logger_lib
		TBB::tbb
)

add_library (${PROJECT_NAME}::sensitivity_core ALIAS sensitivity_core)
install(FILES
		$<TARGET_FILE:TBB::tbb>
		DESTINATION bin)