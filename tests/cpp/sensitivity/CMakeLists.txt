add_executable(sensitivity_test SensitivityWriterTest.cpp SensitivityPbModifierTest.cpp)

target_link_libraries(sensitivity_test
        PRIVATE
        sensitivity
        GTest::Main)

add_test(NAME sensitivity_test COMMAND sensitivity_test WORKING_DIRECTORY ${CURRENT_RUNTIME_OUTPUT_DIRECTORY})
set_property(TEST sensitivity_test PROPERTY LABELS unit)