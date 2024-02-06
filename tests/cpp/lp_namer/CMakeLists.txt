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
        AdditionalConstraintsTest.cc
        AdditionalConstraintsReaderTest.cc
        LinkdataRecordTest.cc
        StudyUpdateTest.cc
        CandidatesINIReaderTest.cpp
        ActiveLinkTest.cpp
        LinkProfileReaderTest.cpp
        ProblemModifierTest.cpp
        VariableFileReaderTest.cpp
        MasterProblemBuilderTest.cpp
        ChronicleMapReaderTest.cpp
        ChronicleMapProviderTest.cpp
        ProblemConstructionTest.cpp 
        NOOPSolver.h
        ProblemGenerationExeOptionsTest.cpp
        LoggerBuilder.h
        LoggerBuilder.cpp
        ProblemGenerationLoggerTest.cpp
        WeightsFileReaderTest.cpp
        LpFilesExtractorTest.cpp
        MpsTxtWriterTest.cpp
)

target_link_libraries (lp_namer_tests PRIVATE
        GTest::Main
        helpers 
        lp_namer_helper
        lp_namer_model
        lp_namer_input_reader
        lp_namer_problem_modifier
        xpansion_study_updater_lib
        problem_generation_main
        )


if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options (unit_tests PUBLIC -Wno-global-constructors)
    target_compile_options (unit_tests PUBLIC -Wno-exit-time-destructors)
    target_compile_options (unit_tests PUBLIC -Wno-weak-vtables)
    target_compile_options (unit_tests PUBLIC -Wno-covered-switch-default)
endif ()

add_test(NAME unit_lpnamer COMMAND lp_namer_tests WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
set_property(TEST unit_lpnamer PROPERTY LABELS unit)

