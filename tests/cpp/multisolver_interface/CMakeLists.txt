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

# --------------------------------------------------------------------------
# SolverMathOpt unit tests
# --------------------------------------------------------------------------
add_executable(test_solver_mathopt
        test_solver_mathopt.cpp
)

target_link_libraries(test_solver_mathopt PRIVATE
        GTest::Main
        antaresXpansion::solvers
)

target_include_directories(test_solver_mathopt PRIVATE
        ${CMAKE_SOURCE_DIR}/src/cpp/multisolver_interface/private
)

configure_clang_options(test_solver_mathopt)

add_test(NAME test_solver_mathopt COMMAND test_solver_mathopt WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
set_property(TEST test_solver_mathopt PROPERTY LABELS unit)

# --------------------------------------------------------------------------
# SolverMathOpt getter tests
# --------------------------------------------------------------------------
add_executable(test_solver_mathopt_getters
        test_solver_mathopt_getters.cpp
)

target_link_libraries(test_solver_mathopt_getters PRIVATE
        GTest::Main
        antaresXpansion::solvers
)

target_include_directories(test_solver_mathopt_getters PRIVATE
        ${CMAKE_SOURCE_DIR}/src/cpp/multisolver_interface/private
)

configure_clang_options(test_solver_mathopt_getters)

add_test(NAME test_solver_mathopt_getters COMMAND test_solver_mathopt_getters WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
set_property(TEST test_solver_mathopt_getters PROPERTY LABELS unit)
