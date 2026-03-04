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

configure_clang_options(test_multisolver)

add_test(NAME test_multisolver COMMAND test_multisolver WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
set_property(TEST test_multisolver PROPERTY LABELS unit)
