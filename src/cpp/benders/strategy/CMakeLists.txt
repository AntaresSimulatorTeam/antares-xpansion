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

# Add a small smoke test when the strategy feature is enabled
if(ENABLE_BENDERS_STRATEGY)
    find_package(GTest QUIET)
    if(GTEST_FOUND)
        add_executable(adapter_smoke_test tests/adapter_smoke_test.cpp)
        target_include_directories(adapter_smoke_test PRIVATE
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../adapters/include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../benders_core/include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../factories/include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../logger/include>
        )
        target_link_libraries(adapter_smoke_test PRIVATE GTest::GTest antaresXpansion::benders_core antaresXpansion::factories)
        add_test(NAME adapter_smoke_test COMMAND adapter_smoke_test)
    else()
        add_executable(adapter_smoke_test tests/adapter_smoke_test.cpp)
        target_include_directories(adapter_smoke_test PRIVATE
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../adapters/include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../benders_core/include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../factories/include>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../logger/include>
        )
        target_link_libraries(adapter_smoke_test PRIVATE antaresXpansion::benders_core antaresXpansion::factories)
        add_test(NAME adapter_smoke_test COMMAND adapter_smoke_test)
        message(STATUS "GTest not found, strategy adapter smoke test will not be built")
    endif()
endif()
