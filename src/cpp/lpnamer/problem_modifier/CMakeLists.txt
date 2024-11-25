# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

add_library(lp_namer_problem_modifier)
target_sources(lp_namer_problem_modifier PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/AdditionalConstraints.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/AntaresProblemToXpansionProblemTranslator.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/FileProblemProviderAdapter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/FileProblemsProviderAdapter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/LauncherHelpers.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/LinkProblemsGenerator.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/LinkdataRecord.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/MPSFileProblemProviderAdapter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/MPSFileWriter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/MasterGeneration.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/MasterProblemBuilder.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ProblemModifier.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ProblemVariablesFileAdapter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ProblemVariablesFromProblemAdapter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ProblemVariablesZipAdapter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/XpansionProblemsFromAntaresProvider.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ZipProblemProviderAdapter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ZipProblemsProviderAdapter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/lpnamer/problem_modifier/AdditionalConstraints.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/lpnamer/problem_modifier/AntaresProblemToXpansionProblemTranslator.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/lpnamer/problem_modifier/FileProblemProviderAdapter.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/lpnamer/problem_modifier/FileProblemsProviderAdapter.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/lpnamer/problem_modifier/IProblemProviderPort.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/lpnamer/problem_modifier/IProblemVariablesProviderPort.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/lpnamer/problem_modifier/IProblemWriter.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/lpnamer/problem_modifier/IXpansionProblemsProvider.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/lpnamer/problem_modifier/LauncherHelpers.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/lpnamer/problem_modifier/LinkProblemsGenerator.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/lpnamer/problem_modifier/LinkdataRecord.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/lpnamer/problem_modifier/MPSFileProblemProviderAdapter.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/lpnamer/problem_modifier/MPSFileWriter.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/lpnamer/problem_modifier/MasterGeneration.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/lpnamer/problem_modifier/MasterProblemBuilder.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/lpnamer/problem_modifier/ProblemModifier.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/lpnamer/problem_modifier/ProblemVariablesFileAdapter.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/lpnamer/problem_modifier/ProblemVariablesFromProblemAdapter.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/lpnamer/problem_modifier/ProblemVariablesZipAdapter.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/lpnamer/problem_modifier/XpansionProblemsFromAntaresProvider.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/lpnamer/problem_modifier/ZipProblemProviderAdapter.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/antares-xpansion/lpnamer/problem_modifier/ZipProblemsProviderAdapter.h
)

target_include_directories(lp_namer_problem_modifier
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(lp_namer_problem_modifier
        PUBLIC
        antaresXpansion::lp_namer_model
        antaresXpansion::lp_namer_input_reader
        antaresXpansion::lp_namer_helper
        antaresXpansion::helpers
        antaresXpansion::solvers
        Antares::antares-solver-simulation
)

if (TBB_FOUND)
    target_link_libraries(lp_namer_problem_modifier PRIVATE tbb)
endif ()

add_library(${PROJECT_NAME}::lp_namer_problem_modifier ALIAS lp_namer_problem_modifier)

install(DIRECTORY include/antares-xpansion
        DESTINATION "include"
)