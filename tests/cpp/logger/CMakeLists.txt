add_executable(logger_test logger_test.cpp)

target_link_libraries(logger_test
        PRIVATE
        antaresXpansion::logger_lib
        tests_utils
        GTest::Main)

add_test(NAME unit_logger COMMAND logger_test)
set_property(TEST unit_logger PROPERTY LABELS unit)
