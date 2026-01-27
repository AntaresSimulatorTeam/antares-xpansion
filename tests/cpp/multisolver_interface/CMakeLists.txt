# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# External requirements
# ===========================================================================
find_package(GTest REQUIRED)

# --------------------------------------------------------------------------
# unit tests
# --------------------------------------------------------------------------
add_executable(test_multisolver
        test_multisolver.cpp
)

target_link_libraries(test_multisolver PRIVATE
        GTest::Main
        antaresXpansion::solvers
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(test_multisolver PUBLIC -Wno-global-constructors)
    target_compile_options(test_multisolver PUBLIC -Wno-exit-time-destructors)
    target_compile_options(test_multisolver PUBLIC -Wno-weak-vtables)
    target_compile_options(test_multisolver PUBLIC -Wno-covered-switch-default)
endif ()

add_test(NAME test_multisolver COMMAND test_multisolver WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
set_property(TEST test_multisolver PROPERTY LABELS unit)
