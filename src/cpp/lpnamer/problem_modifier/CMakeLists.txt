# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

add_library(lp_namer_problem_modifier STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/StudyUpdater.cpp
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
        ${CMAKE_CURRENT_SOURCE_DIR}/LinkParametersCSVOverwriter.h
        ${CMAKE_CURRENT_SOURCE_DIR}/LinkParametersCSVOverwriter.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/LinkdataRecord.h
		${CMAKE_CURRENT_SOURCE_DIR}/LinkCapacitiesCSVWriter.h
		${CMAKE_CURRENT_SOURCE_DIR}/LinkCapacitiesCSVWriter.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/StudyUpdateStrategy.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/StudyUpdateStrategy.h
		${CMAKE_CURRENT_SOURCE_DIR}/StudyUpdateLinkCapacitiesStrategy.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/StudyUpdateLinkCapacitiesStrategy.h
		${CMAKE_CURRENT_SOURCE_DIR}/StudyUpdateLinkParameterStrategy.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/StudyUpdateLinkParameterStrategy.h)

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
)

add_library (${PROJECT_NAME}::lp_namer_problem_modifier ALIAS lp_namer_problem_modifier)
