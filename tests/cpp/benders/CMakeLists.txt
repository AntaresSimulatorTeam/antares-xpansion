add_executable(benders_sequential_test benders_sequential_test.cpp)

target_link_libraries(benders_sequential_test
        PRIVATE
        benders_sequential_core
        benders_core
        output_core
        logger_lib
        GTest::Main)

add_test(NAME unit_benders_sequential COMMAND benders_sequential_test WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
set_property(TEST unit_benders_sequential PROPERTY LABELS unit)