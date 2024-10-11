add_executable(zip_mps_lib_tests zip_mps_lib_tests.cpp)

target_link_libraries(zip_mps_lib_tests
        PRIVATE
        antaresXpansion::helpers
        tests_utils
        GTest::Main)
add_test(NAME zip_mps_lib_tests COMMAND zip_mps_lib_tests WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
set_property(TEST zip_mps_lib_tests PROPERTY LABELS unit)
