# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# External requirements
# ===========================================================================
find_package (GTest REQUIRED)

# ===========================================================================
# Targets
# ===========================================================================

# --------------------------------------------------------------------------
# unit tests
# --------------------------------------------------------------------------
add_executable (lp_namer_tests
        ActiveLinkTest.cpp
        AdditionalConstraintsReaderTest.cc
        AdditionalConstraintsTest.cc
        AntaresProblemToXpansionProblemTranslatorTest.cpp
        CandidatesINIReaderTest.cpp
        ChronicleMapProviderTest.cpp
        ChronicleMapReaderTest.cpp
        GeneralDataReadetTests.cpp
        LinkProfileReaderTest.cpp
        LinkdataRecordTest.cc
        LoggerBuilder.cpp
        LoggerBuilder.h
        LpFilesExtractorTest.cpp
        MasterProblemBuilderTest.cpp
        MpsTxtWriterTest.cpp
        NOOPSolver.h
        ProblemConstructionTest.cpp
        ProblemGenerationExeOptionsTest.cpp
        ProblemGenerationLoggerTest.cpp
        ProblemModifierTest.cpp
        ProblemVariablesFromProblemAdapterTest.cpp
        SettingsReaderTest.cpp
        StudyUpdateTest.cc
        VariableFileReaderTest.cpp
        WeightsFileReaderTest.cpp
        WeightsFileWriterTest.cpp
)

target_link_libraries (lp_namer_tests PRIVATE
        GTest::Main
        antaresXpansion::helpers
        antaresXpansion::lp_namer_helper
        antaresXpansion::lp_namer_model
        antaresXpansion::lp_namer_input_reader
        antaresXpansion::lp_namer_problem_modifier
        antaresXpansion::xpansion_study_updater_lib
        antaresXpansion::problem_generation_main
        antaresXpansion::tests_utils
        )


if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options (unit_tests PUBLIC -Wno-global-constructors)
    target_compile_options (unit_tests PUBLIC -Wno-exit-time-destructors)
    target_compile_options (unit_tests PUBLIC -Wno-weak-vtables)
    target_compile_options (unit_tests PUBLIC -Wno-covered-switch-default)
endif ()

add_test(NAME unit_lpnamer COMMAND lp_namer_tests WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
set_property(TEST unit_lpnamer PROPERTY LABELS unit)

