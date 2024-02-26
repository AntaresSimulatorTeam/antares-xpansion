add_executable(ext_loop_test 
        ext_loop_test.cpp)

target_link_libraries(ext_loop_test
        PRIVATE
        external_loop
        benders_by_batch_core
        benders_core
        output_core
        logger_lib
        GTest::Main
        tests_utils)

# target_include_directories(ext_loop_test
#         PRIVATE
#         ${CMAKE_CURRENT_SOURCE_DIR}/../TestDoubles/
#         )

add_test(NAME unit_ext_loop COMMAND ext_loop_test WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
set_property(TEST unit_ext_loop PROPERTY LABELS unit)