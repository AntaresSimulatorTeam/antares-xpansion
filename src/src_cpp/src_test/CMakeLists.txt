# ===========================================================================
# CMake configuration
# ===========================================================================

# ===========================================================================
# External requirements
# ===========================================================================

find_package (GTest REQUIRED)

message ("Gtest found here: " ${GTEST_INCLUDE_DIRS})
enable_testing ()

# ===========================================================================
# Targets
# ===========================================================================

# --------------------------------------------------------------------------
# unit tests
# --------------------------------------------------------------------------

add_executable (unit_tests
	${CMAKE_CURRENT_SOURCE_DIR}/SimpleTest.cc
	${CMAKE_CURRENT_SOURCE_DIR}/ORToolsTest.cc
	${CMAKE_CURRENT_SOURCE_DIR}/AdditionalConstraintsTest.cc
)

target_include_directories (unit_tests
	SYSTEM PRIVATE
	)
target_include_directories (unit_tests
	PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
	)

target_link_libraries (unit_tests PRIVATE
		GTest::Main
		helpers
		lpnamer
	)

if (USE_SEQUENTIAL)
	target_link_libraries (unit_tests PRIVATE
			${PROJECT_NAME}::sequentialcore
	)
endif ()

if (USE_MPI)
	target_link_libraries (unit_tests PRIVATE
			${PROJECT_NAME}::mpicore
	)
else ()
	set (THREADS_PREFER_PTHREAD_FLAG ON)
	target_link_libraries (unit_tests PRIVATE
			Threads::Threads
		)
endif ()

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	target_compile_options (unit_tests PUBLIC -Wno-global-constructors)
	target_compile_options (unit_tests PUBLIC -Wno-exit-time-destructors)
	target_compile_options (unit_tests PUBLIC -Wno-weak-vtables)
	target_compile_options (unit_tests PUBLIC -Wno-covered-switch-default)
endif ()

add_custom_command (TARGET unit_tests
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:unit_tests> ${CMAKE_BINARY_DIR}/unit_tests.bak
	COMMAND $<TARGET_FILE:unit_tests> --gtest_output=xml:${CMAKE_BINARY_DIR}/test_result.xml
	COMMENT "Runs the unit tests after build"
	)

add_test (unit_tests unit_tests)

# ----------------------------------------------------------------------------
# run_unit_tests & coverage
# ----------------------------------------------------------------------------
set (TESTS_REPORTS_DIR "${CMAKE_BINARY_DIR}/tests-reports")
set (TESTS_REPORTS_FILE "unit_tests-results.xml")

add_custom_target (run_unit_tests
	COMMAND ${CMAKE_COMMAND} -E make_directory "${TESTS_REPORTS_DIR}"
	COMMAND $<TARGET_FILE:unit_tests> --gtest_output=xml:${TESTS_REPORTS_DIR}/${TESTS_REPORTS_FILE}
	DEPENDS $<TARGET_FILE:unit_tests>
	BYPRODUCTS ${TESTS_REPORTS_DIR}/${TESTS_REPORTS_FILE}
	COMMENT "Runs the unit tests")

# --------------------------------------------------------------------------
# coverage
# --------------------------------------------------------------------------
if (${PROJECT_NAME}_WITH_COVERAGE)
	add_custom_command (TARGET unit_tests
	POST_BUILD
	COMMAND ${GCOVR_PATH} --root=${PROJECT_SOURCE_DIR} --object-directory=${PROJECT_BINARY_DIR} --gcov-exclude=".*/src_test/.*Test.c*" --branches --xml-pretty --output ${GCOV_RESULT_FILE} --print-summary
	COMMENT "Generate coverage reports"
	)
endif ()
