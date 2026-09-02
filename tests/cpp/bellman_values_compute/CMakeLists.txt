add_executable(bellman_values_compute_test
        bellman_values_compute_test.cpp)

target_link_libraries(bellman_values_compute_test
        PRIVATE
        antaresXpansion::evaluator_lib
        antaresXpansion::bellman_values_lib
        antaresXpansion::output_core
        antaresXpansion::logger_lib
        antaresXpansion::problem_generation_main
        antaresXpansion::solvers
        GTest::gtest_main
        tests_utils)

target_include_directories(bellman_values_compute_test
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../TestDoubles/
        )

add_test(NAME bellman_values_compute COMMAND bellman_values_compute_test WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
set_property(TEST bellman_values_compute PROPERTY LABELS bellman_values)