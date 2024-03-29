add_executable (helpers_test
		JsonXpansionReaderTest.cc
		AntaresVersionProviderTest.cpp
		OptionsParserTest.cpp)

target_include_directories (helpers_test
		SYSTEM PRIVATE
		)
target_include_directories (helpers_test
		PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}
		)

jsoncpplib_name(jsonlib)
target_link_libraries (helpers_test PRIVATE
		GTest::Main
		helpers
		lp_namer_helper
		lp_namer_model
		lp_namer_input_reader
		lp_namer_problem_modifier
		${jsonlib}
		${PROJECT_NAME}::benders_sequential_core
		)

target_link_libraries (helpers_test PRIVATE
			${PROJECT_NAME}::benders_mpi_core
			)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	target_compile_options (helpers_test PUBLIC -Wno-global-constructors)
	target_compile_options (helpers_test PUBLIC -Wno-exit-time-destructors)
	target_compile_options (helpers_test PUBLIC -Wno-weak-vtables)
	target_compile_options (helpers_test PUBLIC -Wno-covered-switch-default)
endif ()

add_test(NAME helpers_test COMMAND helpers_test)
set_property(TEST helpers_test PROPERTY LABELS unit)
