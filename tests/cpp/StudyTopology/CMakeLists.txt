find_package (GTest REQUIRED)

add_executable (StudyTopology_test
        StudyTopologyTest.cpp CoreHexagone/Link.cpp CoreHexagone/Link.h CoreHexagone/ForProvidingXpansionStudy.cpp CoreHexagone/ForProvidingXpansionStudy.h ACL/StudyAdapter.cpp ACL/StudyAdapter.h ACL/Stub/StudyInMemoryAdapter.cpp ACL/Stub/StudyInMemoryAdapter.h CoreHexagone/Candidate.cpp CoreHexagone/Candidate.h CoreHexagone/XpansionStudy.cpp CoreHexagone/XpansionStudy.h CoreHexagone/CandidateID.cpp CoreHexagone/CandidateID.h ACLTest.cpp StudyFileAdapter/Stub/LinkFileReaderInMemory.cpp StudyFileAdapter/Stub/LinkFileReaderInMemory.h ACL/Adapter.cpp ACL/Adapter.h StudyFileAdapter/Link.cpp StudyFileAdapter/Link.h AreaFileReaderInMemory.cpp AreaFileReaderInMemory.h CoreHexagone/Area.cpp CoreHexagone/Area.h)

target_link_libraries (StudyTopology_test PRIVATE
        GTest::Main
        )

if (USE_MPI)
else ()
    set (THREADS_PREFER_PTHREAD_FLAG ON)
    target_link_libraries (StudyTopology_test PRIVATE
            Threads::Threads
            )
endif ()

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options (unit_tests PUBLIC -Wno-global-constructors)
    target_compile_options (unit_tests PUBLIC -Wno-exit-time-destructors)
    target_compile_options (unit_tests PUBLIC -Wno-weak-vtables)
    target_compile_options (unit_tests PUBLIC -Wno-covered-switch-default)
endif ()

add_test(NAME unit_StudyTopology COMMAND StudyTopology_test)
set_property(TEST unit_StudyTopology PROPERTY LABELS unit)
