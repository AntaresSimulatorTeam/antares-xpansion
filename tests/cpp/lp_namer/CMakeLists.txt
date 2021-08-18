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
        ProblemModifierTest.cpp)

target_link_libraries (lp_namer_tests PRIVATE
        GTest::Main
        helpers
        lp_namer_core
        )

if (USE_MPI)
else ()
    set (THREADS_PREFER_PTHREAD_FLAG ON)
    target_link_libraries (lp_namer_tests PRIVATE
            Threads::Threads
            )
endif ()

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options (unit_tests PUBLIC -Wno-global-constructors)
    target_compile_options (unit_tests PUBLIC -Wno-exit-time-destructors)
    target_compile_options (unit_tests PUBLIC -Wno-weak-vtables)
    target_compile_options (unit_tests PUBLIC -Wno-covered-switch-default)
endif ()

add_test(NAME unit_cpp COMMAND lp_namer_tests)
set_property(TEST unit_cpp PROPERTY LABELS unit)

