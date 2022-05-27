add_executable(benders_sequential benders_sequential_test.cpp)

target_link_libraries(benders_sequential
        PRIVATE
        benders_sequential_core
        benders_core
        output_core
        logger_lib
        GTest::Main)

add_test(NAME benders_sequential COMMAND benders_sequential WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
set_property(TEST benders_sequential PROPERTY LABELS unit)