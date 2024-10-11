# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# Targets
# ===========================================================================

# ---------------------------------------------------------------------------
# Merge MPS Exe
# ---------------------------------------------------------------------------

add_executable (test_solver
	${CMAKE_CURRENT_SOURCE_DIR}/catch2.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/test.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/test_reading_problem.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/test_modifying_problem.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/test_solving_problem.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/test_exceptions.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/define_datas.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/define_datas.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/test_basis.cpp
)
#get_target_property(xpansion_interfaces_path xpansion_interfaces INTERFACE_INCLUDE_DIRECTORIES)
target_link_libraries (test_solver
	PUBLIC
		antaresXpansion::solvers
		xpansion_interfaces
)

add_test(NAME unit_solver COMMAND test_solver WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
set_property(TEST unit_solver PROPERTY LABELS unit)