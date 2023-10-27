# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

find_package(TBB REQUIRED CONFIG)
if (TBB_VERSION_MAJOR VERSION_LESS "2018")
	message(FATAL_ERROR "Require tbb 2018 or higher.")
endif()
if (TBB_VERSION_MAJOR VERSION_GREATER "2020")
	message(FATAL_ERROR "Require tbb 2018 to 2020.")
endif()

add_library(lp_namer_problem_modifier STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/LinkdataRecord.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/LinkProblemsGenerator.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/LinkProblemsGenerator.h
		${CMAKE_CURRENT_SOURCE_DIR}/AdditionalConstraints.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/AdditionalConstraints.h
		${CMAKE_CURRENT_SOURCE_DIR}/ProblemModifier.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/ProblemModifier.h
		${CMAKE_CURRENT_SOURCE_DIR}/MasterProblemBuilder.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/MasterProblemBuilder.h
		${CMAKE_CURRENT_SOURCE_DIR}/LauncherHelpers.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/LauncherHelpers.h
		${CMAKE_CURRENT_SOURCE_DIR}/MasterGeneration.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/MasterGeneration.h
		${CMAKE_CURRENT_SOURCE_DIR}/LinkdataRecord.h
		${CMAKE_CURRENT_SOURCE_DIR}/ZipProblemProviderAdapter.h
		${CMAKE_CURRENT_SOURCE_DIR}/ZipProblemProviderAdapter.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/IProblemProviderPort.h
		${CMAKE_CURRENT_SOURCE_DIR}/IProblemWriter.h
		${CMAKE_CURRENT_SOURCE_DIR}/MPSFileWriter.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/MPSFileWriter.h
		${CMAKE_CURRENT_SOURCE_DIR}/ProblemVariablesZipAdapter.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/ProblemVariablesZipAdapter.h
		${CMAKE_CURRENT_SOURCE_DIR}/IProblemVariablesProviderPort.h
		${CMAKE_CURRENT_SOURCE_DIR}/ProblemVariablesFileAdapter.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/ProblemVariablesFileAdapter.h
		${CMAKE_CURRENT_SOURCE_DIR}/MPSFileProblemProviderAdapter.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/MPSFileProblemProviderAdapter.h
		${CMAKE_CURRENT_SOURCE_DIR}/LpsFromAntares.h
		${CMAKE_CURRENT_SOURCE_DIR}/XpansionProblemsFromAntaresProvider.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/XpansionProblemsFromAntaresProvider.h
		${CMAKE_CURRENT_SOURCE_DIR}/AntaresProblemToXpansionProblemTranslator.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/AntaresProblemToXpansionProblemTranslator.h
		${CMAKE_CURRENT_SOURCE_DIR}/IXpansionProblemsProvider.h
		${CMAKE_CURRENT_SOURCE_DIR}/ZipProblemsProviderAdapter.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/ZipProblemsProviderAdapter.h
		${CMAKE_CURRENT_SOURCE_DIR}/ProblemVariablesFromProblemAdapter.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/ProblemVariablesFromProblemAdapter.h
)

target_include_directories (lp_namer_problem_modifier
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries (lp_namer_problem_modifier
	PUBLIC
	    lp_namer_model
		lp_namer_input_reader
		lp_namer_helper
		helpers
		solvers
		TBB::tbb
)

add_library (${PROJECT_NAME}::lp_namer_problem_modifier ALIAS lp_namer_problem_modifier)
