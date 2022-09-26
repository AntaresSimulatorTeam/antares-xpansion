find_package(GTest REQUIRED)

add_executable(StudyTopology_test
        StudyTopologyTest.cpp
        CoreHexagone/Model/Link.cpp
        CoreHexagone/Model/Link.h
        CoreHexagone/ForProvidingXpansionStudy.cpp
        CoreHexagone/ForProvidingXpansionStudy.h
        ACL/StudyAdapter.cpp
        ACL/StudyAdapter.h
        ACL/Stub/StudyInMemoryAdapter.cpp
        ACL/Stub/StudyInMemoryAdapter.h
        CoreHexagone/Model/Candidate.cpp
        CoreHexagone/Model/Candidate.h
        CoreHexagone/Model/XpansionStudy.cpp
        CoreHexagone/Model/XpansionStudy.h
        CoreHexagone/CandidateID.cpp
        CoreHexagone/CandidateID.h
        ACLTest.cpp
        XpansionStudyFileReader/Stub/LinkFileReaderInMemory.cpp
        XpansionStudyFileReader/Stub/LinkFileReaderInMemory.h
        ACL/XpansionStudyFileAdapter.cpp
        ACL/XpansionStudyFileAdapter.h
        XpansionStudyFileReader//Model/Link.cpp
        XpansionStudyFileReader//Model/Link.h
        XpansionStudyFileReader//Stub/AreaFileReaderInMemory.cpp
        XpansionStudyFileReader//Stub/AreaFileReaderInMemory.h
        CoreHexagone/Model/Area.cpp
        CoreHexagone/Model/Area.h
        XpansionStudyFileReader/Stub/CandidateFileReaderInMemory.cpp
        XpansionStudyFileReader/Stub/CandidateFileReaderInMemory.h
        CoreHexagone/Model/Study.cpp
        CoreHexagone/Model/Study.h
        XpansionStudyFileReader/Model/Candidate.cpp
        XpansionStudyFileReader/Model/Candidate.h
        ACL/LinkFromFileTranslator.cpp
        ACL/LinkFromFileTranslator.h)

target_link_libraries(StudyTopology_test PRIVATE
        GTest::Main
        )

if (USE_MPI)
else ()
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    target_link_libraries(StudyTopology_test PRIVATE
            Threads::Threads
            )
endif ()

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(unit_tests PUBLIC -Wno-global-constructors)
    target_compile_options(unit_tests PUBLIC -Wno-exit-time-destructors)
    target_compile_options(unit_tests PUBLIC -Wno-weak-vtables)
    target_compile_options(unit_tests PUBLIC -Wno-covered-switch-default)
endif ()

add_test(NAME unit_StudyTopology COMMAND StudyTopology_test)
set_property(TEST unit_StudyTopology PROPERTY LABELS unit)
