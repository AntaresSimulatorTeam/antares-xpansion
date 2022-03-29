add_executable (json_reader
		JsonXpansionReaderTest.cc)

target_include_directories (json_reader
		SYSTEM PRIVATE
		)
target_include_directories (json_reader
		PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}
		)

target_link_libraries (json_reader PRIVATE
		GTest::Main
		helpers
		lp_namer_helper
		lp_namer_model
		lp_namer_input_reader
		lp_namer_problem_modifier
		${PROJECT_NAME}::benders_sequential_core
		)

if (USE_MPI)
	target_link_libraries (json_reader PRIVATE
			${PROJECT_NAME}::benders_mpi_core
			)
else ()
	set (THREADS_PREFER_PTHREAD_FLAG ON)
	target_link_libraries (json_reader PRIVATE
			Threads::Threads
			)
endif ()

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	target_compile_options (json_reader PUBLIC -Wno-global-constructors)
	target_compile_options (json_reader PUBLIC -Wno-exit-time-destructors)
	target_compile_options (json_reader PUBLIC -Wno-weak-vtables)
	target_compile_options (json_reader PUBLIC -Wno-covered-switch-default)
endif ()

add_test(NAME json_reader COMMAND json_reader)
set_property(TEST json_reader PROPERTY LABELS unit)

# # ----------------------------------------------------------------------------
# # run_json_reader & coverage
# # ----------------------------------------------------------------------------
# set (TESTS_REPORTS_DIR "${CMAKE_BINARY_DIR}/tests-reports")
# set (TESTS_REPORTS_FILE "json_reader-results.xml")

# add_custom_target (run_json_reader
# 		COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:json_reader> ${CMAKE_BINARY_DIR}/json_reader.bak
# 		COMMAND ${CMAKE_COMMAND} -E make_directory "${TESTS_REPORTS_DIR}"
# 		COMMAND $<TARGET_FILE:json_reader> --gtest_output=xml:${TESTS_REPORTS_DIR}/${TESTS_REPORTS_FILE}
# 		DEPENDS $<TARGET_FILE:json_reader>
# 		BYPRODUCTS ${TESTS_REPORTS_DIR}/${TESTS_REPORTS_FILE}
# 		COMMENT "Runs the unit tests")

# # --------------------------------------------------------------------------
# # coverage
# # --------------------------------------------------------------------------
# if (${PROJECT_NAME}_WITH_COVERAGE AND (CMAKE_BUILD_TYPE STREQUAL Debug))
# 	add_custom_command (TARGET run_json_reader
# 			POST_BUILD
# 			COMMAND ${GCOVR_PATH} --root=${PROJECT_SOURCE_DIR} --object-directory=${PROJECT_BINARY_DIR} --gcov-exclude=".*/src_test/.*Test.c*" --branches --xml-pretty --output ${GCOV_RESULT_FILE} --print-summary
# 			COMMENT "Generate coverage reports"
# 			)
# endif ()
