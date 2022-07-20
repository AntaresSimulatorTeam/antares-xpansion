add_executable(benders_test test_sequential.cpp)

target_link_libraries(benders_test
        PRIVATE
        ${PROJECT_NAME}::benders_sequential_core
        GTest::Main)

target_include_directories(benders_test
        PRIVATE
        )

add_test(NAME benders_test COMMAND benders_test WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
set_property(TEST benders_test PROPERTY LABELS unit)
