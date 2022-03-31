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
