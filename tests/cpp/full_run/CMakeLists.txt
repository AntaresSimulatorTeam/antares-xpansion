add_executable (full_run_tests
		FullRunTest.cpp)



target_link_libraries (full_run_tests
		PRIVATE
		GTest::Main
		${PROJECT_NAME}::full_run_lib
		)


add_test(NAME full_run_tests COMMAND full_run_tests)
set_property(TEST full_run_tests PROPERTY LABELS unit)
