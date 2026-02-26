# CMake for benders_strategy

# Header-only / interface library: adapters are inline in headers to remain non-invasive
add_library(benders_strategy INTERFACE)

target_include_directories(benders_strategy
    INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

# Do not link heavy dependencies here; this is a header-only interface library
# Users can link implementation dependencies at top-level when ready.

add_library(antaresXpansion::benders_strategy ALIAS benders_strategy)

install(DIRECTORY include/antares-xpansion
        DESTINATION "include")

# Add tests - now always enabled since strategy pattern is the default
find_package(GTest QUIET)
if(GTEST_FOUND)
    # SequentialExecutionStrategy unit test
    add_executable(SequentialExecutionStrategy_test tests/SequentialExecutionStrategy_test.cpp)
    target_include_directories(SequentialExecutionStrategy_test PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../benders_sequential/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../benders_core/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../factories/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../logger/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../core/include>
    )
    target_link_libraries(SequentialExecutionStrategy_test PRIVATE 
        GTest::GTest 
        GTest::Main
        antaresXpansion::benders_core 
        antaresXpansion::benders_sequential_core
        antaresXpansion::factories
        antaresXpansion::core)
    add_test(NAME SequentialExecutionStrategy_test COMMAND SequentialExecutionStrategy_test)

    # ParallelMpiExecutionStrategy unit test
    add_executable(ParallelMpiExecutionStrategy_test tests/ParallelMpiExecutionStrategy_test.cpp)
    target_include_directories(ParallelMpiExecutionStrategy_test PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../benders_mpi/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../benders_core/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../factories/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../logger/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../core/include>
    )
    target_link_libraries(ParallelMpiExecutionStrategy_test PRIVATE 
        GTest::GTest 
        GTest::Main
        antaresXpansion::benders_core 
        antaresXpansion::benders_mpi_core
        antaresXpansion::factories
        antaresXpansion::core)
    add_test(NAME ParallelMpiExecutionStrategy_test COMMAND ParallelMpiExecutionStrategy_test)

    # NoBatchingStrategy unit test
    add_executable(NoBatchingStrategy_test tests/NoBatchingStrategy_test.cpp)
    target_include_directories(NoBatchingStrategy_test PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    )
    target_link_libraries(NoBatchingStrategy_test PRIVATE 
        GTest::GTest 
        GTest::Main)
    add_test(NAME NoBatchingStrategy_test COMMAND NoBatchingStrategy_test)

    # ByBatchStrategy unit test
    add_executable(ByBatchStrategy_test tests/ByBatchStrategy_test.cpp)
    target_include_directories(ByBatchStrategy_test PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../benders_by_batch/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../benders_mpi/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../benders_core/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../factories/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../logger/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../core/include>
    )
    target_link_libraries(ByBatchStrategy_test PRIVATE 
        GTest::GTest 
        GTest::Main
        antaresXpansion::benders_core 
        antaresXpansion::benders_by_batch_core
        antaresXpansion::benders_mpi_core
        antaresXpansion::factories
        antaresXpansion::core)
    add_test(NAME ByBatchStrategy_test COMMAND ByBatchStrategy_test)

    # NoOuterLoopStrategy unit test
    add_executable(NoOuterLoopStrategy_test tests/NoOuterLoopStrategy_test.cpp)
    target_include_directories(NoOuterLoopStrategy_test PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    )
    target_link_libraries(NoOuterLoopStrategy_test PRIVATE 
        GTest::GTest 
        GTest::Main)
    add_test(NAME NoOuterLoopStrategy_test COMMAND NoOuterLoopStrategy_test)

    # OuterLoopAdapter unit test
    add_executable(OuterLoopAdapter_test tests/OuterLoopAdapter_test.cpp)
    target_include_directories(OuterLoopAdapter_test PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../outer_loop/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../core/include>
    )
    target_link_libraries(OuterLoopAdapter_test PRIVATE 
        GTest::GTest 
        GTest::Main
        antaresXpansion::outer_loop_lib
        antaresXpansion::core)
    add_test(NAME OuterLoopAdapter_test COMMAND OuterLoopAdapter_test)

    # BendersCore unit test
    add_executable(BendersCore_test tests/BendersCore_test.cpp)
    target_include_directories(BendersCore_test PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../benders_core/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../core/include>
    )
    target_link_libraries(BendersCore_test PRIVATE 
        GTest::GTest 
        GTest::Main
        antaresXpansion::core
        antaresXpansion::benders_core
        antaresXpansion::solvers)
    add_test(NAME BendersCore_test COMMAND BendersCore_test)

    # BendersFactory unit test
    add_executable(BendersFactory_test tests/BendersFactory_test.cpp)
    target_include_directories(BendersFactory_test PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../benders_core/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../factories/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../core/include>
    )
    target_link_libraries(BendersFactory_test PRIVATE 
        GTest::GTest 
        GTest::Main
        antaresXpansion::factories
        antaresXpansion::core)
    add_test(NAME BendersFactory_test COMMAND BendersFactory_test)
else()
    message(STATUS "GTest not found, strategy tests will not be built with GTest::Main")
endif()
