add_executable(sensitivity_test SensitivityInputReaderTest.cpp SensitivityWriterTest.cpp SensitivityPbModifierTest.cpp SensitivityAnalysisTest.cpp)

target_link_libraries(sensitivity_test
        PRIVATE
        sensitivity_core
        GTest::Main)

add_test(NAME unit_sensitivity COMMAND sensitivity_test WORKING_DIRECTORY ${CURRENT_RUNTIME_OUTPUT_DIRECTORY})
set_property(TEST unit_sensitivity PROPERTY LABELS unit)