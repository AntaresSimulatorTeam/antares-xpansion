add_executable(restart_lib_tests
        restart_lib_tests.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../TestDoubles/WriterStub.h
        ${CMAKE_CURRENT_SOURCE_DIR}/../TestDoubles/LoggerStub.h
        )

target_link_libraries(restart_lib_tests
        PRIVATE
        benders_core
        antaresXpansion::logger_lib
        GTest::Main)

target_include_directories(restart_lib_tests
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../TestDoubles/
        )

#GTest::gmock
#target_link_libraries(main PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)

add_test(NAME unit_restart_lib_tests COMMAND restart_lib_tests)
set_property(TEST unit_restart_lib_tests PROPERTY LABELS unit)
