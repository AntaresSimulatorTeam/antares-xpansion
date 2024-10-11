add_executable(benders_sequential_test 
        benders_sequential_test.cpp
        BendersByBatchTest.cpp )

target_link_libraries(benders_sequential_test
        PRIVATE
        antaresXpansion::benders_sequential_core
        antaresXpansion::benders_by_batch_core
        antaresXpansion::benders_core
        antaresXpansion::output_core
        antaresXpansion::logger_lib
        tests_utils
        GTest::Main
        tests_utils)

target_include_directories(benders_sequential_test
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../TestDoubles/
        )

add_test(NAME unit_benders_sequential COMMAND benders_sequential_test WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
set_property(TEST unit_benders_sequential PROPERTY LABELS unit)