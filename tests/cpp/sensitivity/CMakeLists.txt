add_executable(sensitivity_writer_test SensitivityWriterTest.cpp)

target_link_libraries(sensitivity_writer_test
        PRIVATE
        sensitivity
        GTest::Main)

add_test(NAME sensitivity_writer_test COMMAND sensitivity_writer_test)
set_property(TEST sensitivity_writer_test PROPERTY LABELS unit)