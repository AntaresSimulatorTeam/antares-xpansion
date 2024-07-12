add_executable(adequacy_criterion_test 
        adequacy_criterion_test.cpp)

target_link_libraries(adequacy_criterion_test
        PRIVATE
        solvers
        adequacy_criterion_lib
        benders_by_batch_core
        benders_core
        output_core
        logger_lib
        GTest::Main
        tests_utils)

add_test(NAME unit_adequacy_criterion COMMAND adequacy_criterion_test WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
set_property(TEST unit_adequacy_criterion PROPERTY LABELS unit)