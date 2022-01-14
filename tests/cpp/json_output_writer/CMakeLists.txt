add_executable(output_writer_test output_writer_test.cpp)

target_link_libraries(output_writer_test
        PRIVATE
        benders_core
        GTest::Main
        gmock )

add_test(NAME output_writer_test COMMAND output_writer_test)
set_property(TEST output_writer_test PROPERTY LABELS unit)
