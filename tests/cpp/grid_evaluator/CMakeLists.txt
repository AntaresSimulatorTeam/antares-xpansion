add_executable(grid_evaluator_test
        grid_evaluator_test.cpp)

target_link_libraries(grid_evaluator_test
        PRIVATE
        antaresXpansion::grid_evaluator_lib
        antaresXpansion::output_core
        antaresXpansion::logger_lib
        tests_utils
        GTest::Main
        tests_utils)

target_include_directories(grid_evaluator_test
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../TestDoubles/
        )

add_test(NAME unit_grid_evaluator COMMAND grid_evaluator_test WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
set_property(TEST unit_grid_evaluator PROPERTY LABELS unit)