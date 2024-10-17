add_executable(sensitivity_test SensitivityInputReaderTest.cpp  SensitivityLoggerTest.cpp SensitivityWriterTest.cpp SensitivityPbModifierTest.cpp SensitivityStudyTest.cpp)

target_link_libraries(sensitivity_test
        PRIVATE
        antaresXpansion::sensitivity_core
        GTest::Main)

add_test(NAME unit_sensitivity COMMAND sensitivity_test WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
set_property(TEST unit_sensitivity PROPERTY LABELS unit)