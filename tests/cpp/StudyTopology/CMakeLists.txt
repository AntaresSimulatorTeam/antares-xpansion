find_package(GTest REQUIRED)

add_executable(StudyTopology_test
        UseCasesTests.cpp
        CoreHexagone/Model/Xpansion/Link.cpp
        CoreHexagone/Model/Xpansion/Link.h
        ACL/Stub/StudyInMemoryAdapter.cpp
        ACL/Stub/StudyInMemoryAdapter.h
        CoreHexagone/Model/Xpansion/Candidate.cpp
        CoreHexagone/Model/Xpansion/Candidate.h
        ACLTest.cpp
        XpansionStudyFileReader/Stub/LinkFileReaderInMemory.cpp
        XpansionStudyFileReader/Stub/LinkFileReaderInMemory.h
        ACL/XpansionStudyFileAdapter.cpp
        ACL/XpansionStudyFileAdapter.h
        XpansionStudyFileReader//Model/Link.cpp
        XpansionStudyFileReader//Model/Link.h
        XpansionStudyFileReader//Stub/AreaFileReaderInMemory.cpp
        XpansionStudyFileReader//Stub/AreaFileReaderInMemory.h
        CoreHexagone/Model/Xpansion/Area.cpp
        CoreHexagone/Model/Xpansion/Area.h
        XpansionStudyFileReader/Stub/CandidateFileReaderInMemory.cpp
        XpansionStudyFileReader/Stub/CandidateFileReaderInMemory.h
        CoreHexagone/Model/Xpansion/Study.cpp
        CoreHexagone/Model/Xpansion/Study.h
        XpansionStudyFileReader/Model/Candidate.cpp
        XpansionStudyFileReader/Model/Candidate.h
        ACL/LinkFromFileTranslator.cpp
        ACL/LinkFromFileTranslator.h AntaresStudyInMemoryAdapter.cpp AntaresStudyInMemoryAdapter.h CoreHexagone/Model/Antares/Study.cpp CoreHexagone/Model/Antares/Study.h CoreHexagone/OptimizationProblemAdapter.cpp CoreHexagone/OptimizationProblemAdapter.h CoreHexagone/Model/Optimization/OptimizationProblem.cpp CoreHexagone/Model/Optimization/OptimizationProblem.h CoreHexagone/ProblemSolver.cpp CoreHexagone/ProblemSolver.h CoreHexagone/Model/Solution/Solution.cpp CoreHexagone/Model/Solution/Solution.h)

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
