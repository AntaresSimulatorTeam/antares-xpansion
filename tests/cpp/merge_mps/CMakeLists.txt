add_executable(merge_mps_test
        merge_mps_test.cpp
        merge_pathway_test.cpp
)

find_package(fmt REQUIRED)
target_link_libraries(merge_mps_test
        PRIVATE
        antaresXpansion::merge_mps_core
        tests_utils
        GTest::Main
        fmt::fmt
)

target_include_directories(merge_mps_test
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../TestDoubles/
)

add_test(NAME unit_merge_mps COMMAND merge_mps_test)
set_property(TEST unit_merge_mps PROPERTY LABELS unit)