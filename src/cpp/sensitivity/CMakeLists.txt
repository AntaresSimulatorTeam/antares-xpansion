find_package(TBB REQUIRED CONFIG)
if (TBB_VERSION_MAJOR VERSION_LESS "2018")
	message(FATAL_ERROR "Require tbb 2018 or higher.")
endif()
if (TBB_VERSION_MAJOR VERSION_GREATER "2020")
	message(FATAL_ERROR "Require tbb 2018 to 2020.")
endif()

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