add_executable(grid_search_test
        grid_search_test.cpp)

target_link_libraries(grid_search_test
        PRIVATE
        antaresXpansion::grid_search_lib
        antaresXpansion::output_core
        antaresXpansion::logger_lib
        GTest::Main
        tests_utils)

target_include_directories(grid_search_test
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../TestDoubles/
        )

add_test(NAME unit_grid_search COMMAND grid_search_test WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
set_property(TEST unit_grid_search PROPERTY LABELS unit)