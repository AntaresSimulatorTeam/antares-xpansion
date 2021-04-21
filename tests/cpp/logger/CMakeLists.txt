
add_executable(logger_test logger_test.cpp)

target_link_libraries(logger_test
        PRIVATE
        logger_lib
        GTest::Main)

add_test(NAME unit_logger COMMAND logger_test)
set_property(TEST unit_logger PROPERTY LABELS unit)