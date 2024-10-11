add_executable(output_writer output_writer_test.cpp)

target_link_libraries(output_writer
        PRIVATE
        benders_core
        antaresXpansion::output_core
        GTest::Main)
#GTest::gmock
#target_link_libraries(main PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)

add_test(NAME output_writer COMMAND output_writer)
set_property(TEST output_writer PROPERTY LABELS unit)
