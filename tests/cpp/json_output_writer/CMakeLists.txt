add_executable(output_writer_test output_writer_test.cpp)

target_link_libraries(output_writer_test
        PRIVATE
        benders_core
        output_core
        GTest::Main)
#GTest::gmock
#target_link_libraries(main PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)

add_test(NAME output_writer_test COMMAND output_writer_test)
set_property(TEST output_writer_test PROPERTY LABELS unit)
