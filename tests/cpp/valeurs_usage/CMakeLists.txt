add_executable(valeurs_usage_test
        valeurs_usage_test.cpp)

target_link_libraries(valeurs_usage_test
        PRIVATE
        antaresXpansion::valeurs_usage_lib
        antaresXpansion::output_core
        antaresXpansion::logger_lib
        tests_utils
        GTest::Main
        tests_utils)

target_include_directories(valeurs_usage_test
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../TestDoubles/
        )

add_test(NAME unit_valeurs_usage COMMAND valeurs_usage_test WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
set_property(TEST unit_valeurs_usage PROPERTY LABELS unit)