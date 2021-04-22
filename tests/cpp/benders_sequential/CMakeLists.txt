add_executable(benders_core_test benders_core_test.cpp)

target_link_libraries(benders_core_test
        PRIVATE
        benders_sequential_core
        GTest::Main)

add_test(NAME benders_core_unit COMMAND logger_test)
set_property(TEST benders_core_unit PROPERTY LABELS unit)
