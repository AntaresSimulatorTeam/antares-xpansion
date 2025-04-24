add_executable(variation_de_niveaux_de_stock_test
        variation_de_niveaux_de_stock_test.cpp)

target_link_libraries(variation_de_niveaux_de_stock_test
        PRIVATE
        antaresXpansion::variation_de_niveaux_de_stock_lib
        antaresXpansion::output_core
        antaresXpansion::logger_lib
        tests_utils
        GTest::Main
        tests_utils)

target_include_directories(variation_de_niveaux_de_stock_test
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../TestDoubles/
        )

add_test(NAME unit_variation_de_niveaux_de_stock COMMAND variation_de_niveaux_de_stock_test WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
set_property(TEST unit_variation_de_niveaux_de_stock PROPERTY LABELS unit)