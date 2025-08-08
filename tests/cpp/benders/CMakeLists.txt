add_executable(benders_sequential_test)
target_sources(benders_sequential_test
        PRIVATE
        benders_sequential_test.cpp
        BendersBaseTest.cpp
        BendersByBatchTest.cpp
        WorkerMasterTest.cpp
        SubproblemWorkerTest.cpp
        MergeMasterTest.cpp
)

target_link_libraries(benders_sequential_test
        PRIVATE
        antaresXpansion::benders_sequential_core
        antaresXpansion::benders_by_batch_core
        antaresXpansion::benders_core
        antaresXpansion::output_core
        antaresXpansion::logger_lib
        merge_master_mps_core
        tests_utils
        GTest::Main)

target_include_directories(benders_sequential_test
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../TestDoubles/
)

add_test(NAME unit_benders_sequential COMMAND benders_sequential_test WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
set_property(TEST unit_benders_sequential PROPERTY LABELS unit)